#pragma once
#include "USBHost_t36.h"
#include <Arduino.h>

#define SYSEX_START 0xF0
#define SYSEX_END 0xF7
#define LUMI_SYSEX_SPACER 0x77
#define LUMI_DEVICE 0x00 // All devices

#define LUMI_CONFIG_PREFIX 0x10
#define LUMI_KEY_COUNT 12
#define LUMI_CONFIG_KEY_PREFIX 0x30
#define LUMI_SCALE_COUNT 19
#define LUMI_CONFIG_SCALE_PREFIX 0x60
#define LUMI_CONFIG_MPE_NUM_CHANNEL_PREFIX 0x10
#define LUMI_CONFIG_MIDI_MODE_PREFIX 0x20
#define LUMI_CONFIG_UPPER_LOWER_PREFIX 0x00

#define LUMI_CONFIG_ROOT_COLOR_PREFIX 0x30
#define LUMI_CONFIG_GLOBAL_COLOR_PREFIX 0x20

#define LUMI_MPE_MODES 3
#define MPE_RPN_MSB 0
#define MPE_RPN_LSB 6

namespace LumiSysex {

const uint8_t lumi_manufacturer[3] = {0x00, 0x21, 0x10};

enum class Key {
  C = 0,
  C_SHARP,
  D,
  D_SHARP,
  E,
  F,
  F_SHARP,
  G,
  G_SHARP,
  A,
  A_SHARP,
  B,
};

const char key_name[LUMI_KEY_COUNT][3] = {"C",  "C#", "D",  "D#", "E",  "F",
                                          "F#", "G",  "G#", "A",  "A#", "B"};

const uint8_t key_commands[LUMI_KEY_COUNT][2] = {
    {0x03, 0x00}, // C
    {0x23, 0x00}, // C#
    {0x43, 0x00}, // D
    {0x63, 0x00}, // D#
    {0x03, 0x01}, // E
    {0x23, 0x01}, // F
    {0x43, 0x01}, // F#
    {0x63, 0x01}, // G
    {0x03, 0x02}, // G#
    {0x23, 0x02}, // A
    {0x43, 0x02}, // A#
    {0x63, 0x02}  // B
};

enum class Scale {
  MAJOR = 0,
  MINOR,
  HARMONIC_MINOR,
  PENTATONIC_NEUTRAL,
  PENTATONIC_MAJOR,
  PENTATONIC_MINOR,
  BLUES,
  DORIAN,
  PHRYGIAN,
  LYDIAN,
  MIXOLYDIAN,
  LOCRIAN,
  WHOLE_TONE,
  ARABIC_A,
  ARABIC_B,
  JAPANESE,
  RYUKYU,
  EIGHT_TONE_SPANISH,
  CHROMATIC
};

const char scale_name[LUMI_SCALE_COUNT][12] = {
    "Major",       "Minor",   "Harm. Minor", "Pent. Neut.", "Pent. Major",
    "Pent. Minor", "Blues",   "Dorian",      "Phrygian",    "Lydian",
    "Mixolydian",  "Locrian", "Whole Tone",  "Arabic (a)",  "Arabic (b)",
    "Japanese",    "Ryukyu",  "8-tone Sp.",  "Chromatic"};

const uint8_t scale_commands[LUMI_SCALE_COUNT][2] = {
    {0x02, 0x00}, {0x22, 0x00}, {0x42, 0x00}, {0x62, 0x00}, {0x02, 0x01},
    {0x22, 0x01}, {0x42, 0x01}, {0x62, 0x01}, {0x02, 0x02}, {0x22, 0x02},
    {0x42, 0x02}, {0x62, 0x02}, {0x02, 0x03}, {0x22, 0x03}, {0x42, 0x03},
    {0x62, 0x03}, {0x02, 0x04}, {0x22, 0x04}, {0x42, 0x04}};

enum class MIDIMode { MPE, MULTI, SINGLE };

const uint8_t midi_mode_commands[] = {
  0x20, // MPE
  0x00, // Multi
  0x40  // Single
};

enum class MPEMode { OFF, LOWER, UPPER };

const char mpe_mode_name[LUMI_MPE_MODES][8] = {"Off", "Lower", "Upper"};

const uint8_t mpe_mode_commands[LUMI_MPE_MODES] = {
  NULL,
  0x05,
  0x25
};

const uint8_t mpe_num_channel_commands[15][2] = {
  {21, 00}, // 1
  {41, 00},
  {61, 00},
  {01, 01},
  {21, 01},
  {41, 01},
  {61, 01},
  {01, 02},
  {21, 02},
  {41, 02},
  {61, 02},
  {01, 03},
  {21, 03},
  {41, 03},
  {61, 03}  // 15
};

uint8_t get_current_key();
uint8_t get_current_scale();
MPEMode get_current_mpe_mode();
void set_key(MIDIDevice &midi_dev, uint8_t key_index);
void set_scale(MIDIDevice &midi_dev, uint8_t scale_index);
void set_root_color(MIDIDevice &midi_dev, uint8_t r, uint8_t g, uint8_t b);
void set_global_color(MIDIDevice &midi_dev, uint8_t r, uint8_t g, uint8_t b);
void set_mpe_mode(MIDIDevice &midi_dev, MPEMode mode);

void send_enumerate(MIDIDevice &midi_dev);
void send_read_configuration(MIDIDevice &midi_dev);
void set_max_brightness(MIDIDevice &midi_dev);

void sysex_send(MIDIDevice &midi_dev, uint8_t *command, uint8_t length,
                uint8_t device = LUMI_DEVICE,
                uint8_t spacer = LUMI_SYSEX_SPACER);

void lumi_sysex_chunk_handler(const byte *data, uint16_t length, bool last);

} // namespace LumiSysex