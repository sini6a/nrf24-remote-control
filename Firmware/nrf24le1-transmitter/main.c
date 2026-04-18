#include <reg24le1.h>
#include <stdint.h>
#include "config.h"
#include "radio.h"
#include "buttons.h"
#include "debug.h"

/* Sequence number included in each packet. */
static uint8_t seq = 0;
static uint8_t sleep_armed = 0;
static uint8_t radio_sleeping = 0;

/* Safe default: keep CPU polling, only power down radio. */
#define USE_DEEP_SLEEP 1

/* Millisecond busy-delay at 16 MHz. */
static void delay_ms(uint8_t ms) {
    volatile uint16_t i;
    while (ms--) {
        for (i = 0; i < 533; i++);
    }
}

static void wakeup_configure_button_pins(void) {
    uint16_t wake_pins = (uint16_t)P0_BTN_MASK | ((uint16_t)P1_BTN_MASK << 8);

    /* Wake on high level from selected pins. */
    OPMCON &= (uint8_t)~OPMCON_WAKEUP_PINS_POLARITY;
    WUOPC0 = (uint8_t)(wake_pins & 0xFFu);
    WUOPC1 = (uint8_t)((wake_pins >> 8) & 0xFFu);

    /* Wake only from WUOP (pins), disable other wake sources. */
    WUCON = (uint8_t)((0x03u << WUCON_WAKE_ON_MISCIRQ_SHIFT) |
                      (0x02u << WUCON_WAKE_ON_WUOPIRQ_SHIFT) |
                      (0x03u << WUCON_WAKE_ON_RTC2_TICK_SHIFT) |
                      (0x03u << WUCON_WAKE_ON_RFIRQ_SHIFT));
}

static void enter_low_power_wait_for_button(void) {
    radio_power_down();
    DBG_STR("sleep\r\n");

    /* Clear stale wake flag and enter register-retention low-power mode. */
    IRCON_SB_WUOPIRQ = 0;
    PWRDWN = (0x04u << PWRDWN_PWR_CNTL_SHIFT); /* register retention mode */

    /* Execution resumes here after pin wake-up. */
    IRCON_SB_WUOPIRQ = 0;
    radio_power_up();
    delay_ms(2);
    DBG_STR("wake\r\n");
}

static void enter_idle_low_power(void) {
    if (!radio_sleeping) {
        radio_power_down();
        radio_sleeping = 1;
        DBG_STR("radio sleep\r\n");
    }
}

static void leave_idle_low_power(void) {
    if (radio_sleeping) {
        radio_power_up();
        delay_ms(2);
        radio_sleeping = 0;
        DBG_STR("radio wake\r\n");
    }
}

/* Build and transmit one state packet (with retries at app level). */
static void send_state(uint8_t b0, uint8_t b1) {
    uint8_t pkt[RF_PAYLOAD_SIZE];
    uint8_t i;
    uint8_t tx_res = RADIO_TX_OK;
    uint8_t tx_ok = 0;

    /* Force unused programmable slots to 0 in outgoing packets. */
    b0 &= (uint8_t)~UNUSED_MASK_B0;
    b1 &= (uint8_t)~UNUSED_MASK_B1;

    pkt[0] = b0;
    pkt[1] = b1;
    pkt[2] = seq++;
    pkt[3] = pkt[0] ^ pkt[1] ^ pkt[2];

    DBG_HEX("btn0=", b0);
    DBG_HEX("btn1=", b1);
    DBG_HEX("pkt[2]=", pkt[2]);

    for (i = 0; i < TX_REPEAT; i++) {
        tx_res = radio_transmit(pkt, RF_PAYLOAD_SIZE);
        if (tx_res == RADIO_TX_OK) {
            tx_ok = 1;
        }
        if (i < (TX_REPEAT - 1)) {
            delay_ms(5);
        }
    }

    if (tx_ok) {
        DBG_STR("tx OK\r\n");
    } else if (tx_res == RADIO_TX_SPI_ERR) {
        DBG_STR("tx SPI_ERR\r\n");
    } else if (tx_res == RADIO_TX_MAX_RT) {
        DBG_STR("tx MAX_RT\r\n");
    } else {
        DBG_STR("tx TIMEOUT\r\n");
    }
}

void main(void) {
    uint8_t b0, b1;
    uint8_t idle_cycles = 0;

    OPMCON &= ~OPMCON_WATCHDOG_RESET_ENABLE;
    CLKCTRL = 0x20; /* External 16 MHz crystal */

    buttons_init();
#if USE_DEEP_SLEEP
    wakeup_configure_button_pins();
#endif
    debug_init();
    DBG_STR("BOOT\r\n");

    radio_init();
    DBG_STR("RADIO OK\r\n");

    while (1) {
        delay_ms(IDLE_SLEEP_MS);

        if (buttons_changed()) {
            leave_idle_low_power();
            idle_cycles = 0;
            sleep_armed = 0;
            buttons_get_state(&b0, &b1);
            send_state(b0, b1);
        } else {
            idle_cycles++;
            if (idle_cycles >= 50) {
#if USE_DEEP_SLEEP
                if (!sleep_armed) {
                    sleep_armed = 1;
                    enter_low_power_wait_for_button();
                    idle_cycles = 0;
                    radio_sleeping = 0;
                }
#else
                enter_idle_low_power();
#endif
            }
        }
    }
}
