/*
 * nRF52820 USB HID Receiver
 *
 * Receives 4-byte Enhanced ShockBurst packets from an nRF24LE1 remote and
 * forwards button states as a USB HID keyboard report.
 *
 * Radio settings (must match the remote firmware):
 *   Bitrate : 1 Mbps  (nRF52820 does NOT support Nordic 250 Kbps)
 *   Channel : 76      (2476 MHz)
 *   Address : 0xE7E7E7E7E7
 *   CRC     : 16-bit
 *
 * Packet layout from remote (4 bytes):
 *   [0] buttons  1??? 8  (bit0=btn1 ??? bit7=btn8), 1=pressed
 *   [1] buttons  9???15  (bit0=btn9 ??? bit6=btn15)
 *   [2] sequence number (0???255, wraps)
 *   [3] XOR checksum   (byte0 ^ byte1 ^ byte2)
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "nrf_esb.h"
#include "nrf_error.h"
#include "nrf_gpio.h"
#include "nrf_drv_clock.h"
#include "app_usbd.h"
#include "app_usbd_hid_generic.h"
#include "app_usbd_cdc_acm.h"
#include "app_error.h"
#include "sdk_config.h"
#include "hid_descriptor.h"

/* ?????? GPIO ????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????? */
#define STATUS_LED  13      /* adjust to match your PCB */

/* ?????? Protocol constants ??????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????? */
#define PAYLOAD_LEN         4
#define HID_IN_QUEUE_DEPTH  1

/* ?????? State ?????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????? */
static volatile bool    pkt_ready    = false;
static          uint8_t pkt_buf[PAYLOAD_LEN];
static          uint8_t last_b0      = 0xFF;
static          uint8_t last_b1      = 0xFF;
static          uint8_t last_seq     = 0xFF;
static          uint8_t hid_kbd_report[HID_KEYBOARD_REPORT_LEN];
static          uint8_t hid_cons_report[HID_CONSUMER_REPORT_LEN];
static const    uint8_t hid_kbd_release[HID_KEYBOARD_REPORT_LEN] = {0};
static const    uint8_t hid_cons_release[HID_CONSUMER_REPORT_LEN] = {0};

/* ?????? Debug counters ??????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????? */
static volatile uint32_t dbg_rx_total    = 0;   /* raw packets from ESB */
static volatile uint32_t dbg_rx_badlen   = 0;   /* wrong length         */
static          uint32_t dbg_bad_chk     = 0;   /* checksum failures    */
static          uint32_t dbg_dup         = 0;   /* duplicates           */
static          uint32_t dbg_hid_sent    = 0;   /* HID reports queued   */
static          uint32_t dbg_hid_fail    = 0;   /* HID queue/send fail  */

/* Keyboard HID usages (USB HID Usage Tables, Keyboard/Keypad page 0x07) */
#define HID_KEY_ENTER       0x28
#define HID_KEY_ESC         0x29
#define HID_KEY_F12         0x45
#define HID_KEY_PAGEUP      0x4B
#define HID_KEY_PAGEDOWN    0x4E
#define HID_KEY_RIGHT       0x4F
#define HID_KEY_LEFT        0x50
#define HID_KEY_DOWN        0x51
#define HID_KEY_UP          0x52
#define HID_KEY_MENU        0x65
#define HID_KEY_F13         0x68

/* Consumer usages (Usage Page 0x0C) */
#define HID_CONS_NONE       0x0000
#define HID_CONS_VOL_UP     0x00E9
#define HID_CONS_VOL_DOWN   0x00EA
#define HID_CONS_POWER      0x0030

/* ?????? HID instance ????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????? */
static void hid_ev_handler(app_usbd_class_inst_t const *p_inst,
                            app_usbd_hid_user_event_t    event)
{
    (void)p_inst;
    (void)event;
}

/*
 * Wrap the raw descriptor bytes into app_usbd_hid_subclass_desc_t so the
 * SDK macro can reference it as a compile-time pointer.
 */
static const app_usbd_hid_subclass_desc_t kbd_desc = {
    .size   = sizeof(hid_descriptor_kbd),
    .type   = APP_USBD_DESCRIPTOR_REPORT,
    .p_data = hid_descriptor_kbd,
};
static const app_usbd_hid_subclass_desc_t *kbd_reps[] = {&kbd_desc};

#define KBD_ENDPOINT_LIST()   (NRF_DRV_USBD_EPIN1)

APP_USBD_HID_GENERIC_GLOBAL_DEF(
    m_hid_kbd,
    HID_KBD_INTERFACE,
    hid_ev_handler,
    KBD_ENDPOINT_LIST(),
    kbd_reps,
    HID_IN_QUEUE_DEPTH,
    0,
    0,
    APP_USBD_HID_SUBCLASS_BOOT,
    APP_USBD_HID_PROTO_KEYBOARD
);

static const app_usbd_hid_subclass_desc_t cons_desc = {
    .size   = sizeof(hid_descriptor_cons),
    .type   = APP_USBD_DESCRIPTOR_REPORT,
    .p_data = hid_descriptor_cons,
};
static const app_usbd_hid_subclass_desc_t *cons_reps[] = {&cons_desc};

#define CONS_ENDPOINT_LIST()  (NRF_DRV_USBD_EPIN4)

APP_USBD_HID_GENERIC_GLOBAL_DEF(
    m_hid_cons,
    HID_CONS_INTERFACE,
    hid_ev_handler,
    CONS_ENDPOINT_LIST(),
    cons_reps,
    HID_IN_QUEUE_DEPTH,
    0,
    0,
    APP_USBD_HID_SUBCLASS_NONE,
    APP_USBD_HID_PROTO_GENERIC
);

/* ?????? CDC ACM ??? virtual serial port for debug ???????????????????????????????????????????????????????????? */
#define CDC_COMM_INTERFACE  2
#define CDC_DATA_INTERFACE  3

static bool cdc_open    = false;   /* host has the port open */
static bool cdc_tx_busy = false;   /* previous write still in flight */
static bool cdc_boot_banner_pending = true;

static void cdc_ev_handler(app_usbd_class_inst_t const *p_inst,
                            app_usbd_cdc_acm_user_event_t event)
{
    switch (event) {
        case APP_USBD_CDC_ACM_USER_EVT_PORT_OPEN:
            cdc_open    = true;
            cdc_tx_busy = false;
            break;
        case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE:
            cdc_open    = false;
            cdc_tx_busy = false;
            break;
        case APP_USBD_CDC_ACM_USER_EVT_TX_DONE:
            cdc_tx_busy = false;
            break;
        default:
            break;
    }
}

APP_USBD_CDC_ACM_GLOBAL_DEF(
    m_cdc,
    cdc_ev_handler,
    CDC_COMM_INTERFACE,             /* control interface   */
    CDC_DATA_INTERFACE,             /* data interface      */
    NRF_DRV_USBD_EPIN2,            /* notification IN EP  */
    NRF_DRV_USBD_EPIN3,            /* data IN EP          */
    NRF_DRV_USBD_EPOUT1,           /* data OUT EP         */
    APP_USBD_CDC_COMM_PROTOCOL_AT_V250
);

/*
 * Send a formatted debug line over the CDC serial port.
 * Silently dropped if the host hasn't opened the port or a
 * previous write is still in flight.
 */
static void cdc_printf(const char *fmt, ...)
{
    if (!cdc_open || cdc_tx_busy) return;

    static char buf[128];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (len > 0) {
        if ((size_t)len > sizeof(buf) - 1) len = sizeof(buf) - 1;
        if (app_usbd_cdc_acm_write(&m_cdc, buf, (size_t)len) == NRF_SUCCESS) {
            cdc_tx_busy = true;
        }
    }
}

/* ?????? USB power/state events ??????????????????????????????????????????????????????????????????????????????????????????????????????????????? */
static void usbd_ev_handler(app_usbd_event_type_t event)
{
    switch (event) {
        case APP_USBD_EVT_POWER_DETECTED:
            if (!nrf_drv_usbd_is_enabled()) {
                app_usbd_enable();
            }
            break;
        case APP_USBD_EVT_POWER_READY:
            app_usbd_start();
            break;
        case APP_USBD_EVT_STARTED:
            nrf_gpio_pin_clear(STATUS_LED);   /* LED on = USB up */
            break;
        case APP_USBD_EVT_POWER_REMOVED:
            app_usbd_stop();
            break;
        case APP_USBD_EVT_STOPPED:
            app_usbd_disable();
            nrf_gpio_pin_set(STATUS_LED);
            break;
        default:
            break;
    }
}

/* ?????? ESB interrupt handler ?????????????????????????????????????????????????????????????????????????????????????????????????????????????????? */
void nrf_esb_event_handler(nrf_esb_evt_t const *p_event)
{
    nrf_esb_payload_t rx;

    if (p_event->evt_id == NRF_ESB_EVENT_RX_RECEIVED) {
        while (nrf_esb_read_rx_payload(&rx) == NRF_SUCCESS) {
            dbg_rx_total++;
            if (rx.length == PAYLOAD_LEN) {
                memcpy(pkt_buf, rx.data, PAYLOAD_LEN);
                pkt_ready = true;
            } else {
                dbg_rx_badlen++;
                cdc_printf("WARN: bad length %u (expected %u)\r\n",
                            rx.length, PAYLOAD_LEN);
            }
        }
    }
}

/* ?????? Radio initialisation ????????????????????????????????????????????????????????????????????????????????????????????????????????????????????? */
static void radio_init(void)
{
    /*
     * Address layout for the ESB library:
     *   base_addr_0[0..3] = bytes [4..1] of the on-air address (MSB first)
     *   prefix[0]         = byte  [0]    of the on-air address
     * Remote sends to 0xE7E7E7E7E7, so base = {0xE7,0xE7,0xE7,0xE7},
     * prefix = 0xE7.
     */
    uint8_t base[]   = {0xE7, 0xE7, 0xE7, 0xE7};
    uint8_t prefix[] = {0xE7};

    nrf_esb_config_t cfg = NRF_ESB_LEGACY_CONFIG;    /* fixed-payload ESB ??? matches nRF24LE1 */
    cfg.mode               = NRF_ESB_MODE_PRX;
    cfg.event_handler      = nrf_esb_event_handler;
    cfg.protocol           = NRF_ESB_PROTOCOL_ESB;   /* NOT DPL ??? nRF24LE1 uses fixed length */
    cfg.payload_length     = PAYLOAD_LEN;             /* must match remote's payload size     */
    cfg.bitrate            = NRF_ESB_BITRATE_1MBPS;
    cfg.crc                = NRF_ESB_CRC_16BIT;
    cfg.tx_output_power    = NRF_ESB_TX_POWER_0DBM;
    cfg.retransmit_count   = 0;
    cfg.selective_auto_ack = false;

    APP_ERROR_CHECK(nrf_esb_init(&cfg));
    APP_ERROR_CHECK(nrf_esb_set_base_address_0(base));
    APP_ERROR_CHECK(nrf_esb_set_prefixes(prefix, 1));
    APP_ERROR_CHECK(nrf_esb_set_rf_channel(2));

    /*
     * The ESB library sets CRCCNF with SKIPADDR=Include (0), meaning it
     * includes the address field in the CRC calculation.  The nRF24L01+
     * hardware computes CRC over PCF + payload only (address excluded).
     * Override CRCCNF here to set SKIPADDR=Skip so the two CRCs match.
     * nrf_esb_start_rx() does not touch CRCCNF, so this value sticks.
     */
    // NRF_RADIO->CRCCNF = (2 << RADIO_CRCCNF_LEN_Pos) |   /* 2-byte CRC          */
//                         (RADIO_CRCCNF_SKIPADDR_Skip
//                          << RADIO_CRCCNF_SKIPADDR_Pos);   /* skip address in CRC */
    APP_ERROR_CHECK(nrf_esb_start_rx());
}

static void kbd_add_key(uint8_t *rep, uint8_t usage)
{
    uint8_t i;
    for (i = 2; i < HID_KEYBOARD_REPORT_LEN; i++) {
        if (rep[i] == usage) return;
        if (rep[i] == 0) {
            rep[i] = usage;
            return;
        }
    }
}

static void build_keyboard_report(uint8_t b0, uint8_t b1, uint8_t *rep)
{
    memset(rep, 0, HID_KEYBOARD_REPORT_LEN);
    /* [0]=modifiers, [1]=reserved, [2..7]=keys */

    if (b0 & (1u << 1)) kbd_add_key(rep, HID_KEY_LEFT);      /* P0.1 Left */
    if (b0 & (1u << 2)) kbd_add_key(rep, HID_KEY_DOWN);      /* P0.2 Down */
    if (b0 & (1u << 3)) kbd_add_key(rep, HID_KEY_PAGEDOWN);  /* P0.3 Program Down */
    if (b0 & (1u << 4)) kbd_add_key(rep, HID_KEY_ESC);       /* P0.4 Back */
    if (b0 & (1u << 7)) kbd_add_key(rep, HID_KEY_PAGEUP);    /* P0.7 Program Up */
    if (b1 & (1u << 0)) kbd_add_key(rep, HID_KEY_MENU);      /* P1.0 Options */
    if (b1 & (1u << 1)) kbd_add_key(rep, HID_KEY_ENTER);     /* P1.1 Enter */
    if (b1 & (1u << 2)) kbd_add_key(rep, HID_KEY_RIGHT);     /* P1.2 Right */
    if (b1 & (1u << 4)) kbd_add_key(rep, HID_KEY_F13);       /* P1.4 Programmable */
    if (b1 & (1u << 5)) kbd_add_key(rep, HID_KEY_F12);       /* P1.5 Power */
    if (b1 & (1u << 6)) kbd_add_key(rep, HID_KEY_UP);        /* P1.6 Up */
}

static void build_consumer_report(uint8_t b0, uint8_t b1, uint8_t *rep)
{
    uint16_t usage = HID_CONS_NONE;
    if (b0 & (1u << 5)) {
        usage = HID_CONS_VOL_UP;
    } else if (b0 & (1u << 6)) {
        usage = HID_CONS_VOL_DOWN;
    } else if (b1 & (1u << 5)) {
        usage = HID_CONS_POWER;
    }
    rep[0] = (uint8_t)(usage & 0xFFu);
    rep[1] = (uint8_t)((usage >> 8) & 0xFFu);
}

static void send_hid_report(app_usbd_hid_generic_t const *inst,
                            const uint8_t *rep, size_t len, const char *tag)
{
    ret_code_t hid_ret = app_usbd_hid_generic_in_report_set(inst, rep, len);
    if (hid_ret == NRF_SUCCESS) {
        dbg_hid_sent++;
        cdc_printf("HID %s #%lu\r\n", tag, (unsigned long)dbg_hid_sent);
    } else {
        dbg_hid_fail++;
        cdc_printf("WARN: HID %s fail ret=%lu fail=%lu\r\n",
                   tag, (unsigned long)hid_ret, (unsigned long)dbg_hid_fail);
    }
}

/* ?????? Packet processing ?????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????? */
static void process_packet(const uint8_t *pkt)
{
    uint8_t b0  = pkt[0];
    uint8_t b1  = pkt[1];
    uint8_t seq = pkt[2];
    uint8_t chk = pkt[3];

    cdc_printf("PKT raw: b0=%02X b1=%02X seq=%u chk=%02X calc=%02X\r\n",
               b0, b1, seq, chk, b0 ^ b1 ^ seq);

    if ((b0 ^ b1 ^ seq) != chk) {
        dbg_bad_chk++;
        cdc_printf("WARN: checksum FAIL (total bad=%lu)\r\n", dbg_bad_chk);
        return;
    }
    if (seq == last_seq) {
        dbg_dup++;
        return;   /* silent ??? duplicates are frequent and noisy */
    }
    last_seq = seq;

    cdc_printf("PKT ok seq=%u b0=%02X b1=%02X rx_total=%lu\r\n",
               seq, b0, b1, (unsigned long)dbg_rx_total);

    if (b0 != last_b0 || b1 != last_b1) {
        last_b0 = b0;
        last_b1 = b1;
        build_keyboard_report(b0, b1, hid_kbd_report);
        build_consumer_report(b0, b1, hid_cons_report);
        send_hid_report(&m_hid_kbd, hid_kbd_report, HID_KEYBOARD_REPORT_LEN, "kbd");
        send_hid_report(&m_hid_cons, hid_cons_report, HID_CONSUMER_REPORT_LEN, "cons");

        /* Always emit explicit key/button release to avoid host-side latch. */
        send_hid_report(&m_hid_kbd, hid_kbd_release, HID_KEYBOARD_REPORT_LEN, "kbd_rel");
        send_hid_report(&m_hid_cons, hid_cons_release, HID_CONSUMER_REPORT_LEN, "cons_rel");
    }
}

/* ?????? Entry point ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????? */
int main(void)
{
    nrf_gpio_cfg_output(STATUS_LED);
    nrf_gpio_pin_set(STATUS_LED);   /* LED off until USB enumerates */

    /* LFCLK ??? required by USB */
    APP_ERROR_CHECK(nrf_drv_clock_init());
    nrf_drv_clock_lfclk_request(NULL);
    while (!nrf_drv_clock_lfclk_is_running()) {}

    /* USB stack ??? HID + CDC composite device */
    static const app_usbd_config_t usbd_cfg = {
        .ev_state_proc = usbd_ev_handler,
    };
    APP_ERROR_CHECK(app_usbd_init(&usbd_cfg));

    app_usbd_class_inst_t const *hid_kbd_inst =
        app_usbd_hid_generic_class_inst_get(&m_hid_kbd);
    APP_ERROR_CHECK(app_usbd_class_append(hid_kbd_inst));

    app_usbd_class_inst_t const *hid_cons_inst =
        app_usbd_hid_generic_class_inst_get(&m_hid_cons);
    APP_ERROR_CHECK(app_usbd_class_append(hid_cons_inst));

    app_usbd_class_inst_t const *cdc_inst =
        app_usbd_cdc_acm_class_inst_get(&m_cdc);
    APP_ERROR_CHECK(app_usbd_class_append(cdc_inst));

    APP_ERROR_CHECK(app_usbd_power_events_enable());

    /* Force HFCLK for ESB radio before init */
    nrf_drv_clock_hfclk_request(NULL);
    while (!nrf_drv_clock_hfclk_is_running()) {}
    cdc_printf("BOOT: receiver radio init...\r\n");
    /* Radio */
    radio_init();
    cdc_printf("BOOT: receiver radio_init OK\r\n");

    /* Main loop */
    uint32_t loop_cnt = 0;
    for (;;) {
        while (app_usbd_event_queue_process()) {}

        if (cdc_boot_banner_pending && cdc_open && !cdc_tx_busy) {
            cdc_printf("READY: receiver running (open after boot supported)\r\n");
            cdc_boot_banner_pending = false;
        }

        if (pkt_ready) {
            pkt_ready = false;
            process_packet(pkt_buf);
        }

        /* Print stats every ~100 000 iterations when port is open */
        if (++loop_cnt >= 100000) {
            loop_cnt = 0;
            cdc_printf("STATS rx=%lu badlen=%lu badchk=%lu dup=%lu hid=%lu fail=%lu\r\n",
                       (unsigned long)dbg_rx_total,
                       (unsigned long)dbg_rx_badlen,
                       (unsigned long)dbg_bad_chk,
                       (unsigned long)dbg_dup,
                       (unsigned long)dbg_hid_sent,
                       (unsigned long)dbg_hid_fail);
        }
    }
}








