#include <libopencm3/cm3/common.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/l0/rcc.h>

// hack!
#define LIBOPENCM3_RTC_H
#include <libopencm3/stm32/common/rtc_common_l1f024.h>
#include <libopencm3/stm32/f0/crs.h>

#define PWR_CR MMIO32(POWER_CONTROL_BASE + 0x00)
#define SYSCFG_CFGR3 MMIO32(0x40010000 + 0x20)

#include "usb_cdc.h"

extern "C" {
	void usb_rx(char *data, int len) {
		usb_tx("OK!\n", 4);
		gpio_toggle(GPIOA, GPIO10);
	}
}

int main() {
	// disable JTAG?
	// clock: 2MHz MSI

	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_SYSCFG);

	// GPIOA
	//  0: unused
	//  1: unused
	//  2: unused
	//  3: unused
	//  4: dw1k IRQ
	//  5: dw1k SPICLK
	//  6: dw1k SPIMISO
	//  7: dw1k SPIMOSI
	//  8: sensor on/off
	//  9: LED1
	// 10: LED2
	// 11: USB D-
	// 12: USB D+
	// 13: SWD CLK
	// 14: SWD DIO
	// 15: sensor 32kHz

	// GPIOB
	//  0: dw1k SPICS
	//  1: unused
	//  3: SCK
	//  4: MISO
	//  5: MOSI
	//  6: SCL
	//  7: SDA

	gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, GPIO_ALL);
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO5 | GPIO6 | GPIO7 | GPIO11 | GPIO12 | GPIO13 | GPIO14);
	gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO4);
	gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO8 | GPIO9 | GPIO10 | GPIO15);
	gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_HIGH, GPIO5 | GPIO7);

	gpio_mode_setup(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, GPIO_ALL);
	gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO0);
	gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_HIGH, GPIO0);

	//PWR_CR = (1<<14) | (3<<11) | (1<<9) | (1<<8) | (1<<1) | (1<<0);
	//PWR_CR = (3<<11) | (1<<8) | (1<<1) | (1<<0);

	rcc_osc_on(HSI16);
	rcc_wait_for_osc_ready(HSI16);
	RCC_CFGR = RCC_CFGR_SW_SYSCLKSEL_HSI16CLK;

	// Enable VREFINT for HSI48.
	SYSCFG_CFGR3 |= (1 << 13) | (1 << 0); // ENREF_HSI48, EN_VREFINT
	while(!(SYSCFG_CFGR3 & (1 << 26))); // REF_HSI48_RDYF

	// Enable HSI48.
	RCC_CRRCR |= 1 << 0; // HSI48ON
	while(!(RCC_CRRCR & (1 << 1))); // HSI48RDY

	// TODO figure out if the CRS actually works?
	//rcc_periph_clock_enable(RCC_CRS);
	//CRS_CR = CRS_CR_AUTOTRIMEN | CRS_CR_CEN;

	RCC_CCIPR = RCC_CCIPR_HSI48SEL;

	/*
	RTC_WPR = 0xCA;
	RTC_WPR = 0x53;
	RTC_CR = RTC_CR_WUCLKSEL_RTC_DIV16;
	while(!((RTC_ISR) & (RTC_ISR_WUTWF)))
		;
	RTC_WUTR = 2047;
	RTC_CR = RTC_CR_WUCLKSEL_RTC_DIV16 | RTC_CR_WUTE;
	RTC_WPR = 0xFF;
	*/

	gpio_set(GPIOA, GPIO10);

	usb_init();
	while(true) {
		gpio_toggle(GPIOA, GPIO9);
		usb_poll();
		//usb_tx("OHAI\n", 5);
	}
}
