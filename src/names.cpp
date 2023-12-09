#include <avr/pgmspace.h>
#include <usb_names.h>

#define PRODUCT_NAME {'R','u','m','i','M','a','t','e'}
#define PRODUCT_NAME_LEN 8
#define MANUFACTURER_NAME {'L','i','t','u','i'}
#define MANUFACTURER_NAME_LEN 5
#define MIDI_NAME {'R','u','m','i','M','a','t','e'}
#define MIDI_NAME_LEN 8

PROGMEM struct usb_string_descriptor_struct usb_string_manufacturer_name = {
    2 + MANUFACTURER_NAME_LEN * 2,
    3,
    MANUFACTURER_NAME
};

PROGMEM struct usb_string_descriptor_struct usb_string_product_name = {
    2 + PRODUCT_NAME_LEN * 2,
    3,
    PRODUCT_NAME
};