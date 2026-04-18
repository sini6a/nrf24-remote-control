#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>

/*
 * Serial debug output - TX only, 19200 8-N-1 at 16 MHz.
 *
 * UART TX pin (hardware-fixed, depends on package):
 *   24-pin: P0.5
 *   32-pin: P0.3
 *   48-pin: P1.0
 * config.h adjusts button masks automatically based on package type.
 *
 * Usage:
 *   Define DEBUG in config.h (or via compiler flag -DDEBUG) to enable.
 *   When DEBUG is not defined all macros compile to nothing.
 */

#ifdef DEBUG

void debug_init(void);
void debug_str(const char *s);
void debug_hex8(uint8_t v);
void debug_uint8(uint8_t v);
void debug_newline(void);

/* Convenience: label + hex byte, then newline */
#define DBG_STR(s)          debug_str(s)
#define DBG_HEX(label, v)   do { debug_str(label); debug_hex8(v); debug_newline(); } while(0)
#define DBG_DEC(label, v)   do { debug_str(label); debug_uint8(v); debug_newline(); } while(0)
#define DBG_NL()            debug_newline()

#else /* DEBUG not defined - strip everything */

#define debug_init()        do {} while(0)
#define DBG_STR(s)          do {} while(0)
#define DBG_HEX(label, v)   do {} while(0)
#define DBG_DEC(label, v)   do {} while(0)
#define DBG_NL()            do {} while(0)

#endif /* DEBUG */

#endif /* DEBUG_H */

