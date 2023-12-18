#include "settings.hpp"

namespace Settings {

uint32_t last_millis = 0;
bool data_changed = true;

Params real_settings;
Params last_values;

Params load_settings() {
  Serial.println("Loading settings...");
  EEPROM.get<Params>(0, real_settings);
  data_changed = false;
  return real_settings;
}

void save_settings(Params settings) {
  real_settings = settings;
  data_changed = true;
}

Params get_settings() {
  return real_settings;
}

void real_save_settings() {
  EEPROM.put<Params>(0, real_settings);
}

void begin() {
  EEPROM.begin();
  last_values = load_settings();
}

void tick() {
  uint32_t current_millis = millis();

  if (current_millis > 60000 + last_millis && data_changed) {
    last_millis = current_millis;

    Serial.println("Settings changed and time elapsed. Saving settings.");
    real_save_settings();
    data_changed = false;
  }
}

} // namespace Settings