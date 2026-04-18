#include <reg24le1.h>
#include <stdint.h>
#include "config.h"
#include "radio.h"
#include "debug.h"

#define RX_BRINGUP_ONLY 0

static void delay_ms(uint8_t ms) {
    volatile uint16_t i;
    while (ms--) {
        for (i = 0; i < 533; i++);
    }
}

void main(void) {
    uint8_t pkt[RF_PAYLOAD_SIZE];
    uint8_t rstreas;
    uint16_t rx_count = 0;
    uint16_t hb = 0;

    OPMCON &= ~OPMCON_WATCHDOG_RESET_ENABLE;
    debug_init();
    DBG_STR("RX preclk: CLKCTRL=");
    debug_hex8(CLKCTRL);
    DBG_NL();

    /* Switch to external 16 MHz crystal, same as TX firmware */
    CLKCTRL = 0x20;
    delay_ms(10);
    DBG_STR("RX postclk: CLKCTRL=");
    debug_hex8(CLKCTRL);
    DBG_NL();
    DBG_STR("RX CLK: CLKCTRL=");
    debug_hex8(CLKCTRL);
    DBG_STR(" CLKLFCTRL=");
    debug_hex8(CLKLFCTRL);
    DBG_NL();

    /* Boot burst to make startup visible even if terminal opens late */
    DBG_STR("RX boot 1\r\n");
    delay_ms(100);
    DBG_STR("RX boot 2\r\n");
    delay_ms(100);
    DBG_STR("RX boot 3\r\n");
    delay_ms(100);

    rstreas = RSTREAS;
    DBG_STR("RX BOOT: RSTREAS=");
    debug_hex8(rstreas);
    DBG_NL();

#if RX_BRINGUP_ONLY
    DBG_STR("RX bringup only: radio init skipped\r\n");
    while (1) {
        DBG_STR("RX UART alive\r\n");
        delay_ms(200);
    }
#else
    radio_init_rx();
    DBG_STR("RX: radio_init_rx OK\r\n");
    DBG_STR("RX: CONFIG=");
    debug_hex8(radio_read_register(NRF_CONFIG));
    DBG_STR(" CH=");
    debug_hex8(radio_read_register(NRF_RF_CH));
    DBG_STR(" RFSET=");
    debug_hex8(radio_read_register(NRF_RF_SETUP));
    DBG_NL();
    DBG_STR("RX: waiting for packets...\r\n");

    while (1) {
        if (radio_receive(pkt, RF_PAYLOAD_SIZE)) {
            rx_count++;
            DBG_STR("RX pkt ");
            debug_uint8((uint8_t)(rx_count & 0xFF));
            DBG_STR(": ");
            debug_hex8(pkt[0]); DBG_STR(" ");
            debug_hex8(pkt[1]); DBG_STR(" ");
            debug_hex8(pkt[2]); DBG_STR(" ");
            debug_hex8(pkt[3]); DBG_NL();
        }

        hb++;
        if (hb >= 200) { /* ~1 s at 5 ms loop delay */
            hb = 0;
            DBG_STR("RX alive cnt=");
            debug_uint8((uint8_t)(rx_count & 0xFF));
            DBG_STR(" STATUS=");
            debug_hex8(radio_read_register(NRF_STATUS));
            DBG_STR(" FIFO=");
            debug_hex8(radio_read_register(NRF_FIFO_STATUS));
            DBG_NL();
        }
        delay_ms(5);
    }
#endif
}
