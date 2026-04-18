#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "nrf_esb.h"
#include "nrf_error.h"
#include "nrf_gpio.h"
#include "nrf_drv_clock.h"
#include "nrf_delay.h"
#include "app_error.h"

#define STATUS_LED       13
#define PAYLOAD_LEN      4

static volatile uint32_t s_tx_success = 0;
static volatile uint32_t s_tx_failed  = 0;
static uint8_t s_seq = 0;

void nrf_esb_event_handler(nrf_esb_evt_t const * p_event)
{
    if (p_event->evt_id == NRF_ESB_EVENT_TX_SUCCESS) {
        s_tx_success++;
        nrf_gpio_pin_clear(STATUS_LED);
    } else if (p_event->evt_id == NRF_ESB_EVENT_TX_FAILED) {
        s_tx_failed++;
        nrf_gpio_pin_set(STATUS_LED);
        (void)nrf_esb_flush_tx();
    }
}

static void radio_init(void)
{
    uint8_t base[]   = {0xE7, 0xE7, 0xE7, 0xE7};
    uint8_t prefix[] = {0xE7};

    nrf_esb_config_t cfg = NRF_ESB_LEGACY_CONFIG;
    cfg.mode               = NRF_ESB_MODE_PTX;
    cfg.event_handler      = nrf_esb_event_handler;
    cfg.protocol           = NRF_ESB_PROTOCOL_ESB;
    cfg.payload_length     = PAYLOAD_LEN;
    cfg.bitrate            = NRF_ESB_BITRATE_2MBPS;
    cfg.crc                = NRF_ESB_CRC_8BIT;
    cfg.tx_output_power    = NRF_ESB_TX_POWER_0DBM;
    cfg.retransmit_count   = 0;
    cfg.selective_auto_ack = false;

    APP_ERROR_CHECK(nrf_esb_init(&cfg));
    APP_ERROR_CHECK(nrf_esb_set_base_address_0(base));
    APP_ERROR_CHECK(nrf_esb_set_prefixes(prefix, 1));
    APP_ERROR_CHECK(nrf_esb_set_rf_channel(2));
}

static void send_packet(void)
{
    nrf_esb_payload_t tx_payload;
    memset(&tx_payload, 0, sizeof(tx_payload));
    tx_payload.length = PAYLOAD_LEN;

    tx_payload.data[0] = 0x00;
    tx_payload.data[1] = 0x00;
    tx_payload.data[2] = s_seq++;
    tx_payload.data[3] = tx_payload.data[0] ^ tx_payload.data[1] ^ tx_payload.data[2];

    if (nrf_esb_write_payload(&tx_payload) != NRF_SUCCESS) {
        (void)nrf_esb_flush_tx();
    }
}

int main(void)
{
    nrf_gpio_cfg_output(STATUS_LED);
    nrf_gpio_pin_set(STATUS_LED);

    APP_ERROR_CHECK(nrf_drv_clock_init());
    nrf_drv_clock_hfclk_request(NULL);
    while (!nrf_drv_clock_hfclk_is_running()) {}

    radio_init();

    while (1) {
        send_packet();
        nrf_delay_ms(100);

        if ((s_tx_success & 0x07u) == 0u) {
            nrf_gpio_pin_toggle(STATUS_LED);
        }
    }
}
