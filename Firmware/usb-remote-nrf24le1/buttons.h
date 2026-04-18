#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdint.h>

/*
 * Button module for 15 buttons across P0 and P1.
 *
 * All GPIOs are configured as inputs with internal pull-up.
 * A pressed button pulls the pin to GND → reads as 0.
 * We invert so that bit=1 means PRESSED in the returned state.
 *
 * Return format (matches packet layout in config.h):
 *   btn_state[0]: buttons 1–8  (bit0=btn1 … bit7=btn8)
 *   btn_state[1]: buttons 9–15 (bit0=btn9 … bit6=btn15)
 */

void     buttons_init(void);
uint8_t  buttons_changed(void);             /* Returns 1 if state changed since last call */
void     buttons_get_state(uint8_t *b0, uint8_t *b1);

#endif /* BUTTONS_H */
