#ifndef SDK_CONFIG_H
#define SDK_CONFIG_H

/* ── Clock ────────────────────────────────────────────────────── */
#define CLOCK_ENABLED                           1
#define NRF_CLOCK_ENABLED                       1   /* required by NRF_MODULE_ENABLED(NRF_CLOCK) in nrf_drv_clock.c */
#define NRFX_CLOCK_ENABLED                      1
#define NRFX_CLOCK_CONFIG_IRQ_PRIORITY          6
/* LF clock source: 0=RC, 1=XTAL(32.768kHz), 2=Synth(from 32MHz HFXO)
 * Using RC — no separate 32.768 kHz crystal required.
 * The 32 MHz HFXO is started automatically by the USB stack. */
#define NRFX_CLOCK_CONFIG_LF_SRC               0
#define CLOCK_CONFIG_LF_SRC                     0   /* legacy name, same value */

/* ── Power ────────────────────────────────────────────────────── */
#define POWER_ENABLED                           1
#define NRFX_POWER_ENABLED                      1
#define NRFX_POWER_CONFIG_IRQ_PRIORITY          6
/* VDDH connected to USB 5V — use DC/DC regulators for efficiency.
 * REG1 (DCDCENHV): 5V VDDH → 3.3V VDD
 * REG0 (DCDCEN):   3.3V VDD → 1.8V core */
#define NRFX_POWER_CONFIG_DEFAULT_DCDCEN        0   /* LDO — no external inductor required */
#define NRFX_POWER_CONFIG_DEFAULT_DCDCENHV      0   /* LDO — no external inductor required */

/* ── USBD (hardware layer) ────────────────────────────────────── */
#define NRFX_USBD_ENABLED                       1
#define NRFX_USBD_CONFIG_IRQ_PRIORITY           6
#define NRFX_USBD_CONFIG_DMASCHEDULER_MODE      0
#define NRFX_USBD_CONFIG_DMASCHEDULER_ISO_BOOST 1
/* Legacy (non-prefixed) names used directly by app_usbd.c */
#define USBD_CONFIG_IRQ_PRIORITY                6
#define POWER_CONFIG_IRQ_PRIORITY               6

/* ── USB stack ────────────────────────────────────────────────── */
#define APP_USBD_ENABLED                        1
#define APP_USBD_VID                            0x1915   /* Nordic Semiconductor */
#define APP_USBD_PID                            0x520B
#define APP_USBD_DEVICE_VER_MAJOR               1
#define APP_USBD_DEVICE_VER_MINOR               0
#define APP_USBD_DEVICE_VER_SUB                 0
#define APP_USBD_CONFIG_SELF_POWERED            1
#define APP_USBD_STRING_ID_MANUFACTURER         1
#define APP_USBD_STRING_ID_PRODUCT              2
#define APP_USBD_STRING_ID_SERIAL               3
#define APP_USBD_STRING_ID_CONFIGURATION        4
#define APP_USBD_CONFIG_EVENT_QUEUE_ENABLE      1
#define APP_USBD_CONFIG_EVENT_QUEUE_SIZE        16
#define APP_USBD_CONFIG_EP_0_SIZE               64
#define APP_USBD_CONFIG_MAX_POWER               100   /* mA */
#define APP_USBD_CONFIG_POWER_EVENTS_PROCESS    1
#define APP_USBD_CONFIG_SOF_HANDLING_MODE       1
#define APP_USBD_CONFIG_SOF_TIMESTAMP_PROVIDE   0
#define APP_USBD_CONFIG_DESC_STRING_SIZE        31

/* USB string descriptors */
#define APP_USBD_STRINGS_LANGIDS        APP_USBD_LANG_AND_SUBLANG(APP_USBD_LANG_ENGLISH, APP_USBD_SUBLANG_ENGLISH_US)
#define APP_USBD_STRINGS_MANUFACTURER   APP_USBD_STRING_DESC("Custom")
#define APP_USBD_STRINGS_PRODUCT        APP_USBD_STRING_DESC("nRF24 Remote HID")
#define APP_USBD_STRING_SERIAL          APP_USBD_STRING_DESC("000000000000")
#define APP_USBD_STRINGS_CONFIGURATION  APP_USBD_STRING_DESC("Default Configuration")
/* No user-defined extra strings. The empty definition is required so that
 * (APP_USBD_STRINGS_USER 0) evaluates to (0) in APP_USBD_STRINGS_NUM. */
#define APP_USBD_STRINGS_USER

/* ── USB HID ──────────────────────────────────────────────────── */
#define APP_USBD_HID_ENABLED                    1
#define APP_USBD_HID_DEFAULT_IDLE_RATE          0
#define APP_USBD_HID_REPORT_IDLE_TABLE_SIZE     4
#define APP_USBD_HID_GENERIC_ENABLED            1

/* ── Queue (used internally by HID generic IN report buffering)── */
#define NRF_QUEUE_ENABLED                       1

/* ── ESB ──────────────────────────────────────────────────────── */
#define ESB_ENABLED                             1
#define ESB_MAX_PAYLOAD_LENGTH                  4
#define ESB_PIPE_COUNT                          1
#define ESB_DYNAMIC_PAYLOAD_ENABLED             0

/* ── CDC ACM — virtual serial port for debug output ──────────── */
#define APP_USBD_CDC_ACM_ENABLED                1
#define APP_USBD_CDC_ACM_ZLP_ON_EPSIZE_WRITE    1
#define APP_USBD_CDC_ACM_BUFFERS_UART_FLOW_CONTROL 0
#define APP_USBD_CDC_ACM_CONFIG_LOG_ENABLED     0

/* ── Logging (disabled — debug via CDC serial port) ───────────── */
#define NRF_LOG_ENABLED                         0
#define NRF_LOG_DEFAULT_LEVEL                   0
#define APP_USBD_CONFIG_LOG_ENABLED             0

/* ── App error ────────────────────────────────────────────────── */
#define APP_ERROR_HANDLER_ENABLED               1

#endif /* SDK_CONFIG_H */
