#ifndef CONFIG_H
#define CONFIG_H

#include <reg24le1.h>  /* Needed for NRF24LE1_PACKAGE_* defines used below */

/*
 * nRF24LE1 Remote Control - Configuration
 * 15 push buttons, pulled high when pressed (active high, external pull-up)
 * Transmits to nRF52820 receiver via Enhanced ShockBurst
 */

/* Debug output */
/* Uncomment to enable serial debug output (UART TX, 19200 8-N-1).
 * UART TX pin depends on package:
 *   24-pin: P0.5
 *   32-pin: P0.3
 *   48-pin: P1.0
 */
/* #define DEBUG */

/* RF channel & address */
#define RF_CHANNEL          2           /* Legacy nRF24 default channel for interoperability tests */
#define RF_PAYLOAD_SIZE     4           /* 4 bytes: 2 button state + 1 seq + 1 checksum */
#define RF_RETRIES          10
#define RF_RETRY_DELAY      2           /* 750 us between retries (value * 250us + 250us) */

/* 5-byte pipe address shared with receiver */
#define ADDR_BYTE0          0xE7
#define ADDR_BYTE1          0xE7
#define ADDR_BYTE2          0xE7
#define ADDR_BYTE3          0xE7
#define ADDR_BYTE4          0xE7

/* Timing */
#define DEBOUNCE_MS         20          /* Debounce window in milliseconds */
#define IDLE_SLEEP_MS       10          /* Sleep between polls when no change */
#define TX_REPEAT           3           /* Send packet N times on button change */

/*
 * Button pins:
 * P0.0 = Button 1     P1.0 = Button 7
 * P0.1 = Button 2     P1.1 = Button 8
 * P0.2 = Button 3     P1.2 = Button 9
 * P0.3 = Button 4     P1.3 = Button 10
 * P0.4 = Button 5     P1.4 = Button 11
 * P0.5 = Button 6     P1.5 = Button 12
 * P0.6 = Button 13    P0.7 = Button 14
 * P1.6 = Button 15
 */

/* Masks for used pins.
 * In DEBUG mode, exclude the hardware UART TX pin from button scanning.
 */
#ifdef DEBUG
#  if (__TARG_PACKAGE_TYPE == NRF24LE1_PACKAGE_24_PIN)
#    define P0_BTN_MASK     0xDF        /* Exclude P0.5 (UART TX) */
#    define P1_BTN_MASK     0x7F
#  elif (__TARG_PACKAGE_TYPE == NRF24LE1_PACKAGE_32_PIN)
#    define P0_BTN_MASK     0xF7        /* Exclude P0.3 (UART TX) */
#    define P1_BTN_MASK     0x7F
#  else
#    define P0_BTN_MASK     0xFF
#    define P1_BTN_MASK     0x7E        /* Exclude P1.0 (UART TX) */
#  endif
#else
#  define P0_BTN_MASK       0xFF
#  define P1_BTN_MASK       0x7F
#endif

/* Packet layout:
 * Byte 0: buttons 1-8
 * Byte 1: buttons 9-15 (bit7 = 0)
 * Byte 2: sequence number
 * Byte 3: XOR checksum of bytes 0-2
 */

/*
 * Pin to function mapping (32-pin package):
 * P0.0 = Programmable (unused)
 * P0.1 = Left
 * P0.2 = Down
 * P0.3 = Program Down
 * P0.4 = Back
 * P0.5 = Volume Up
 * P0.6 = Volume Down
 * P0.7 = Program Up
 * P1.0 = Options
 * P1.1 = Enter
 * P1.2 = Right
 * P1.3 = Programmable (unused)
 * P1.4 = Programmable
 * P1.5 = Power
 * P1.6 = Up
 */
#define UNUSED_MASK_B0      (1u << 0)   /* P0.0 */
#define UNUSED_MASK_B1      (1u << 3)   /* P1.3 */

#endif /* CONFIG_H */

