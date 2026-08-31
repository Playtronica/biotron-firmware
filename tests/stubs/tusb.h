#ifndef TEST_TUSB_H
#define TEST_TUSB_H

#include <stdint.h>
#include <stdbool.h>

#define CFG_TUD_CDC 1
#define CFG_TUD_MSC 0
#define CFG_TUD_HID 0
#define CFG_TUD_MIDI 2
#define CFG_TUD_VENDOR 0
#define CFG_TUD_ENDPOINT0_SIZE 64
#define CFG_TUSB_MCU 0

#define OPT_MCU_LPC175X_6X 1
#define OPT_MCU_LPC177X_8X 2
#define OPT_MCU_LPC40XX 3
#define OPT_MCU_SAMG 4
#define OPT_MCU_SAMX7X 5
#define OPT_MCU_FT90X 6
#define OPT_MCU_FT93X 7

#define TUD_OPT_HIGH_SPEED 0
#define TUSB_DESC_DEVICE 1
#define TUSB_DESC_STRING 3

typedef struct __attribute__((packed)) {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdUSB;
    uint8_t bDeviceClass;
    uint8_t bDeviceSubClass;
    uint8_t bDeviceProtocol;
    uint8_t bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t iManufacturer;
    uint8_t iProduct;
    uint8_t iSerialNumber;
    uint8_t bNumConfigurations;
} tusb_desc_device_t;

#define TUD_CONFIG_DESC_LEN 1
#define TUD_MIDI_DESC_HEAD_LEN 1
#define TUD_MIDI_DESC_JACK_LEN 1
#define TUD_MIDI_DESC_EP_LEN(cables) 1
#define TUD_CDC_DESC_LEN 1

#define TUD_CONFIG_DESCRIPTOR(...) 0
#define TUD_CDC_DESCRIPTOR(...) 0
#define TUD_MIDI_DESC_HEAD(...) 0
#define TUD_MIDI_DESC_JACK(...) 0
#define TUD_MIDI_DESC_EP(...) 0
#define TUD_MIDI_JACKID_IN_EMB(...) 0
#define TUD_MIDI_JACKID_OUT_EMB(...) 0

bool tud_midi_packet_read(uint8_t packet[4]);
uint32_t tud_midi_available(void);
uint32_t tud_midi_stream_write(uint8_t cable, const uint8_t *data,
                               uint32_t length);

#endif
