#
# Makefile for Simple Monitor
#

# C source files for the project
PROJ_NAME = simple_monitor
SRCS  = my_main.c mycode.s mytest.c 
SRCS += hal_nrf.c radio.c aes_core.c crypto.c misc.c
SRCS += HD44780_F3.c laser.c pulse.c

BUILD = build

PROCESSOR = STM32L432xx
#PROCESSOR  = STM32F303xE

# Support multiple STLink
STLINK_DIR=/dev/serial/by-id
STLINK =$(shell if [ -d $(STLINK_DIR) ]; then \
		ls $(STLINK_DIR) | awk -F- 'match($$2, "STM32_STLink_") {print substr($$2, RSTART+RLENGTH)}'; fi)

###################################################
# Processor specific bits
ifeq ($(PROCESSOR),STM32L432xx)
#CUBEMX     = cubemx_stm32l432
CUBEMX     = cubemx/stm32l432
LDSCRIPT   = STM32L432KCUx_FLASH.ld
SRCS      += $(MONITOR)/decoder/STM32L4x2.c
HAL_PREFIX = stm32l4xx
HAL        = $(CUBEMX)/Drivers/STM32L4xx_HAL_Driver
CMSIS_DEV  = $(CMSIS)/Device/ST/STM32L4xx
SRCS      += $(CUBEMX)/startup_stm32l432xx.s
#SRCS      += $(CUBEMX)/startup_stm32l432kcux.s
OPENOCD_BOARD = ./openocd/stm32l4discovery.cfg
endif

ifeq ($(PROCESSOR),STM32F303xE)
#CUBEMX     = cubemx_stm32f303
CUBEMX     = cubemx/stm32f303
LDSCRIPT   = STM32F303RETx_FLASH.ld
SRCS      += $(MONITOR)/decoder/STM32F30xE.c
HAL_PREFIX = stm32f3xx
HAL        = $(CUBEMX)/Drivers/STM32F3xx_HAL_Driver
CMSIS_DEV  = $(CMSIS)/Device/ST/STM32F3xx
SRCS      += $(CUBEMX)/startup_stm32f303xe.s
OPENOCD_BOARD = ./openocd/stm32f3discovery.cfg
endif


# startup file

# Location of the linker scripts
LDSCRIPT_INC = ld

# monitor sources
SRCS += syscall.c

MONITOR = monitor
SRCS += $(MONITOR)/monitor.c $(MONITOR)/parser.c $(MONITOR)/dump.c \
	$(MONITOR)/terminal.c $(MONITOR)/dfu.c $(MONITOR)/wdog.c \
	$(MONITOR)/tasking.c $(MONITOR)/default.c \
	$(MONITOR)/decoder/decoder.c


# CubeMx generated files
CUBEMX_SRC = $(CUBEMX)/Src
CUBEMX_INC = $(CUBEMX)/Inc
SRCS      += $(CUBEMX_SRC)/main.c \
             $(CUBEMX_SRC)/$(HAL_PREFIX)_it.c \
             $(CUBEMX_SRC)/$(HAL_PREFIX)_hal_msp.c \
             $(CUBEMX_SRC)/system_$(HAL_PREFIX).c \
             $(CUBEMX_SRC)/adc.c \
             $(CUBEMX_SRC)/dma.c \
             $(CUBEMX_SRC)/spi.c \
					   $(CUBEMX_SRC)/gpio.c \
             $(CUBEMX_SRC)/usart.c \
             $(CUBEMX_SRC)/sysmem.c \
						 $(CUBEMX_SRC)/tim.c \
						 $(CUBEMX_SRC)/iwdg.c \


# Location of CMSIS files for our device
CMSIS     = $(CUBEMX)/Drivers/CMSIS
CMSIS_INC = $(CMSIS)/Include
CMSIS_DEV_INC = $(CMSIS_DEV)/Include
CMSIS_DEV_SRC = $(CMSIS_DEV)/Source/Templates

# Location of HAL drivers
HAL_INC = $(HAL)/Inc
HAL_SRC = $(HAL)/Src
SRCS   += $(HAL_SRC)/$(HAL_PREFIX)_hal.c \
          $(HAL_SRC)/$(HAL_PREFIX)_hal_adc.c \
          $(HAL_SRC)/$(HAL_PREFIX)_hal_adc_ex.c \
          $(HAL_SRC)/$(HAL_PREFIX)_hal_cortex.c \
          $(HAL_SRC)/$(HAL_PREFIX)_hal_dma.c \
          $(HAL_SRC)/$(HAL_PREFIX)_hal_dma_ex.c \
          $(HAL_SRC)/$(HAL_PREFIX)_hal_exti.c \
          $(HAL_SRC)/$(HAL_PREFIX)_hal_flash.c \
          $(HAL_SRC)/$(HAL_PREFIX)_hal_flash_ex.c \
          $(HAL_SRC)/$(HAL_PREFIX)_hal_flash_ramfunc.c \
          $(HAL_SRC)/$(HAL_PREFIX)_hal_gpio.c \
          $(HAL_SRC)/$(HAL_PREFIX)_hal_i2c.c \
          $(HAL_SRC)/$(HAL_PREFIX)_hal_i2c_ex.c \
          $(HAL_SRC)/$(HAL_PREFIX)_hal_iwdg.c \
          $(HAL_SRC)/$(HAL_PREFIX)_hal_pwr.c \
          $(HAL_SRC)/$(HAL_PREFIX)_hal_pwr_ex.c \
          $(HAL_SRC)/$(HAL_PREFIX)_hal_rcc.c \
          $(HAL_SRC)/$(HAL_PREFIX)_hal_rcc_ex.c \
          $(HAL_SRC)/$(HAL_PREFIX)_hal_spi.c \
          $(HAL_SRC)/$(HAL_PREFIX)_hal_spi_ex.c \
          $(HAL_SRC)/$(HAL_PREFIX)_hal_tim.c \
          $(HAL_SRC)/$(HAL_PREFIX)_hal_tim_ex.c \
          $(HAL_SRC)/$(HAL_PREFIX)_hal_uart.c \
          $(HAL_SRC)/$(HAL_PREFIX)_hal_uart_ex.c \


PREFIX	=	arm-none-eabi-
CC=$(PREFIX)gcc
AR=$(PREFIX)ar
AS=$(PREFIX)as
GDB=$(PREFIX)gdb
OBJCOPY=$(PREFIX)objcopy
OBJDUMP=$(PREFIX)objdump
SIZE=$(PREFIX)size

#FLOAT = -mfpu=fpv4-sp-d16 -mfloat-abi=softfp
FLOAT = -mfpu=fpv4-sp-d16 -mfloat-abi=hard
#FLOAT = -mfpu=fpv4-sp-d16 -mfloat-abi=soft


CFLAGS  = -Wall -g -std=gnu99
CFLAGS += -Os
CFLAGS += -Werror
CFLAGS += -mlittle-endian -mcpu=cortex-m4
CFLAGS += -mthumb
CFLAGS += $(FLOAT)
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -D$(PROCESSOR)
CFLAGS += -DUSE_FULL_LL_DRIVER
CFLAGS += -DUSE_HAL_DRIVER
CFLAGS += -DUSE_FULL_LL_DRIVER
CFLAGS += -I .
CFLAGS += -I $(MONITOR)
CFLAGS += -I $(CMSIS_INC)
CFLAGS += -I $(CMSIS_DEV_INC)
CFLAGS += -I $(CUBEMX_INC)
CFLAGS += -I $(HAL_INC)
CFLAGS += -I $(MONITOR)
CFLAGS += -I $(MONITOR)/decoder
CFLAGS += --specs=nano.specs -u _printf_float
#CFLAGS += -flto

LDFLAGS  = -Wall -g -std=c99 -Os
LDFLAGS += -mlittle-endian -mcpu=cortex-m4
LDFLAGS += -Wl,--gc-sections -Wl,-Map=$(PROJ_NAME).map
LDFLAGS += -mthumb
LDFLAGS += --specs=nano.specs -u _printf_float
LDFLAGS += $(FLOAT)
LDFLAGS += -ffunction-sections -fdata-sections
#LDFLAGS += -flto

ASFLAGS =  -g -mlittle-endian -mcpu=cortex-m4
#ASFLAGS += -march=armv7e-m
ASFLAGS += -mthumb
ASFLAGS += $(FLOAT)
#ASFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=soft
###################################################

OBJS = $(addprefix $(BUILD)/,$(addsuffix .o,$(basename $(SRCS))))
OBJS += $(BUILD)/version.o
FILES = $(notdir $(SRCS))
DEPS = $(addprefix deps/,$(addsuffix .d,$(basename $(FILES))))

###################################################

.PHONY: all program debug clean reallyclean

all: $(BUILD) $(PROJ_NAME).elf $(PROJ_NAME).dfu $(PROJ_NAME).hex \
	$(PROJ_NAME).bin

-include $(DEPS)

$(BUILD):
	mkdir -p $(sort $(dir $(OBJS)))

.depsdir: 
	mkdir -p deps
	touch .depsdir

$(BUILD)/%.o : %.c .depsdir
	$(CC) $(CFLAGS) -c -o $@ $< -MMD -MF deps/$(*F).d

$(BUILD)/%.o : %.s .depsdir
	$(AS) $(ASFLAGS) -c -o $@ $<

%.dfu: %.bin
#	dfu-suffix -v 0x0483 -p 0x0000 -d 0x0000 -S 0x011A -a $@
	cp $< $@
	dfu-suffix -v 0x0483 -a $@

%.hex: %.elf
	$(OBJCOPY) -O ihex $< $@

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

$(PROJ_NAME).elf: $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@ -L$(LDSCRIPT_INC) -T$(LDSCRIPT)
	$(OBJCOPY) -O ihex $(PROJ_NAME).elf $(PROJ_NAME).hex
	$(OBJDUMP) -St $(PROJ_NAME).elf >$(PROJ_NAME).lst
	$(SIZE) $(PROJ_NAME).elf

program: all install
#	-openocd -f board/st_nucleo_f0.cfg -c "init" -c "reset init" -c "halt" -c "flash write_image erase $(PROJ_NAME).elf" -c "reset run" -c shutdown
#-openocd -f $(OPENOCD_BOARD) -c "init" -c "reset init" -c "halt" -c "flash write_image erase $(PROJ_NAME).elf" -c "reset run" -c "shutdown"

# Support multiple STLink
install:
	@for serial in $(STLINK); do \
		echo " "; \
		echo "openocd -f $(OPENOCD_BOARD) -c \"hla_serial $${serial}\""; \
		openocd -f $(OPENOCD_BOARD) -c "hla_serial $${serial}" -c "init" -c "reset init" -c "halt" -c "flash write_image erase $(PROJ_NAME).elf" -c "reset run" -c "shutdown"; \
	done

dfu: $(PROJ_NAME).dfu
	dfu-util -a 0 --dfuse-address 0x08000000:leave -D $(PROJ_NAME).dfu

$(BUILD)/gdb_cmds: $(BUILD) Makefile
	echo "target extended-remote | openocd -f $(OPENOCD_BOARD) -c \"gdb_port pipe; log_output openocd.log\"" > $@
	echo "monitor reset halt" >> $@


debug: program $(BUILD)/gdb_cmds
	$(GDB) --tui -x build/gdb_cmds $(PROJ_NAME).elf

ddddebug: program $(BUILD)/gdb_cmds
	ddd --gdb --debugger "$(GDB) -x $(BUILD)/gdb_cmds" $(PROJ_NAME).elf

$(BUILD)/version.c: make_version $(SRCS)
	./make_version > $@

make_version: make_version.c
	cc -o $@ $^

clean:
	@if [ -d $(BUILD) ]; then \rm -r $(BUILD); fi
	@echo "rm -r $(BUILD)"
	find ./ -name '*~' | xargs rm -f
	find ./ -name '*.o' | xargs rm -f
	@if [ -d deps ]; then \rm -f deps/*.d; fi
	@echo "rm -f deps/*.d"
	rm -f .depsdir
	rm -f $(PROJ_NAME).elf
	rm -f $(PROJ_NAME).hex
	rm -f $(PROJ_NAME).bin
	rm -f $(PROJ_NAME).map
	rm -f $(PROJ_NAME).lst
	rm -f $(PROJ_NAME).dfu
	rm -f openocd.log
	rm -f make_version
	@if [ -d deps ]; then \rmdir deps; fi
	@echo "rmdir deps"
