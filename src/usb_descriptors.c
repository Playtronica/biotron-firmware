/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <pico/bootrom.h>
#include "tusb.h"

/* A combination of interfaces must have a unique product id, since PC will save device driver after the first plug.
 * Same VID/PID with different interface e.g MSC (first), then CDC (later) will possibly cause system error on PC.
 *
 * Auto ProductID layout's Bitmap:
 *   [MSB]       MIDI | HID | MSC | CDC          [LSB]
 */
#define _PID_MAP(itf, n)  ( (CFG_TUD_##itf) << (n) )
#define USB_PID           (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) | \
                           _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4) )

#define USB_VID   0xCafe
#define USB_BCD   0x0200

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device =
        {
                .bLength            = sizeof(tusb_desc_device_t),
                .bDescriptorType    = TUSB_DESC_DEVICE,
                .bcdUSB             = USB_BCD,

                // Use Interface Association Descriptor (IAD) for CDC
                // As required by USB Specs IAD's subclass must be common class (2) and protocol must be IAD (1)
                .bDeviceClass       = 0x00,
                .bDeviceSubClass    = 0x00,
                .bDeviceProtocol    = 0x00,
                .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

                .idVendor           = USB_VID,
                .idProduct          = USB_PID,
                .bcdDevice          = 0x0100,

                .iManufacturer      = 0x01,
                .iProduct           = 0x02,
                .iSerialNumber      = 0x03,

                .bNumConfigurations = 0x01
        };


// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const * tud_descriptor_device_cb(void)
{
    return (uint8_t const *) &desc_device;
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+
enum
{

    ITF_NUM_MIDI = 0,
    ITF_NUM_MIDI_STREAMING,
    ITF_NUM_CDC,
    ITF_NUM_CDC_DATA,
    ITF_NUM_TOTAL
};

#define CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + \
(TUD_MIDI_DESC_HEAD_LEN + TUD_MIDI_DESC_JACK_LEN * MIDI_NUM_CABLES + TUD_MIDI_DESC_EP_LEN(MIDI_NUM_CABLES) * 2) + \
(TUD_CDC_DESC_LEN))
#if CFG_TUSB_MCU == OPT_MCU_LPC175X_6X || CFG_TUSB_MCU == OPT_MCU_LPC177X_8X || CFG_TUSB_MCU == OPT_MCU_LPC40XX
// LPC 17xx and 40xx endpoint type (bulk/interrupt/iso) are fixed by its number
  // 0 control, 1 In, 2 Bulk, 3 Iso, 4 In etc ...
  #define EPNUM_MIDI   0x02
#else
#define EPNUM_MIDI   0x01
#endif
#define MIDI_NUM_CABLES 2

#if CFG_TUSB_MCU == OPT_MCU_LPC175X_6X || CFG_TUSB_MCU == OPT_MCU_LPC177X_8X || CFG_TUSB_MCU == OPT_MCU_LPC40XX
// LPC 17xx and 40xx endpoint type (bulk/interrupt/iso) are fixed by its number
  // 0 control, 1 In, 2 Bulk, 3 Iso, 4 In etc ...
  #define EPNUM_CDC_0_NOTIF   0x81
  #define EPNUM_CDC_0_OUT     0x02
  #define EPNUM_CDC_0_IN      0x82

  #define EPNUM_CDC_1_NOTIF   0x84
  #define EPNUM_CDC_1_OUT     0x05
  #define EPNUM_CDC_1_IN      0x85

#elif CFG_TUSB_MCU == OPT_MCU_SAMG || CFG_TUSB_MCU ==  OPT_MCU_SAMX7X
// SAMG & SAME70 don't support a same endpoint number with different direction IN and OUT
  //    e.g EP1 OUT & EP1 IN cannot exist together
  #define EPNUM_CDC_0_NOTIF   0x81
  #define EPNUM_CDC_0_OUT     0x02
  #define EPNUM_CDC_0_IN      0x83

  #define EPNUM_CDC_1_NOTIF   0x84
  #define EPNUM_CDC_1_OUT     0x05
  #define EPNUM_CDC_1_IN      0x86

#else
#define EPNUM_CDC_0_NOTIF   0x83
#define EPNUM_CDC_0_OUT     0x02
#define EPNUM_CDC_0_IN      0x82

#endif

#if CFG_TUSB_MCU == OPT_MCU_LPC175X_6X || CFG_TUSB_MCU == OPT_MCU_LPC177X_8X || CFG_TUSB_MCU == OPT_MCU_LPC40XX
// LPC 17xx and 40xx endpoint type (bulk/interrupt/iso) are fixed by its number
  // 0 control, 1 In, 2 Bulk, 3 Iso, 4 In etc ...
  #define EPNUM_MIDI_OUT   0x02
  #define EPNUM_MIDI_IN   0x02
#elif CFG_TUSB_MCU == OPT_MCU_FT90X || CFG_TUSB_MCU == OPT_MCU_FT93X
// On Bridgetek FT9xx endpoint numbers must be unique...
  #define EPNUM_MIDI_OUT   0x02
  #define EPNUM_MIDI_IN   0x03
#else
#define EPNUM_MIDI_OUT   0x01
#define EPNUM_MIDI_IN   0x01
#endif

uint8_t const desc_fs_configuration[] =
        {
                // Config number, interface count, string channel_id, total length, attribute, power in mA
                TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),


                TUD_MIDI_DESC_HEAD(ITF_NUM_MIDI, 0, MIDI_NUM_CABLES),
                TUD_MIDI_DESC_JACK(1),
                TUD_MIDI_DESC_JACK(2),
                TUD_MIDI_DESC_EP(EPNUM_MIDI, 64, MIDI_NUM_CABLES),
                TUD_MIDI_JACKID_IN_EMB(1),
                TUD_MIDI_JACKID_IN_EMB(2),
                TUD_MIDI_DESC_EP(0x80 | EPNUM_MIDI, 64, MIDI_NUM_CABLES),
                TUD_MIDI_JACKID_OUT_EMB(1),
                TUD_MIDI_JACKID_OUT_EMB(2),
                TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 0, EPNUM_CDC_0_NOTIF, 8, EPNUM_CDC_0_OUT, EPNUM_CDC_0_IN, 64),
        };

#if TUD_OPT_HIGH_SPEED
// Per USB specs: high speed capable device must report device_qualifier and other_speed_configuration

uint8_t const desc_hs_configuration[] =
{
  // Config number, interface count, string channel_id, total length, attribute, power in mA
  TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),

  // 1st CDC: Interface number, string channel_id, EP notification address and size, EP data address (out, in) and size.
  TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 4, EPNUM_CDC_0_NOTIF, 8, EPNUM_CDC_0_OUT, EPNUM_CDC_0_IN, 512),
  TUD_MIDI_DESCRIPTOR(ITF_NUM_MIDI, 4, EPNUM_MIDI, 0x80 | EPNUM_MIDI, 512),
};

// device qualifier is mostly similar to device descriptor since we don't change configuration based on speed
tusb_desc_device_qualifier_t const desc_device_qualifier =
{
  .bLength            = sizeof(tusb_desc_device_t),
  .bDescriptorType    = TUSB_DESC_DEVICE,
  .bcdUSB             = USB_BCD,

  .bDeviceClass       = TUSB_CLASS_MISC,
  .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
  .bDeviceProtocol    = MISC_PROTOCOL_IAD,

  .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
  .bNumConfigurations = 0x01,
  .bReserved          = 0x00
};

// Invoked when received GET DEVICE QUALIFIER DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete.
// device_qualifier descriptor describes information about a high-speed capable device that would
// change if the device were operating at the other speed. If not highspeed capable stall this request.
uint8_t const* tud_descriptor_device_qualifier_cb(void)
{
  return (uint8_t const*) &desc_device_qualifier;
}

// Invoked when received GET OTHER SEED CONFIGURATION DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
// Configuration descriptor in the other speed e.g if high speed then this is for full speed and vice versa
uint8_t const* tud_descriptor_other_speed_configuration_cb(uint8_t channel_id)
{
  (void) channel_id; // for multiple configurations

  // if link speed is high return fullspeed config, and vice versa
  return (tud_speed_get() == TUSB_SPEED_HIGH) ?  desc_fs_configuration : desc_hs_configuration;
}

#endif // highspeed

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const * tud_descriptor_configuration_cb(uint8_t channel_id)
{
    (void) channel_id; // for multiple configurations

#if TUD_OPT_HIGH_SPEED
    // Although we are highspeed, host may be fullspeed.
  return (tud_speed_get() == TUSB_SPEED_HIGH) ?  desc_hs_configuration : desc_fs_configuration;
#else
    return desc_fs_configuration;
#endif
}


//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)
// array of pointer to string descriptors
char const* string_desc_arr [] =
        {
                (const char[]) { 0x09, 0x04 },  // 0: is supported language is English (0x0409)
                "Playtronica",                          // 1: Manufacturer
#ifdef DEVICE_NAME
                STRINGIZE(DEVICE_NAME), // 2: Product
#else
                "PL_SDK_DEVICE_UNKNOWN", // 2: Product
#endif
                "Bio5",                                 // 3: Serials, should use chip ID
                "TinyUSB CDC"                           // 4: CDC Interface
        };

static uint16_t _desc_str[32];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const* tud_descriptor_string_cb(uint8_t channel_id, uint16_t langid)
{
    (void) langid;

    uint8_t chr_count;

    if ( channel_id == 0)
    {
        memcpy(&_desc_str[1], string_desc_arr[0], 2);
        chr_count = 1;
    }
    else
    {
        // Note: the 0xEE channel_id string is a Microsoft OS 1.0 Descriptors.
        // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

        if ( !(channel_id < sizeof(string_desc_arr)/sizeof(string_desc_arr[0])) ) return NULL;

        const char* str = string_desc_arr[channel_id];

        // Cap at max char
        chr_count = (uint8_t) strlen(str);
        if ( chr_count > 31 ) chr_count = 31;

        // Convert ASCII string into UTF-16
        for(uint8_t i=0; i<chr_count; i++)
        {
            _desc_str[1+i] = str[i];
        }
    }

    // first byte is length (including header), second byte is string type
    _desc_str[0] = (uint16_t) ((TUSB_DESC_STRING << 8 ) | (2*chr_count + 2));

    return _desc_str;
}