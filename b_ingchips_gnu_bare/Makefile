
#/*
#** COPYRIGHT (c) 2020 by INGCHIPS
#*/

TARGET = ing9188
DEBUG = 1
OPT = -Os

BUILD_DIR = build

BTSTACK_ROOT ?= ./btstack
VPATH += ${BTSTACK_ROOT}/example

C_SOURCES =  \
	${BTSTACK_ROOT}/main.c \
	${BTSTACK_ROOT}/FWlib/peripheral_gpio.c \
	${BTSTACK_ROOT}/FWlib/peripheral_pinctrl.c \
	${BTSTACK_ROOT}/FWlib/peripheral_sysctrl.c \
	${BTSTACK_ROOT}/FWlib/peripheral_uart.c \

ASM_SOURCES = startup_ing9188.s

PREFIX = arm-none-eabi-
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif

HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

CPU = -mcpu=cortex-m3
FPU =
FLOAT-ABI =
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

AS_DEFS =
C_DEFS =

AS_INCLUDES =

C_INCLUDES =  \
	-I${BTSTACK_ROOT}/FWlib \
	-IDrivers/CMSIS/Include \

ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections
CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif

CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

LDSCRIPT = ing9188.ld

LIBS = -lc -lnosys
LIBDIR =
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

EXAMPLES = \
	example_main \

GATT_FILES =

OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

all: \
	$(OBJECTS) \
	$(addprefix $(BUILD_DIR)/,$(EXAMPLES:=.elf)) \
	$(addprefix $(BUILD_DIR)/,$(EXAMPLES:=.hex)) \
	$(addprefix $(BUILD_DIR)/,$(EXAMPLES:=.bin)) \
	$(addprefix $(BUILD_DIR)/,$(EXAMPLES:=.jdebug)) \

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.elf: Makefile $(OBJECTS) $(BUILD_DIR)/%.o
	$(CC) $(filter-out Makefile,$^)  $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@

$(BUILD_DIR)/%.jdebug: ozone.jdebug | $(BUILD_DIR)
	sed -e "s|EXAMPLE|$(basename $(notdir $@))|" $<  > $@

$(BUILD_DIR):
	mkdir $@

clean:
	-rm -fR $(BUILD_DIR)

-include $(wildcard $(BUILD_DIR)/*.d)

