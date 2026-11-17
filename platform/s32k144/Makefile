# Makefile - mini AUTOSAR cluster on the MaaZEDU S32K144 board (bare metal, no NXP SDK)
#   make            -> build/cluster_ecu.elf / .hex / .bin
#   make clean
# Needs arm-none-eabi-gcc (e.g. the GCC 11.4 shipped with S32 Design Studio 3.6) and GNU make.

TOOLCHAIN ?= C:/NXP/S32DS.3.6.11/S32DS/build_tools/gcc_v11.4/gcc-11.4-arm32-eabi/bin
PREFIX    := $(TOOLCHAIN)/arm-none-eabi-
CC        := $(PREFIX)gcc
OBJCOPY   := $(PREFIX)objcopy
SIZE      := $(PREFIX)size

ROOT      := ../..
TARGET    := cluster_ecu
BUILD     := build

SRC_DIRS  := $(ROOT)/src $(ROOT)/src/Common $(ROOT)/src/Cdd $(ROOT)/src/Ecual $(ROOT)/src/Service \
             $(ROOT)/src/Rte $(ROOT)/src/Swc . Mcal
SRCS      := $(foreach d,$(SRC_DIRS),$(wildcard $(d)/*.c))
OBJS      := $(addprefix $(BUILD)/,$(notdir $(SRCS:.c=.o)))
vpath %.c $(SRC_DIRS)

CPUFLAGS  := -mcpu=cortex-m4 -mthumb -mfloat-abi=soft
CFLAGS    := $(CPUFLAGS) -std=gnu11 -Os -g3 -Wall -Wextra -Werror \
             -ffunction-sections -fdata-sections -fno-common \
             $(addprefix -I,$(SRC_DIRS))
LDFLAGS   := $(CPUFLAGS) -T S32K144_flash.ld -nostartfiles -Wl,--gc-sections \
             -Wl,-Map=$(BUILD)/$(TARGET).map --specs=nano.specs --specs=nosys.specs

all: $(BUILD)/$(TARGET).hex $(BUILD)/$(TARGET).bin
	$(SIZE) $(BUILD)/$(TARGET).elf

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: %.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/$(TARGET).elf: $(OBJS) S32K144_flash.ld
	$(CC) $(LDFLAGS) $(OBJS) -o $@

$(BUILD)/$(TARGET).hex: $(BUILD)/$(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

$(BUILD)/$(TARGET).bin: $(BUILD)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

clean:
	rm -rf $(BUILD)

.PHONY: all clean
