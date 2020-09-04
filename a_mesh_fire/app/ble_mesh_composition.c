
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#include <stdio.h>
#include <stdint.h>

#include "mesh_def.h"
#include "mesh_api.h"
#include "access.h"
#include "mesh.h"
#include "transition.h"
#include "device_composition.h"

#include "profile.h"
#include "chip_peripherals.h"

#if defined __cplusplus
    extern "C" {
#endif

#define INGCHIPS_COMP_ID           (0x06AC)
#define ALIbaba_COMP_ID            (0x01A8)

#define NO_TRANSTION_STATUS (1)
#define BT_MESH_MODEL_OP_GEN_ONOFF_GET        BT_MESH_MODEL_OP_2(0x82, 0x01)
#define BT_MESH_MODEL_OP_GEN_ONOFF_SET        BT_MESH_MODEL_OP_2(0x82, 0x02)
#define BT_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK  BT_MESH_MODEL_OP_2(0x82, 0x03)
#define BT_MESH_MODEL_OP_GEN_ONOFF_STATUS     BT_MESH_MODEL_OP_2(0x82, 0x04)

#define hal_gpio_write(a, b)

#define k_sem ble_npl_sem

#undef  BT_MESH_MODEL_CFG_SRV
#undef  BT_MESH_MODEL_CFG_CLI
#define BT_MESH_MODEL_CFG_SRV(_user_data) BT_MESH_MODEL(BT_MESH_MODEL_ID_CFG_SRV, NULL, NULL, _user_data)
#define BT_MESH_MODEL_CFG_CLI(_user_data) BT_MESH_MODEL(BT_MESH_MODEL_ID_CFG_CLI, NULL, NULL, _user_data)

struct bt_mesh_cfg_cli {
    struct bt_mesh_model *model;
    struct k_sem          op_sync;
    u32_t                 op_pending;
    void                 *op_param;
};

typedef struct remote_control_adv
{
    uint16_t ing_id;
    uint8_t control_type;
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint16_t syn_cnt;
} remote_control_adv_t;

static struct bt_mesh_cfg_srv cfg_srv = {
    .relay = BT_MESH_RELAY_ENABLED,
    .beacon = BT_MESH_BEACON_ENABLED,

#if defined(CONFIG_BT_MESH_FRIEND)
    .frnd = BT_MESH_FRIEND_ENABLED,
#else
    .frnd = BT_MESH_FRIEND_NOT_SUPPORTED,
#endif

#if defined(CONFIG_BT_MESH_GATT_PROXY)
    .gatt_proxy = BT_MESH_GATT_PROXY_ENABLED,
#else
    .gatt_proxy = BT_MESH_GATT_PROXY_NOT_SUPPORTED,
#endif

    .default_ttl = 7,
    .net_transmit = BT_MESH_TRANSMIT(2, 20),
    .relay_retransmit = BT_MESH_TRANSMIT(3, 20),
};

static struct bt_mesh_model_pub gen_onoff_srv_pub_root;
static struct bt_mesh_model_pub gen_onoff_cli_pub_root;
static struct bt_mesh_model_pub vnd_pub;

static struct os_mbuf *bt_mesh_pub_msg_gen_onoff_srv_pub_root;
static struct os_mbuf *bt_mesh_pub_msg_vnd_pub;

static struct generic_onoff_state gen_onoff_srv_root_user_data;
static struct vendor_state vnd_user_data;
static struct bt_mesh_elem elements[];

static uint16_t vnd_counter = 0;
static bool beacon_disabled = 0;

static void gen_onoff_get(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, struct os_mbuf *buf);
static void gen_onoff_set(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, struct os_mbuf *buf);
static void gen_onoff_set_unack(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, struct os_mbuf *buf);
static void gen_onoff_status(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, struct os_mbuf *buf);
static void vnd_get(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, struct os_mbuf *buf);
static void vnd_set(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, struct os_mbuf *buf);
static void vnd_set_unack(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, struct os_mbuf *buf);
static void vnd_status(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, struct os_mbuf *buf);

static const struct bt_mesh_model_op gen_onoff_srv_op[] = {
    { BT_MESH_MODEL_OP_2(0x82, 0x01), 0, gen_onoff_get },
    { BT_MESH_MODEL_OP_2(0x82, 0x02), 2, gen_onoff_set },
    { BT_MESH_MODEL_OP_2(0x82, 0x03), 2, gen_onoff_set_unack },
    BT_MESH_MODEL_OP_END,
};

static const struct bt_mesh_model_op gen_onoff_cli_op[] = {
    { BT_MESH_MODEL_OP_2(0x82, 0x04), 1, gen_onoff_status },
    BT_MESH_MODEL_OP_END,
};

static const struct bt_mesh_model_op vnd_ops[] = {
    { BT_MESH_MODEL_OP_3(0x01, INGCHIPS_COMP_ID), 0, vnd_get },
    { BT_MESH_MODEL_OP_3(0x02, INGCHIPS_COMP_ID), 3, vnd_set },
    { BT_MESH_MODEL_OP_3(0x03, INGCHIPS_COMP_ID), 3, vnd_set_unack },
    { BT_MESH_MODEL_OP_3(0x04, INGCHIPS_COMP_ID), 6, vnd_status },
    BT_MESH_MODEL_OP_END,
};

static struct bt_mesh_cfg_cli cfg_cli = {
};

static struct bt_mesh_model root_models[] = {
    BT_MESH_MODEL_CFG_SRV(&cfg_srv),
    BT_MESH_MODEL_CFG_CLI(&cfg_cli),
    BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_SRV, gen_onoff_srv_op, &gen_onoff_srv_pub_root, &gen_onoff_srv_root_user_data),
    BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_CLI, gen_onoff_cli_op, &gen_onoff_cli_pub_root, NULL),
};

struct bt_mesh_model vnd_models[] = {
    BT_MESH_MODEL_VND(INGCHIPS_COMP_ID, 0x4321, vnd_ops, &vnd_pub, &vnd_user_data),
};

static struct bt_mesh_elem elements[] = {
    BT_MESH_ELEM(0, root_models, vnd_models),
};

static const struct bt_mesh_comp comp = {
    .cid = CID_RUNTIME,
    .elem = elements,
    .elem_count = ARRAY_SIZE(elements),
};

static void gen_onoff_get(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, struct os_mbuf *buf)
{
    struct os_mbuf *msg = NET_BUF_SIMPLE(2 + 3 + 4);
    struct generic_onoff_state *state = model->user_data;

    bt_mesh_model_msg_init(msg, BT_MESH_MODEL_OP_GEN_ONOFF_STATUS);
#if (NO_TRANSTION_STATUS==0)
    net_buf_simple_add_u8(msg, state->onoff);
    if (state->transition->counter) {
        //calculate_rt(state->transition);
        net_buf_simple_add_u8(msg, state->target_onoff);
        net_buf_simple_add_u8(msg, state->transition->rt);
    }
#else
    net_buf_simple_add_u8(msg, state->target_onoff);
#endif

    if (bt_mesh_model_send(model, ctx, msg, NULL, NULL)) {
        printk("Unable to send GEN_ONOFF_SRV Status response\n");
    }

    os_mbuf_free_chain(msg);

    return;
}

static void gen_onoff_publish(struct bt_mesh_model *model)
{
    int err;
    struct os_mbuf *msg = model->pub->msg;
    struct generic_onoff_state *state = model->user_data;

    if (model->pub->addr == BT_MESH_ADDR_UNASSIGNED) {
        return;
    }

    bt_mesh_model_msg_init(msg, BT_MESH_MODEL_OP_GEN_ONOFF_STATUS);
    net_buf_simple_add_u8(msg, state->onoff);
    if (state->transition->counter) {
        //calculate_rt(state->transition);
        net_buf_simple_add_u8(msg, state->target_onoff);
        net_buf_simple_add_u8(msg, state->transition->rt);
    }

    err = bt_mesh_model_publish(model);
    if (err) {
        printk("bt_mesh_model_publish err %d\n", err);
    }

    return;
}

static void gen_onoff_set_unack(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, struct os_mbuf *buf)
{
    u8_t tid, onoff, tt;
    s64_t now;
    struct generic_onoff_state *state = model->user_data;

    onoff = net_buf_simple_pull_u8(buf);
    tid = net_buf_simple_pull_u8(buf);

    if (onoff > STATE_ON) {
        return;
    }

    now = k_uptime_get();
    if ((state->last_tid == tid) &&
        (state->last_src_addr == ctx->addr) &&
        (state->last_dst_addr == ctx->recv_dst) &&
        (now - state->last_msg_timestamp <= K_SECONDS(6))) {
        return;
    }

    switch (buf->om_len) {
    case 0x00:
        break;
    case 0x02:
        tt = net_buf_simple_pull_u8(buf);
        if ((tt & 0x3F) == 0x3F) {
            return;
        }
        break;
    default:
        return;
    }

    state->last_tid = tid;
    state->last_src_addr = ctx->addr;
    state->last_dst_addr = ctx->recv_dst;
    state->last_msg_timestamp = now;
    state->target_onoff = onoff;

    if (state->target_onoff != state->onoff) {
        //onoff_tt_values(state, tt, delay);
        printf("tt 3 0x%p \n",state->transition);
    } else {
        gen_onoff_publish(model);
        return;
    }

    if (state->transition->counter == 0) {
        state->onoff = state->target_onoff;
    }

    state->transition->just_started = true;
    gen_onoff_publish(model);

    return;
}

static void gen_onoff_set(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, struct os_mbuf *buf)
{
    u8_t tid, onoff, tt;
    s64_t now;
    struct generic_onoff_state *state = model->user_data;

    onoff = net_buf_simple_pull_u8(buf);
    tid = net_buf_simple_pull_u8(buf);
    printf("onoff 0x%x\n",onoff);
    set_led_color(onoff, 0, 0);
    if (onoff > STATE_ON) {
        return;
    }

    now = k_uptime_get();
    if ((state->last_tid == tid) &&
        (state->last_src_addr == ctx->addr) &&
        (state->last_dst_addr == ctx->recv_dst) &&
        (now - state->last_msg_timestamp <= K_SECONDS(6))) {
        printf("less < 6s\n");
        gen_onoff_get(model, ctx, buf);
        return;
    }

    switch (buf->om_len) {
    case 0x00:
        break;
    case 0x02:
        tt = net_buf_simple_pull_u8(buf);
        if ((tt & 0x3F) == 0x3F) {
            return;
        }
        break;
    default:
        return;
    }

    state->last_tid = tid;
    state->last_src_addr = ctx->addr;
    state->last_dst_addr = ctx->recv_dst;
    state->last_msg_timestamp = now;
    state->target_onoff = onoff;
    printf("tt value 0x%x 0x%x state 0x%x target stat 0x%x\n", tt, state->transition, state->onoff, state->target_onoff);
    if (state->target_onoff != state->onoff) {
        //onoff_tt_values(state, tt, delay);
    } else {
        gen_onoff_get(model, ctx, buf);
        gen_onoff_publish(model);
        return;
    }

    /* For Instantaneous Transition */
    if (state->transition->counter == 0) {
        state->onoff = state->target_onoff;
    }

    state->transition->just_started = true;
    gen_onoff_get(model, ctx, buf);
    gen_onoff_publish(model);
    printf("stat 0x%x \n", state);
    //onoff_handler(state);

    return;
}

static void gen_onoff_status(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, struct os_mbuf *buf)
{
    printk("Acknownledgement from GEN_ONOFF_SRV\n");
    printk("Present OnOff = %02x\n", net_buf_simple_pull_u8(buf));

    if (buf->om_len == 2) {
        printk("Target OnOff = %02x\n", net_buf_simple_pull_u8(buf));
        printk("Remaining Time = %02x\n", net_buf_simple_pull_u8(buf));
    }

    return;
}

static void vnd_get(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, struct os_mbuf *buf)
{
    struct os_mbuf *msg = NET_BUF_SIMPLE(3 + 6 + 4);
    struct vendor_state *state = model->user_data;

    /* This is dummy response for demo purpose */
    state->response = 0xA578FEB3;

    bt_mesh_model_msg_init(msg, BT_MESH_MODEL_OP_3(0x04, CID_RUNTIME));
    net_buf_simple_add_le16(msg, state->current);
    net_buf_simple_add_le32(msg, state->response);

    if (bt_mesh_model_send(model, ctx, msg, NULL, NULL)) {
        printk("Unable to send VENDOR Status response\n");
    }

    os_mbuf_free_chain(msg);

    return;
}

static void vnd_set_unack(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, struct os_mbuf *buf)
{
    u8_t tid;
    int current;
    s64_t now;
    struct vendor_state *state = model->user_data;

    current = net_buf_simple_pull_le16(buf);
    tid = net_buf_simple_pull_u8(buf);

    now = k_uptime_get();
    if ((state->last_tid == tid) &&
        (state->last_src_addr == ctx->addr) &&
        (state->last_dst_addr == ctx->recv_dst) &&
        (now - state->last_msg_timestamp <= K_SECONDS(6))) {
        return;
    }

    state->last_tid = tid;
    state->last_src_addr = ctx->addr;
    state->last_dst_addr = ctx->recv_dst;
    state->last_msg_timestamp = now;
    state->current = current;

    printk("Vendor model message = %04x\n", state->current);

    return;
}

static void update_led_command(remote_control_adv_t remote_control_adv_t, uint8_t length)
{
    return;
}

static void vnd_set(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, struct os_mbuf *buf)
{
    u8_t *p = buf->om_data;

    vnd_counter++;
    if (beacon_disabled == 0) {
        gap_beacon_disable();
        beacon_disabled = 1;
    }
    printf("VND set 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
    update_led_command(*(remote_control_adv_t *)buf->om_data, 8);

    return;
}

static void vnd_status(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, struct os_mbuf *buf)
{
    printk("Acknownledgement from Vendor\n");
    printk("cmd = %04x\n", net_buf_simple_pull_le16(buf));
    printk("response = %08lx\n", net_buf_simple_pull_le32(buf));

    return;
}

struct bt_mesh_elem * get_element_of_node()
{
    return elements;
}

struct bt_mesh_comp * get_comp_of_node()
{
    return (struct bt_mesh_comp *)&comp;
}

uint8_t model_info_pub()
{
    uint8_t count = 0;
    struct bt_mesh_elem *elem = &elements[0];
    struct bt_mesh_model *model = (struct bt_mesh_model *)(elem->models);

    for (count = 0; count < 7; count++) {
        model++;
    }

    return 0;
}

uint8_t get_element_count(void)
{
    return ARRAY_SIZE(elements);
}

struct bt_mesh_model * get_model_by_id(uint16_t id)
{
    uint8_t r_models =  sizeof(root_models)/sizeof(root_models[0]);
    uint8_t v_models =  sizeof(vnd_models)/sizeof(vnd_models[0]);
    uint8_t i = 0;

    for (i = 0; i < r_models; i++) {
        if (root_models[i].id == id) {
            return &root_models[i];
        }
    }
    for (i = 0; i < v_models; i++) {
        if (vnd_models[i].vnd.company == id) {
            return &vnd_models[i];
        }
    }

    return NULL;
}

bool get_group_addr_by_id(uint16_t id,uint16_t* group)
{
    struct bt_mesh_model *pmod = get_model_by_id(id);
    uint8_t i = CONFIG_BT_MESH_MODEL_GROUP_COUNT;

    if (pmod == NULL) {
        return false;
    }

    for (i = 0; i < CONFIG_BT_MESH_MODEL_GROUP_COUNT; i++) {
        if(pmod->groups[i] != BT_MESH_ADDR_UNASSIGNED) {
            *group = pmod->groups[i];
            return true;
        }
    }

    return false;
}

void init_pub(void)
{
    bt_mesh_pub_msg_gen_onoff_srv_pub_root = NET_BUF_SIMPLE(2 + 3);
    gen_onoff_srv_pub_root.msg = bt_mesh_pub_msg_gen_onoff_srv_pub_root;
    vnd_pub.msg =  bt_mesh_pub_msg_vnd_pub;
}

#if defined __cplusplus
    }
#endif

