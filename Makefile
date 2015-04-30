# source code
COMPFLAGS += -Wall -Wextra -Os -g
CFLAGS += -std=c11
CXXFLAGS += -std=c++14 -fno-exceptions

# controller
COMPFLAGS += -mcpu=cortex-m0 -mthumb -D STM32L0
LDFLAGS += -Tstm32l0xx8.ld -nostartfiles -lopencm3_stm32l0 -lstdc++_nano

COMPFLAGS += -I/usr/arm-none-eabi/include -fshort-enums -fdata-sections -ffunction-sections -fno-stack-protector
LDFLAGS += --specs=nosys.specs --static -nostartfiles -Wl,--gc-sections -mcpu=cortex-m0 -mthumb
TARGET = -target arm-none-eabi

OBJS += main.o usb_cdc.o

%.o: %.c
	arm-none-eabi-gcc ${COMPFLAGS} ${CFLAGS} -c -o $@ $<

%.o: %.cpp
	arm-none-eabi-gcc ${COMPFLAGS} ${CXXFLAGS} -c -o $@ $<

main.elf: ${OBJS}
	arm-none-eabi-gcc $^ -o $@ ${LDFLAGS}

gdb: main.elf
	arm-none-eabi-gdb -x gdb.scr $<

clean:
	rm -f ${OBJS} main.elf
