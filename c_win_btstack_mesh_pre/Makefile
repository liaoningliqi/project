
# Makefile for windows WinUSB based examples
TARGET = main_app_mesh_node

BUILD_DIR        = build
PROJECT_SRC_ROOT ?= .
ING_SDK_ROOT     ?= ../../src
FREERTOS_ROOT    ?= ../../open_source/FreeRTOS
BTSTACK_ROOT     ?= ../../open_source/btstack
CMSIS_INC_ROOT   ?= ../CMSIS/Include
MAIN_ROOT        ?= ${PROJECT_SRC_ROOT}/main
MAIN_APP_ROOT    ?= ${PROJECT_SRC_ROOT}/main_app

CORE += main.c main_app_mesh_node.c app_gatt_service.c

CFLAGS += \
	-I${BTSTACK_ROOT}/platform/windows \
	-I${BTSTACK_ROOT}/platform/posix \
	-I${BTSTACK_ROOT}/platform/embedded \
	-I${BTSTACK_ROOT}/3rd-party/rijndael \
	-I${BTSTACK_ROOT}/3rd-party/tinydir \
	-I${MAIN_APP_ROOT} \

VPATH += \
	${PROJECT_SRC_ROOT}/main \
	${PROJECT_SRC_ROOT}/main_app \

CORE += btstack_stdin_windows.c btstack_tlv_posix.c

COMMON  += hci_transport_h2_winusb.c btstack_run_loop_windows.c le_device_db_tlv.c btstack_link_key_db_tlv.c wav_util.c
COMMON  += rijndael.c

include ${MAIN_APP_ROOT}/Makefile.inc

# CC = gcc-fsf-4.9
CFLAGS  += -g -Wall -Wmissing-prototypes -Wstrict-prototypes -Wshadow -Wpedantic
# CFLAGS += -Werror

VPATH += ${BTSTACK_ROOT}/3rd-party/rijndael
VPATH += ${BTSTACK_ROOT}/platform/embedded
VPATH += ${BTSTACK_ROOT}/platform/posix
VPATH += ${BTSTACK_ROOT}/platform/windows
VPATH += ${BTSTACK_ROOT}/chipset/csr

LDFLAGS += -lsetupapi -lwinusb

EXAMPLES = ${EXAMPLES_GENERAL} ${EXAMPLES_CLASSIC_ONLY} ${EXAMPLES_LE_ONLY} ${EXAMPLES_DUAL_MODE}
EXAMPLES_LE_ONLY += main_app_mesh_node

$(TARGET): ${CORE_OBJ} ${COMMON_OBJ} ${MESH_OBJ} ${ATT_OBJ} ${GATT_SERVER_OBJ} ${SM_OBJ} | $(BUILD_DIR)
	${CC} $^ ${CFLAGS} ${LDFLAGS} -o $@

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	${CC} -c $(CFLAGS) $< -o $@

$(BUILD_DIR):
	mkdir $@

all: $(TARGET)

clean:
	-rm -fR $(BUILD_DIR)

