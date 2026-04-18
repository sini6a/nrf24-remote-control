#ifndef HID_DESCRIPTOR_H
#define HID_DESCRIPTOR_H

#include <stdint.h>

/* Interface numbers in composite device */
#define HID_KBD_INTERFACE       0
#define HID_CONS_INTERFACE      1

/* Keyboard input report length (boot keyboard) */
#define HID_KEYBOARD_REPORT_LEN 8
/* Consumer control input report length (single 16-bit usage) */
#define HID_CONSUMER_REPORT_LEN 2

/* Standard boot keyboard report descriptor */
static const uint8_t hid_descriptor_kbd[] = {
    0x05, 0x01,       /* Usage Page (Generic Desktop) */
    0x09, 0x06,       /* Usage (Keyboard) */
    0xA1, 0x01,       /* Collection (Application) */
    0x05, 0x07,       /* Usage Page (Keyboard/Keypad) */
    0x19, 0xE0,       /* Usage Minimum (224) */
    0x29, 0xE7,       /* Usage Maximum (231) */
    0x15, 0x00,       /* Logical Minimum (0) */
    0x25, 0x01,       /* Logical Maximum (1) */
    0x75, 0x01,       /* Report Size (1) */
    0x95, 0x08,       /* Report Count (8) */
    0x81, 0x02,       /* Input (Data,Var,Abs) - modifiers */
    0x75, 0x08,       /* Report Size (8) */
    0x95, 0x01,       /* Report Count (1) */
    0x81, 0x03,       /* Input (Const,Var,Abs) - reserved */
    0x75, 0x08,       /* Report Size (8) */
    0x95, 0x06,       /* Report Count (6) */
    0x15, 0x00,       /* Logical Minimum (0) */
    0x26, 0xE7, 0x00, /* Logical Maximum (231) */
    0x05, 0x07,       /* Usage Page (Keyboard/Keypad) */
    0x19, 0x00,       /* Usage Minimum (0) */
    0x29, 0xE7,       /* Usage Maximum (231) */
    0x81, 0x00,       /* Input (Data,Array,Abs) - keys */
    0xC0              /* End Collection */
};

/* Consumer control report descriptor (single 16-bit usage code) */
static const uint8_t hid_descriptor_cons[] = {
    0x05, 0x0C,       /* Usage Page (Consumer) */
    0x09, 0x01,       /* Usage (Consumer Control) */
    0xA1, 0x01,       /* Collection (Application) */
    0x15, 0x00,       /* Logical Minimum (0) */
    0x26, 0xFF, 0x03, /* Logical Maximum (1023) */
    0x19, 0x00,       /* Usage Minimum (0) */
    0x2A, 0xFF, 0x03, /* Usage Maximum (1023) */
    0x75, 0x10,       /* Report Size (16) */
    0x95, 0x01,       /* Report Count (1) */
    0x81, 0x00,       /* Input (Data,Array,Abs) */
    0xC0              /* End Collection */
};

#endif /* HID_DESCRIPTOR_H */
