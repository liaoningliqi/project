
#/*
#** COPYRIGHT (c) 2020 by INGCHIPS
#*/

TARGET = ing9188
DEBUG = 1
OPT = -Og

BUILD_DIR = build

PROJECT_SRC_ROOT ?= .
ING_SDK_ROOT     ?= ../../src
FREERTOS_ROOT    ?= ../../open_source/FreeRTOS
CMSIS_INC_ROOT   ?= ../CMSIS/Include
MAIN_ROOT        ?= ${PROJECT_SRC_ROOT}/main
VPATH            += ${PROJECT_SRC_ROOT}/example

C_SOURCES = \
	${MAIN_ROOT}/main.c \
	${MAIN_ROOT}/transp_btstack.c \
	${MAIN_ROOT}/transp_freertos.c \
	${ING_SDK_ROOT}/FWlib/peripheral_gpio.c \
	${ING_SDK_ROOT}/FWlib/peripheral_pinctrl.c \
	${ING_SDK_ROOT}/FWlib/peripheral_sysctrl.c \
	${ING_SDK_ROOT}/FWlib/peripheral_uart.c \
	${FREERTOS_ROOT}/Source/list.c \
	${FREERTOS_ROOT}/Source/queue.c \
	${FREERTOS_ROOT}/Source/stream_buffer.c \
	${FREERTOS_ROOT}/Source/tasks.c \
	${FREERTOS_ROOT}/Source/event_groups.c \
	${FREERTOS_ROOT}/Source/timers.c \
	${FREERTOS_ROOT}/Source/portable/GCC/ARM_CM3/port.c \
	${FREERTOS_ROOT}/Source/portable/MemMang/heap_4.c \

EXAMPLES_APP = \
	app_main \

ASM_SOURCES = ${MAIN_ROOT}/startup_ing9188.s

LDSCRIPT = ${MAIN_ROOT}/ing9188.ld

C_INCLUDES =  \
	-I${PROJECT_SRC_ROOT}/example \
	-I${ING_SDK_ROOT}/FWlib \
	-I${ING_SDK_ROOT}/StartUP \
	-I${FREERTOS_ROOT}/Source/include \
	-I${FREERTOS_ROOT}/Source/portable/GCC/ARM_CM3 \
	-I${CMSIS_INC_ROOT} \

C_DEFS = \
	-Ddebug \

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

AS_INCLUDES =

ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections
CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif

CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

LIBS = -lc -lnosys
LIBDIR =
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

GATT_FILES =

OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

all: \
	$(OBJECTS) \
	$(addprefix $(BUILD_DIR)/,$(EXAMPLES_APP:=.elf)) \
	$(addprefix $(BUILD_DIR)/,$(EXAMPLES_APP:=.hex)) \
	$(addprefix $(BUILD_DIR)/,$(EXAMPLES_APP:=.bin)) \
	$(addprefix $(BUILD_DIR)/,$(EXAMPLES_APP:=.jdebug)) \

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

