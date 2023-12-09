#include "lumi_sysex.hpp"

namespace LumiSysex {

uint8_t last_scale = 18;
uint8_t last_key = 0;

uint8_t get_current_key() {
  return last_key;
}

uint8_t get_current_scale() {
  return last_scale;
}

void print_bytes(const byte *data, unsigned int size) {
  while (size > 0) {
    byte b = *data++;
    if (b < 16)
      Serial.print('0');
    Serial.print(b, HEX);
    if (size > 1)
      Serial.print(' ');
    size = size - 1;
  }
}

uint8_t sysex_checksum(uint8_t *chk_bytes, uint8_t size) {
  uint8_t c = size;
  for (uint8_t i = 0; i < size; i++) {
    c = (c * 3 + chk_bytes[i]) & 0xff;
  }
  return c & 0x7f;
}

void sysex_send(MIDIDevice &midi_dev, uint8_t *command, uint8_t length,
                uint8_t device, uint8_t spacer) {
  uint8_t sysex_msg[17] = {0};
  // memchr(sysex_msg, 0x00, 16);
  sysex_msg[0] = SYSEX_START;
  memcpy(&sysex_msg[1], lumi_manufacturer, 3);
  sysex_msg[4] = LUMI_SYSEX_SPACER;
  sysex_msg[5] = 0x00;
  memcpy(&sysex_msg[6], command, length);
  sysex_msg[14] = sysex_checksum(&sysex_msg[6], 8);
  sysex_msg[15] = SYSEX_END;

  midi_dev.sendSysEx(16, sysex_msg, true);
}

void set_key(MIDIDevice &midi_dev, uint8_t key_index) {
  uint8_t command[4];
  command[0] = LUMI_CONFIG_PREFIX;
  command[1] = LUMI_CONFIG_KEY_PREFIX;
  memcpy(&command[2], key_commands[key_index], 2);

  sysex_send(midi_dev, command, 4);
  last_key = key_index;
}

void set_scale(MIDIDevice &midi_dev, uint8_t scale_index) {
  uint8_t command[4];
  command[0] = LUMI_CONFIG_PREFIX;
  command[1] = LUMI_CONFIG_SCALE_PREFIX;
  memcpy(&command[2], scale_commands[scale_index], 2);

  sysex_send(midi_dev, command, 4);
  last_scale = scale_index;
}

void set_global_color(MIDIDevice &midi_dev, uint8_t r, uint8_t g, uint8_t b) {
  uint64_t color_bits = 
    0b00100 | 
    b << 6 |
    g << 15 |
    r << 24 |
    (uint64_t)0b11111100 << 32;

  uint8_t color_command[8] = {
    LUMI_CONFIG_PREFIX,
    LUMI_CONFIG_GLOBAL_COLOR_PREFIX,
    (uint8_t)color_bits,
    (uint8_t)(color_bits >> 8),
    (uint8_t)(color_bits >> 16),
    (uint8_t)(color_bits >> 24),
    (uint8_t)(color_bits >> 32)
  };

  sysex_send(midi_dev, color_command, 7);
}

void set_root_color(MIDIDevice &midi_dev, uint8_t r, uint8_t g, uint8_t b) {
  uint64_t color_bits = 
    0b00100 | 
    b << 6 |
    g << 15 |
    r << 24 |
    (uint64_t)0b11111100 << 32;

  uint8_t color_command[8] = {
    LUMI_CONFIG_PREFIX,
    LUMI_CONFIG_ROOT_COLOR_PREFIX,
    (uint8_t)color_bits,
    (uint8_t)(color_bits >> 8),
    (uint8_t)(color_bits >> 16),
    (uint8_t)(color_bits >> 24),
    (uint8_t)(color_bits >> 32)
  };

  sysex_send(midi_dev, color_command, 7);
}

void set_max_brightness(MIDIDevice &midi_dev) {
  uint8_t command[8] = {
    LUMI_CONFIG_PREFIX,
    0x40,
    0x04,
    0x19
  };

  sysex_send(midi_dev, command, 4);
}

void send_enumerate(MIDIDevice &midi_dev) {
  // F0 00 21 10 77 00 01 01 00 5D
  uint8_t command[4] = {0x01, 0x01, 0x00, 0x5D};
  sysex_send(midi_dev, command, 4);
}

void send_read_configuration(MIDIDevice &midi_dev) {
  uint8_t command[4] = {0x01, 0x02};
  sysex_send(midi_dev, command, 4);
}

void lumi_sysex_chunk_handler(const byte *data, uint16_t length, bool last) {
  Serial.print("SysEx Message: ");
  print_bytes(data, length);
  if (last) {
    Serial.println(" (end)");
  } else {
    Serial.println(" (to be continued)");
  }
}

} // namespace LumiSysex