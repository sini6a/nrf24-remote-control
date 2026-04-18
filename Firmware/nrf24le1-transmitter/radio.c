#include "radio.h"
#include "config.h"
#include <reg24le1.h>

/*
 * RFCON (0xE8) bits:
 *   bit 0 = RFCE  (Chip Enable)
 *   bit 1 = RFCSN (SPI CS, active low)
 *   bit 2 = RFCKEN (Radio clock enable)
 *
 * SPIR registers used for the integrated radio SPI:
 *   SPIRCON0 (0xE4), SPIRSTAT (0xE6), SPIRDAT (0xE7)
 */

#define RFCON_RFCE      0x01
#define RFCON_RFCSN     0x02
#define RFCON_RFCKEN    0x04

#define SPIRSTAT_RX_READY   SPIRSTAT_IRQ_RX_FIFO_READY

static uint8_t spi_error = 0;

/*
 * The nRF24LE1 has the nRF24L01+ radio core connected internally.
 * The internal SPI to the radio is accessed via RFCON / RFDAT SFRs,
 * and CE/CSN are controlled via RFCKEN and the radio power registers.
 *
 * SPI to radio uses: SPISEL2 = 1 to select internal radio SPI.
 */

/* ?????? Low-level internal SPI ????????????????????????????????????????????????????????????????????????????????????????????????????????? */

static void spi_begin(void) {
    spi_error = 0;
    RFCON &= ~RFCON_RFCSN;      /* CSN low = select radio */
}

static void spi_end(void) {
    RFCON |= RFCON_RFCSN;       /* CSN high = deselect */
}

static uint8_t spi_byte(uint8_t tx) {
    uint16_t timeout = 60000;
    SPIRDAT = tx;
    while (!(SPIRSTAT & SPIRSTAT_RX_READY) && --timeout);
    if (!timeout) {
        spi_error = 1;
        return 0;
    }
    IRCON_SB_RFRDY = 0;  /* Clear RF SPI ready IRQ flag */
    return SPIRDAT;
}

/* ?????? Register access ????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????? */

static uint8_t read_reg(uint8_t reg) {
    uint8_t val;
    spi_begin();
    spi_byte(SPI_READ_REG | (reg & 0x1F));
    val = spi_byte(SPI_NOP);
    spi_end();
    if (spi_error) return 0;
    return val;
}

static void write_reg(uint8_t reg, uint8_t val) {
    spi_begin();
    spi_byte(SPI_WRITE_REG | (reg & 0x1F));
    spi_byte(val);
    spi_end();
}

static void write_reg_buf(uint8_t reg, uint8_t *buf, uint8_t len) {
    uint8_t i;
    spi_begin();
    spi_byte(SPI_WRITE_REG | (reg & 0x1F));
    for (i = 0; i < len; i++) {
        spi_byte(buf[i]);
    }
    spi_end();
}

static void flush_tx(void) {
    spi_begin();
    spi_byte(SPI_FLUSH_TX);
    spi_end();
}

static void flush_rx(void) {
    spi_begin();
    spi_byte(SPI_FLUSH_RX);
    spi_end();
}

/* ?????? CE control ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????? */

static void ce_high(void) { RFCON |= RFCON_RFCE;  }
static void ce_low(void)  { RFCON &= ~RFCON_RFCE; }

/* ?????? Delay helpers ??????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????? */

/* Rough microsecond busy-wait at 16 MHz */
static void delay_us(uint8_t us) {
    volatile uint8_t i;
    while (us--) {
        for (i = 0; i < 4; i++);
    }
}

/* ?????? Public API ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????? */

void radio_init(void) {
    uint8_t addr[5] = {ADDR_BYTE0, ADDR_BYTE1, ADDR_BYTE2,
                       ADDR_BYTE3, ADDR_BYTE4};

    /*
     * Enable integrated RF SPI.
     * nRF24LE1 errata sequence: first write 0x02, then enable RF clock.
     */
    RFCON = RFCON_RFCSN;
    RFCON = RFCON_RFCKEN;
    RFCON |= RFCON_RFCSN; /* CSN idle high */

    ce_low();

    /* Interop profile: TX, power-up, CRC enabled, 16-bit CRC */
    write_reg(NRF_CONFIG, CONFIG_PWR_UP | CONFIG_PRIM_TX | CONFIG_CRC_EN | CONFIG_CRC_2BYTE);

    /* Auto-ack enabled on pipe 0 */
    write_reg(NRF_EN_AA, 0x01);

    /* Enable RX pipe 0 (for receiving ACKs) */
    write_reg(NRF_EN_RXADDR, 0x01);

    /* 5-byte address width */
    write_reg(NRF_SETUP_AW, 0x03);

    /* Auto-retransmit: delay + count from config.h */
    write_reg(NRF_SETUP_RETR, ((RF_RETRY_DELAY & 0x0F) << 4) | (RF_RETRIES & 0x0F));

    /* RF channel */
    write_reg(NRF_RF_CH, RF_CHANNEL);

    /* Interop profile: 1 Mbps, 0 dBm */
    write_reg(NRF_RF_SETUP, RF_SETUP_1MBPS | RF_SETUP_0DBM);

    /* Set TX and RX0 addresses (must match for ACK) */
    write_reg_buf(NRF_TX_ADDR,    addr, 5);
    write_reg_buf(NRF_RX_ADDR_P0, addr, 5);

    /* Fixed payload size */
    write_reg(NRF_RX_PW_P0, RF_PAYLOAD_SIZE);

    /* No dynamic payload, no features needed */
    write_reg(NRF_DYNPD,   0x00);
    write_reg(NRF_FEATURE, 0x00);

    /* Clear any stale status flags */
    write_reg(NRF_STATUS, (STATUS_TX_DS | STATUS_MAX_RT));
    flush_tx();

    /* Power-up settling time: 1.5 ms */
    delay_us(200);
    delay_us(200);
    delay_us(200);
    delay_us(200);
    delay_us(200);
    delay_us(200);
    delay_us(200);
    delay_us(200);
}

uint8_t radio_transmit(uint8_t *payload, uint8_t len) {
    uint8_t status;
    uint8_t i;
    uint16_t timeout = 60000;

    /* Clear TX flags */
    write_reg(NRF_STATUS, STATUS_TX_DS | STATUS_MAX_RT);
    flush_tx();

    /* Write payload to TX FIFO */
    spi_begin();
    spi_byte(SPI_W_TX_PAYLOAD);
    for (i = 0; i < len; i++) {
        spi_byte(payload[i]);
    }
    spi_end();
    if (spi_error) {
        return RADIO_TX_SPI_ERR;
    }

    /* Pulse CE for >= 10 ??s to start transmission */
    ce_high();
    delay_us(15);
    ce_low();

    /* Wait for TX_DS (sent + acked) or MAX_RT (failed) */
    do {
        status = read_reg(NRF_STATUS);
        if (spi_error) {
            return RADIO_TX_SPI_ERR;
        }
        if (--timeout == 0) {
            return RADIO_TX_TIMEOUT;
        }
    } while (!(status & (STATUS_TX_DS | STATUS_MAX_RT)));

    /* Clear flags */
    write_reg(NRF_STATUS, STATUS_TX_DS | STATUS_MAX_RT);

    if (status & STATUS_MAX_RT) {
        flush_tx();     /* Must flush on max retries */
        return RADIO_TX_MAX_RT;
    }

    return RADIO_TX_OK;
}

void radio_init_rx(void) {
    uint8_t addr[5] = {ADDR_BYTE0, ADDR_BYTE1, ADDR_BYTE2,
                       ADDR_BYTE3, ADDR_BYTE4};

    RFCON = RFCON_RFCSN;
    RFCON = RFCON_RFCKEN;
    RFCON |= RFCON_RFCSN;

    ce_low();

    /*
     * Mask all nRF24 IRQ outputs in RX test mode. We poll STATUS in firmware,
     * so IRQ pin is not needed and masking avoids unexpected IRQ/reset behavior.
     */
    write_reg(NRF_CONFIG, CONFIG_PWR_UP | CONFIG_PRIM_RX | CONFIG_CRC_EN | CONFIG_CRC_2BYTE | 0x70);
    write_reg(NRF_EN_AA, 0x01);
    write_reg(NRF_EN_RXADDR, 0x01);
    write_reg(NRF_SETUP_AW, 0x03);
    write_reg(NRF_SETUP_RETR, ((RF_RETRY_DELAY & 0x0F) << 4) | (RF_RETRIES & 0x0F));
    write_reg(NRF_RF_CH, RF_CHANNEL);
    write_reg(NRF_RF_SETUP, RF_SETUP_1MBPS | RF_SETUP_0DBM);

    write_reg_buf(NRF_RX_ADDR_P0, addr, 5);
    write_reg(NRF_RX_PW_P0, RF_PAYLOAD_SIZE);
    write_reg(NRF_DYNPD, 0x00);
    write_reg(NRF_FEATURE, 0x00);

    write_reg(NRF_STATUS, STATUS_RX_DR | STATUS_TX_DS | STATUS_MAX_RT);
    flush_rx();
    flush_tx();

    delay_us(200);
    delay_us(200);
    delay_us(200);
    delay_us(200);
    delay_us(200);
    delay_us(200);
    delay_us(200);
    delay_us(200);

    ce_high();
}

uint8_t radio_receive(uint8_t *payload, uint8_t len) {
    uint8_t status;
    uint8_t i;

    if (len != RF_PAYLOAD_SIZE) {
        return 0;
    }

    status = read_reg(NRF_STATUS);
    if (spi_error) {
        return 0;
    }
    if (!(status & STATUS_RX_DR)) {
        return 0;
    }

    spi_begin();
    spi_byte(SPI_R_RX_PAYLOAD);
    for (i = 0; i < len; i++) {
        payload[i] = spi_byte(SPI_NOP);
    }
    spi_end();
    if (spi_error) {
        return 0;
    }

    write_reg(NRF_STATUS, STATUS_RX_DR);
    return 1;
}

void radio_power_down(void) {
    uint8_t cfg;
    ce_low();
    /* Clear only PWR_UP, preserve CRC/mode bits configured at init */
    cfg = read_reg(NRF_CONFIG);
    if (!spi_error) {
        write_reg(NRF_CONFIG, cfg & ~CONFIG_PWR_UP);
    }
}

void radio_power_up(void) {
    uint8_t cfg;
    cfg = read_reg(NRF_CONFIG);
    write_reg(NRF_CONFIG, cfg | CONFIG_PWR_UP);
    /* Settling: ~1.5 ms */
    delay_us(200);
    delay_us(200);
    delay_us(200);
    delay_us(200);
    delay_us(200);
    delay_us(200);
    delay_us(200);
    delay_us(200);
}

uint8_t radio_read_register(uint8_t reg) {
    return read_reg(reg);
}

