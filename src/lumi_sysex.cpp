#include "lumi_sysex.hpp"
#include "settings.hpp"

namespace LumiSysex {

uint8_t get_current_key() { return (uint8_t)Settings::last_values.key; }

uint8_t get_current_scale() { return (uint8_t)Settings::last_values.scale; }

MPEMode get_current_mpe_mode() { return Settings::last_values.mpe_mode; }

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
  if (Settings::last_values.key != LumiSysex::Key(key_index)) {
    Settings::last_values.key = LumiSysex::Key(key_index);
    Settings::save_settings(Settings::last_values);
  }
}

void set_scale(MIDIDevice &midi_dev, uint8_t scale_index) {
  uint8_t command[4];
  command[0] = LUMI_CONFIG_PREFIX;
  command[1] = LUMI_CONFIG_SCALE_PREFIX;
  memcpy(&command[2], scale_commands[scale_index], 2);

  sysex_send(midi_dev, command, 4);

  if (Settings::last_values.scale != LumiSysex::Scale(scale_index)) {
    Settings::last_values.scale = LumiSysex::Scale(scale_index);
    Settings::save_settings(Settings::last_values);
  }
}

void set_global_color(MIDIDevice &midi_dev, uint8_t r, uint8_t g, uint8_t b) {
  uint64_t color_bits =
      0b00100 | b << 6 | g << 15 | r << 24 | (uint64_t)0b11111100 << 32;

  uint8_t color_command[8] = {
      LUMI_CONFIG_PREFIX,          LUMI_CONFIG_GLOBAL_COLOR_PREFIX,
      (uint8_t)color_bits,         (uint8_t)(color_bits >> 8),
      (uint8_t)(color_bits >> 16), (uint8_t)(color_bits >> 24),
      (uint8_t)(color_bits >> 32)};

  sysex_send(midi_dev, color_command, 7);

  Settings::last_values.global_r = r;
  Settings::last_values.global_g = g;
  Settings::last_values.global_b = b;
  Settings::save_settings(Settings::last_values);
}

void set_mpe_mode(MIDIDevice &midi_dev, MPEMode mode) {
  uint8_t upper_msb = 0;
  uint8_t upper_lsb = 0;
  uint8_t lower_msb = 0;
  uint8_t lower_lsb = 0;
  uint8_t upper_control_channel = 15;
  uint8_t lower_control_channel = 0;

  uint8_t midi_mode_command[3] = {0};
  midi_mode_command[0] = LUMI_CONFIG_PREFIX;
  midi_mode_command[1] = LUMI_CONFIG_MIDI_MODE_PREFIX;

  bool send_mpe_params = false;
  uint8_t upper_lower_command[3] = {0};
  upper_lower_command[0] = LUMI_CONFIG_PREFIX;
  upper_lower_command[1] = LUMI_CONFIG_UPPER_LOWER_PREFIX;
  uint8_t num_channel_command[4] = {0};
  num_channel_command[0] = LUMI_CONFIG_PREFIX;
  num_channel_command[1] = LUMI_CONFIG_MPE_NUM_CHANNEL_PREFIX;
  memcpy(&num_channel_command[2], mpe_num_channel_commands[15], 2);

  if (mode == MPEMode::LOWER) {
    lower_msb = 0xf; // Assign 15 channels to the zone
    lower_lsb = 0;   // unused
    upper_msb = 0;
    upper_lsb = 0;
    midi_mode_command[2] = midi_mode_commands[(uint8_t)MIDIMode::MPE];
    send_mpe_params = true;
    upper_lower_command[2] = mpe_mode_commands[(uint8_t)MPEMode::LOWER];
  } else if (mode == MPEMode::UPPER) {
    upper_msb = 0xf; // Assign 15 channels to the zone
    upper_lsb = 0;   // unused
    lower_msb = 0;
    lower_lsb = 0;
    midi_mode_command[2] = midi_mode_commands[(uint8_t)MIDIMode::MPE];
    send_mpe_params = true;
    upper_lower_command[2] = mpe_mode_commands[(uint8_t)MPEMode::UPPER];

  } else {
    midi_mode_command[2] = midi_mode_commands[(uint8_t)MIDIMode::MULTI];
  }

  sysex_send(midi_dev, midi_mode_command, 3);

  if (send_mpe_params) {
    sysex_send(midi_dev, upper_lower_command, 3);
    sysex_send(midi_dev, num_channel_command, 4);

    midi_dev.beginRpn((MPE_RPN_MSB << 7) | MPE_RPN_LSB, lower_control_channel);
    midi_dev.sendRpnValue((lower_msb << 7) | lower_lsb, lower_control_channel);
    midi_dev.endRpn(lower_control_channel);

    midi_dev.beginRpn((MPE_RPN_MSB << 7) | MPE_RPN_LSB, upper_control_channel);
    midi_dev.sendRpnValue((upper_msb << 7) | upper_lsb, upper_control_channel);
    midi_dev.endRpn(upper_control_channel);
  }

  // MPELowZone	F0 00 21 10 77 2E 10 00  05 00 00 00 00 00 77 F7
  // MPEUpZone	F0 00 21 10 77 2E 10 00  25 00 00 00 00 00 57 F7
  // MPENumChn 1	F0 00 21 10 77 2E 10 10  21 00 00 00 00 00 1B F7
  // MPENumChn 2	F0 00 21 10 77 2E 10 10  41 00 00 00 00 00 7B F7
  // MPENumChn15 F0 00 21 10 77 2E 10 10  61 03 00 00 00 00 4E F7

  if (Settings::last_values.mpe_mode != mode) {
    Settings::last_values.mpe_mode = mode;
    Settings::save_settings(Settings::last_values);
  }
}

void set_root_color(MIDIDevice &midi_dev, uint8_t r, uint8_t g, uint8_t b) {
  uint64_t color_bits =
      0b00100 | b << 6 | g << 15 | r << 24 | (uint64_t)0b11111100 << 32;

  uint8_t color_command[8] = {
      LUMI_CONFIG_PREFIX,          LUMI_CONFIG_ROOT_COLOR_PREFIX,
      (uint8_t)color_bits,         (uint8_t)(color_bits >> 8),
      (uint8_t)(color_bits >> 16), (uint8_t)(color_bits >> 24),
      (uint8_t)(color_bits >> 32)};

  sysex_send(midi_dev, color_command, 7);

  Settings::last_values.root_r = r;
  Settings::last_values.root_g = g;
  Settings::last_values.root_b = b;
  Settings::save_settings(Settings::last_values);
}

void set_max_brightness(MIDIDevice &midi_dev) {
  uint8_t command[8] = {LUMI_CONFIG_PREFIX, 0x40, 0x04, 0x19};

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