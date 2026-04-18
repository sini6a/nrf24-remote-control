#include "buttons.h"
#include "config.h"
#include <reg24le1.h>

/*
 * Debounce strategy: integrating debounce.
 * Each button has a counter. When pin is low (pressed), counter increments.
 * When pin is high (released), counter decrements.
 * Button is considered pressed when counter reaches DEBOUNCE_COUNT,
 * released when it reaches 0.
 *
 * At ~10 ms polling interval, DEBOUNCE_COUNT=3 gives ~30 ms debounce.
 */

#define DEBOUNCE_COUNT      3
#define NUM_BUTTONS         15

/* Per-button debounce counters */
static uint8_t  db_count[NUM_BUTTONS];

/* Last confirmed stable state, 1=pressed */
static uint8_t  stable[NUM_BUTTONS];

/* Previous stable state (for change detection) */
static uint8_t  prev_stable[NUM_BUTTONS];

static uint8_t button_enabled(uint8_t idx) {
    if (idx < 8) {
        return (P0_BTN_MASK & (1 << idx)) ? 1 : 0;
    }
    return (P1_BTN_MASK & (1 << (idx - 8))) ? 1 : 0;
}

void buttons_init(void) {
    uint8_t i;

    /*
     * Configure P0.0???P0.7 as inputs with pull-up
     * P0DIR = 0xFF means all P0 pins are inputs
     * P0CON bits enable pull-ups (ENPU bits)
     */
    P0DIR = P0_BTN_MASK;
    P1DIR = P1_BTN_MASK;

    /*
     * Enable internal pull-ups.
     * On nRF24LE1, pull-ups are controlled via P0CON / P1CON registers.
     * Bit layout per pin: [7:6]=ENPU+ENPU_PD, [5:4]=dir, etc.
     * Simplest: set all pull-up enable bits.
     */
    // P0CON = 0xFF;
    // P1CON = 0x7F;           /* P1.0???P1.6 only */
    P0CON = 0x00;   // Pull-ups disabled
    P1CON = 0x00;

    /* Clear state */
    for (i = 0; i < NUM_BUTTONS; i++) {
        db_count[i]    = 0;
        stable[i]      = 0;
        prev_stable[i] = 0;
    }
}

/*
 * Read raw pin state for a given button index (0-based).
 * Returns 1 if button is currently pressed (pin reads low).
 */
static uint8_t read_raw(uint8_t idx) {
    uint8_t pin_val;

    if (idx < 8) {
        /* Buttons 1???8: P0.0 ??? P0.7 */
        pin_val = (P0 >> idx) & 1;
    } else if (idx < 14) {
        /* Buttons 9???14: P1.0 ??? P1.5 */
        pin_val = (P1 >> (idx - 8)) & 1;
    } else {
        /* Button 15: P1.6 */
        pin_val = (P1 >> 6) & 1;
    }

    return pin_val;
}

/*
 * Call this periodically (~10 ms). Updates debounce counters.
 * Returns 1 if any button's stable state changed.
 */
uint8_t buttons_changed(void) {
    uint8_t i, raw, changed = 0;

    for (i = 0; i < NUM_BUTTONS; i++) {
        if (!button_enabled(i)) {
            prev_stable[i] = stable[i];
            stable[i] = 0;
            db_count[i] = 0;
            continue;
        }

        prev_stable[i] = stable[i];
        raw = read_raw(i);

        if (raw) {
            if (db_count[i] < DEBOUNCE_COUNT) db_count[i]++;
        } else {
            if (db_count[i] > 0) db_count[i]--;
        }

        /* Update stable state at threshold */
        if (db_count[i] >= DEBOUNCE_COUNT) {
            stable[i] = 1;
        } else if (db_count[i] == 0) {
            stable[i] = 0;
        }

        if (stable[i] != prev_stable[i]) changed = 1;
    }

    return changed;
}

/*
 * Get current debounced state packed into two bytes.
 * b0: buttons 1???8  (bit0=btn1)
 * b1: buttons 9???15 (bit0=btn9, bit6=btn15)
 */
void buttons_get_state(uint8_t *b0, uint8_t *b1) {
    uint8_t i;
    *b0 = 0;
    *b1 = 0;

    for (i = 0; i < 8; i++) {
        if (!button_enabled(i)) continue;
        if (stable[i]) *b0 |= (1 << i);
    }
    for (i = 8; i < NUM_BUTTONS; i++) {
        if (!button_enabled(i)) continue;
        if (stable[i]) *b1 |= (1 << (i - 8));
    }
}
