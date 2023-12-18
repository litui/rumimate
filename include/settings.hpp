#pragma once

#include <EEPROM.h>
#include "lumi_sysex.hpp"
#include "quantizer.hpp"

namespace Settings {

struct Params {
  LumiSysex::Key key = LumiSysex::Key::C;
  LumiSysex::Scale scale = LumiSysex::Scale::CHROMATIC;
  Quantizer::Quantizer::Mode quantizer_mode = Quantizer::Quantizer::Mode::ROUND_UP;
  LumiSysex::MPEMode mpe_mode = LumiSysex::MPEMode::LOWER;
  uint8_t root_r = 255;
  uint8_t root_g = 0;
  uint8_t root_b = 255;
  uint8_t global_r = 0;
  uint8_t global_g = 0;
  uint8_t global_b = 255;
};

Params load_settings();
void save_settings(Params settings);
Params get_settings();

void begin();
void tick();

extern Params last_values;

} // namespace Settings