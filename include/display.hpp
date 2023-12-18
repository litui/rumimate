#pragma once

#include <Arduino.h>
#include <TeensyThreads.h>
#include <Wire.h>
#include <Adafruit_SH110X.h>

#define DISPLAY_LINE_HEIGHT 16

namespace Display {

class Driver {
public:
  Driver() {};

  void begin();
  void tick();

  enum class Screen {
    KEY_SCALE = 0,
    __COUNT
  };

  enum class KSOption {
    KEY = 0,
    SCALE,
    QUANTIZER_MODE,
    MPE_MODE,
    __COUNT
  };

  const char ks_option_names[(uint8_t)KSOption::__COUNT][10] = {
    "Key",
    "Scale",
    "Quantizer",
    "MPE"
  };

private:
  Adafruit_SH1107 sh1107 = Adafruit_SH1107(64, 128, &Wire);

  uint8_t last_menu_select = 0;

  Screen current_screen = Screen::KEY_SCALE;
  KSOption ks_current_option = KSOption::KEY;
  bool current_entered = false;
};

extern Driver drv;

} // namespace Display