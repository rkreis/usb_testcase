#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/common.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/l0/rcc.h>

// hack!
#define LIBOPENCM3_RTC_H
#include <libopencm3/stm32/f0/crs.h>

#define PWR_CR MMIO32(POWER_CONTROL_BASE + 0x00)
#define SYSCFG_CFGR3 MMIO32(0x40010000 + 0x20)

#include "usb_cdc.h"

extern "C" {
	void usb_rx(char *data, int len) {
		usb_tx("OK!\n", 4);
		gpio_toggle(GPIOA, GPIO10);
		if(len == 1 && *data == '!')
			while(true);
	}
}

int main() {
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_SYSCFG);

	// LEDs at PA9,10; USB at PA11,12, others input with pulldown

	gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, GPIO_ALL);
	gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO9 | GPIO10);
	gpio_mode_setup(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, GPIO_ALL);

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

	// make sure we never get interrupts
	asm volatile("cpsid i");

	// prepare wakeup events
	nvic_enable_irq(NVIC_USB_IRQ);

	usb_init();
	while(true) {
		asm volatile("wfi");
		usb_poll();
		nvic_clear_pending_irq(NVIC_USB_IRQ);
		gpio_toggle(GPIOA, GPIO9);
	}
}
