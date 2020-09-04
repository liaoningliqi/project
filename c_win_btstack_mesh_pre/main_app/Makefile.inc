
VPATH += ${BTSTACK_ROOT}/src
VPATH += ${BTSTACK_ROOT}/src/ble
VPATH += ${BTSTACK_ROOT}/src/ble/gatt-service
VPATH += ${BTSTACK_ROOT}/src/classic
VPATH += ${BTSTACK_ROOT}/src/mesh
VPATH += ${BTSTACK_ROOT}/example
VPATH += ${BTSTACK_ROOT}/3rd-party/bluedroid/decoder/srce
VPATH += ${BTSTACK_ROOT}/3rd-party/bluedroid/encoder/srce
VPATH += ${BTSTACK_ROOT}/3rd-party/hxcmod-player
VPATH += ${BTSTACK_ROOT}/3rd-party/hxcmod-player
VPATH += ${BTSTACK_ROOT}/3rd-party/hxcmod-player/mods
VPATH += ${BTSTACK_ROOT}/3rd-party/lwip/core/src/core/
VPATH += ${BTSTACK_ROOT}/3rd-party/lwip/core/src/core/ipv4
VPATH += ${BTSTACK_ROOT}/3rd-party/lwip/core/src/core/ipv6
VPATH += ${BTSTACK_ROOT}/3rd-party/lwip/core/src/netif
VPATH += ${BTSTACK_ROOT}/3rd-party/lwip/core/src/apps/http
VPATH += ${BTSTACK_ROOT}/3rd-party/lwip/dhcp-server
VPATH += ${BTSTACK_ROOT}/3rd-party/md5
VPATH += ${BTSTACK_ROOT}/3rd-party/micro-ecc
VPATH += ${BTSTACK_ROOT}/3rd-party/yxml
VPATH += ${BTSTACK_ROOT}/platform/lwip
VPATH += ${BTSTACK_ROOT}/platform/lwip/port

CFLAGS += -I.
CFLAGS += -I${BTSTACK_ROOT}/src/ble
CFLAGS += -I${BTSTACK_ROOT}/src/classic
CFLAGS += -I${BTSTACK_ROOT}/src/mesh
CFLAGS += -I${BTSTACK_ROOT}/src
CFLAGS += -I${BTSTACK_ROOT}/3rd-party/bluedroid/decoder/include
CFLAGS += -I${BTSTACK_ROOT}/3rd-party/bluedroid/encoder/include
CFLAGS += -I${BTSTACK_ROOT}/3rd-party/hxcmod-player
CFLAGS += -I${BTSTACK_ROOT}/3rd-party/lwip/core/src/include/
CFLAGS += -I${BTSTACK_ROOT}/3rd-party/lwip/dhcp-server
CFLAGS += -I${BTSTACK_ROOT}/3rd-party/md5
CFLAGS += -I${BTSTACK_ROOT}/3rd-party/micro-ecc
CFLAGS += -I${BTSTACK_ROOT}/3rd-party/yxml
CFLAGS += -I${BTSTACK_ROOT}/platform/lwip
CFLAGS += -I${BTSTACK_ROOT}/platform/lwip/port

# for CVSD/SBC PLC
LDFLAGS += -lm

CORE += \
	btstack_memory.c            \
	btstack_linked_list.c       \
	btstack_memory_pool.c       \
	btstack_run_loop.c          \
	btstack_util.c              \

COMMON += \
	ad_parser.c                 \
	hci.c                       \
	hci_cmd.c                   \
	hci_dump.c                  \
	l2cap.c                     \
	l2cap_signaling.c           \
	btstack_audio.c             \
	btstack_tlv.c               \
	btstack_crypto.c            \
	uECC.c                      \
	sm.c                        \

CLASSIC += \
	sdp_util.c                  \
	gatt_sdp.c                  \
	spp_server.c                \
	rfcomm.c                    \
	bnep.c                      \
	sdp_server.c                \
	device_id_server.c          \

SDP_CLIENT += \
	sdp_client.o                \
	sdp_client_rfcomm.o         \

ATT	+= \
	att_dispatch.c              \

GATT_SERVER += \
	att_db.c                    \
	att_db_util.c               \
	att_server.c                \

GATT_CLIENT += \
	gatt_client.c               \

PAN += \
	pan.c                       \

MBEDTLS = \
	bignum.c                    \
	ecp.c                       \
	ecp_curves.c                \
	sm_mbedtls_allocator.c      \
	memory_buffer_alloc.c       \
	platform.c                  \

LWIP_CORE_SRC  = init.c mem.c memp.c netif.c udp.c ip.c pbuf.c inet_chksum.c def.c tcp.c tcp_in.c tcp_out.c timeouts.c sys_arch.c
LWIP_IPV4_SRC  = acd.c dhcp.c etharp.c icmp.c ip4.c ip4_frag.c ip4_addr.c
LWIP_NETIF_SRC = ethernet.c
LWIP_HTTPD = altcp_proxyconnect.c fs.c httpd.c
LWIP_SRC = ${LWIP_CORE_SRC} ${LWIP_IPV4_SRC} ${LWIP_NETIF_SRC} ${LWIP_HTTPD} dhserver.c

# List of files for Bluedroid SBC codec
include ${BTSTACK_ROOT}/3rd-party/bluedroid/decoder/Makefile.inc
include ${BTSTACK_ROOT}/3rd-party/bluedroid/encoder/Makefile.inc

SBC_DECODER += \
	btstack_sbc_plc.c \
	btstack_sbc_decoder_bluedroid.c \

SBC_ENCODER += \
	btstack_sbc_encoder_bluedroid.c \
	hfp_msbc.c \

CVSD_PLC = \
	btstack_cvsd_plc.c \

AVDTP += \
	avdtp_util.c           \
	avdtp.c                \
	avdtp_initiator.c      \
	avdtp_acceptor.c       \
	avdtp_source.c 	       \
	avdtp_sink.c           \
	a2dp_source.c          \
	a2dp_sink.c            \
	btstack_ring_buffer.c \

AVRCP += \
	avrcp.c                        \
	avrcp_controller.c             \
	avrcp_target.c                 \
	avrcp_browsing.c               \
	avrcp_browsing_controller.c    \
	avrcp_browsing_target.c        \
	avrcp_media_item_iterator.c    \

HXCMOD_PLAYER = \
	hxcmod.c                      \
	nao-deceased_by_disease.c     \

MESH = \
	adv_bearer.c                  \
	beacon.c                      \
	gatt_bearer.c                 \
	mesh.c                        \
	mesh_access.c                 \
	mesh_configuration_client.c   \
	mesh_configuration_server.c   \
	mesh_crypto.c \
	mesh_foundation.c \
	mesh_generic_default_transition_time_client.c \
	mesh_generic_default_transition_time_server.c \
	mesh_generic_level_client.c \
	mesh_generic_level_server.c \
	mesh_generic_on_off_client.c \
	mesh_generic_on_off_server.c \
	mesh_health_server.c \
	mesh_iv_index_seq_number.c \
	mesh_keys.c \
	mesh_lower_transport.c \
	mesh_network.c \
	mesh_node.c \
	mesh_peer.c \
	mesh_provisioning_service_server.c \
	mesh_proxy.c \
	mesh_proxy_service_server.c \
	mesh_upper_transport.c \
	mesh_virtual_addresses.c \
	pb_adv.c \
	pb_gatt.c \
	provisioning.c \
	provisioning_device.c \
	provisioning_provisioner.c \

CORE_OBJ           = $(addprefix $(BUILD_DIR)/, $(CORE:.c=.o)          )
COMMON_OBJ         = $(addprefix $(BUILD_DIR)/, $(COMMON:.c=.o)        )
CLASSIC_OBJ        = $(addprefix $(BUILD_DIR)/, $(CLASSIC:.c=.o)       )
ATT_OBJ            = $(addprefix $(BUILD_DIR)/, $(ATT:.c=.o)           )
GATT_CLIENT_OBJ    = $(addprefix $(BUILD_DIR)/, $(GATT_CLIENT:.c=.o)   )
GATT_SERVER_OBJ    = $(addprefix $(BUILD_DIR)/, $(GATT_SERVER:.c=.o)   )
PAN_OBJ            = $(addprefix $(BUILD_DIR)/, $(PAN:.c=.o)           )
SBC_DECODER_OBJ    = $(addprefix $(BUILD_DIR)/, $(SBC_DECODER:.c=.o)   )
SBC_ENCODER_OBJ    = $(addprefix $(BUILD_DIR)/, $(SBC_ENCODER:.c=.o)   )
CVSD_PLC_OBJ       = $(addprefix $(BUILD_DIR)/, $(CVSD_PLC:.c=.o)      )
AVDTP_OBJ          = $(addprefix $(BUILD_DIR)/, $(AVDTP:.c=.o)         )
AVRCP_OBJ          = $(addprefix $(BUILD_DIR)/, $(AVRCP:.c=.o)         )
HXCMOD_PLAYER_OBJ  = $(addprefix $(BUILD_DIR)/, $(HXCMOD_PLAYER:.c=.o) )
LWIP_OBJ           = $(addprefix $(BUILD_DIR)/, $(LWIP_SRC:.c=.o)      )
MESH_OBJ           = $(addprefix $(BUILD_DIR)/, $(MESH:.c=.o)          )

