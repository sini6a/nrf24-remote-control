#ifndef RADIO_H
#define RADIO_H

#include <stdint.h>

/*
 * nRF24LE1 radio driver
 * Uses the built-in nRF24L01+ core via internal SPI (no external wiring needed).
 * The nRF24LE1 has the radio mapped to internal SFR-controlled SPI.
 */

/* nRF24L01+ register addresses */
#define NRF_CONFIG          0x00
#define NRF_EN_AA           0x01    /* Auto-acknowledgement */
#define NRF_EN_RXADDR       0x02
#define NRF_SETUP_AW        0x03    /* Address width */
#define NRF_SETUP_RETR      0x04    /* Retransmit config */
#define NRF_RF_CH           0x05
#define NRF_RF_SETUP        0x06
#define NRF_STATUS          0x07
#define NRF_TX_ADDR         0x10
#define NRF_RX_ADDR_P0      0x0A
#define NRF_RX_PW_P0        0x11
#define NRF_FIFO_STATUS     0x17
#define NRF_DYNPD           0x1C
#define NRF_FEATURE         0x1D

/* SPI commands */
#define SPI_READ_REG        0x00
#define SPI_WRITE_REG       0x20
#define SPI_FLUSH_TX        0xE1
#define SPI_FLUSH_RX        0xE2
#define SPI_W_TX_PAYLOAD    0xA0
#define SPI_R_RX_PAYLOAD    0x61
#define SPI_NOP             0xFF

/* CONFIG register bits */
#define CONFIG_PWR_UP       (1 << 1)
#define CONFIG_PRIM_TX      (0 << 0)   /* TX mode */
#define CONFIG_PRIM_RX      (1 << 0)   /* RX mode */
#define CONFIG_CRC_EN       (1 << 3)
#define CONFIG_CRC_2BYTE    (1 << 2)

/* RF_SETUP: data rate */
#define RF_SETUP_250KBPS    (1 << 5)   // RF_DR_LOW
#define RF_SETUP_2MBPS      (1 << 3)   // RF_DR_HIGH
#define RF_SETUP_1MBPS      0x00       // both bits = 0

/* RF_SETUP: TX power */
#define RF_SETUP_0DBM       (0x03 << 1)

/* STATUS bits */
#define STATUS_RX_DR        (1 << 6)   /* Data ready RX FIFO interrupt */
#define STATUS_TX_DS        (1 << 5)   /* Transmit data sent */
#define STATUS_MAX_RT       (1 << 4)   /* Max retransmits reached */
#define STATUS_TX_FULL      (1 << 0)

void radio_init(void);
/* Return codes for radio_transmit() */
#define RADIO_TX_OK         0
#define RADIO_TX_MAX_RT     1
#define RADIO_TX_TIMEOUT    2
#define RADIO_TX_SPI_ERR    3
uint8_t radio_transmit(uint8_t *payload, uint8_t len);
void radio_init_rx(void);
uint8_t radio_receive(uint8_t *payload, uint8_t len);
void radio_power_down(void);
void radio_power_up(void);
uint8_t radio_read_register(uint8_t reg);

#endif /* RADIO_H */
