#include "config.h"   /* pulls in #define DEBUG if enabled */
#include "debug.h"

#ifdef DEBUG

#include <reg24le1.h>
#include "nRF24LE1_SDK/include/uart.h"

/* Init UART: 9600 8-N-1, TX only */
void debug_init(void)
{
    /* Ensure hardware UART TX pin is configured as output. */
#if (__TARG_PACKAGE_TYPE == NRF24LE1_PACKAGE_24_PIN)
    P0DIR &= ~(1 << 5);   /* P0.5 */
#elif (__TARG_PACKAGE_TYPE == NRF24LE1_PACKAGE_32_PIN)
    P0DIR &= ~(1 << 3);   /* P0.3 */
#else
    P1DIR &= ~(1 << 0);   /* P1.0 */
#endif

    uart_configure_8_n_1_9600();
    debug_str("\r\n--- DEBUG BOOT ---\r\n");
}

/* ── Transmit one character, wait until sent ──────────────────── */
static void tx(char c)
{
    uart_send(c);
    while (!uart_tx_data_sent());
    /* Clear TI flag for next byte */
    S0CON &= ~0x02;   /* S0CON_SB_TI0 */
}

/* ── Send null-terminated string ──────────────────────────────── */
void debug_str(const char *s)
{
    while (*s)
        tx(*s++);
}

/* ── Send byte as two hex digits ──────────────────────────────── */
void debug_hex8(uint8_t v)
{
    static const char hex[] = "0123456789ABCDEF";
    tx('0');
    tx('x');
    tx(hex[(v >> 4) & 0x0F]);
    tx(hex[v & 0x0F]);
}

/* ── Send byte as decimal (0–255) ─────────────────────────────── */
void debug_uint8(uint8_t v)
{
    uint8_t hundreds = v / 100;
    uint8_t tens     = (v % 100) / 10;
    uint8_t units    = v % 10;

    if (hundreds)            tx('0' + hundreds);
    if (hundreds || tens)    tx('0' + tens);
    tx('0' + units);
}

/* ── CR+LF ────────────────────────────────────────────────────── */
void debug_newline(void)
{
    tx('\r');
    tx('\n');
}

#endif /* DEBUG */
