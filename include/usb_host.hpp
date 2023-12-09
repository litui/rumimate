#pragma once

#include <Arduino.h>
#include <EEPROM.h>
#include <TeensyThreads.h>
#include <USBHost_t36.h>

#define RESERVE_CHANNEL_1 1
#define RESERVE_CHANNEL_16 1

#define MIDI_HOST_DEVICE_POOL 2

namespace LMUSBHost {

PROGMEM static const int lumi_vendor_id[] = {0x2af4};
PROGMEM static const int lumi_product_id[] = {0x0e00};
#define LUMI_VENDOR_COUNT (sizeof(lumi_vendor_id) / sizeof(lumi_vendor_id[0]))
#define LUMI_PRODUCT_COUNT                                                     \
  (sizeof(lumi_product_id) / sizeof(lumi_product_id[0]))

PROGMEM static const int ewi_vendor_id[] = {0x09e8};
PROGMEM static const int ewi_product_id[] = {0x006d};
#define EWI_VENDOR_COUNT (sizeof(ewi_vendor_id) / sizeof(ewi_vendor_id[0]))
#define EWI_PRODUCT_COUNT                                                     \
  (sizeof(ewi_product_id) / sizeof(ewi_product_id[0]))

void begin();
void tick();

bool is_lumi(MIDIDevice midi_dev);
bool is_ewi(MIDIDevice midi_dev);

MIDIDevice *get_lumi_dev();
MIDIDevice *get_ewi_dev();

extern MIDIDevice usb_midi_dev[MIDI_HOST_DEVICE_POOL];

} // namespace LMUSBHost