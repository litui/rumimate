#include "display.hpp"
#include "encoder.hpp"
#include "lumi_sysex.hpp"
#include "quantizer.hpp"
#include "usb_host.hpp"

namespace Display {

Driver drv;
uint32_t last_update = 0;

void Driver::begin() {
  LMEncoder::begin();

  sh1107.begin(0x3C, true);

  sh1107.setRotation(1);
  sh1107.display();
}

void Driver::tick() {
  uint32_t current_millis = millis();

  if (current_millis > last_update + 100) {
    last_update = current_millis;

    sh1107.clearDisplay();

    sh1107.setCursor(0, 0);
    sh1107.setTextColor(SH110X_WHITE);
    sh1107.setTextWrap(false);
    sh1107.println("LumiMate");
    sh1107.println();

    long lower = 0;
    long upper = 127;
    if (current_screen == Screen::KEY_SCALE) {
      upper = (uint8_t)KSOption::__COUNT - 1;
    }

    int8_t menu_select =
        current_entered ? last_menu_select
                        : LMEncoder::read_bounded_encoder_pos(lower, upper);

    bool enc_button_state =
        LMEncoder::read_reset_button_state(LMEncoder::Button::ENC_BUTTON);

    if (current_screen == Screen::KEY_SCALE) {
      for (uint8_t i = 0; i < (uint8_t)KSOption::__COUNT; i++) {
        char *name;
        long item_index = 0;
        long item_count = 0;
        bool selected = KSOption(menu_select) == KSOption(i);
        char sel_char = selected ? '>' : ' ';

        if (KSOption(i) == KSOption::KEY) {
          item_index = LumiSysex::get_current_key();
          item_count = LUMI_KEY_COUNT;

          if (selected && !current_entered) {
            if (enc_button_state) {
              current_entered = enc_button_state;
              LMEncoder::reset_encoder_pos(item_index);
            }
          } else if (selected && current_entered) {
            item_index = LMEncoder::read_bounded_encoder_pos(0, item_count - 1);
            MIDIDevice *lumi = LMUSBHost::get_lumi_dev();
            if (lumi != NULL) {
              LumiSysex::set_key(*lumi, item_index);
            }

            if (enc_button_state) {
              current_entered = false;
              LMEncoder::reset_encoder_pos(menu_select);
            }
          }

          name = (char *)LumiSysex::key_name[item_index];
        } else if (KSOption(i) == KSOption::SCALE) {
          item_index = LumiSysex::get_current_scale();
          item_count = LUMI_SCALE_COUNT;

          if (selected && !current_entered) {
            if (enc_button_state) {
              current_entered = enc_button_state;
              LMEncoder::reset_encoder_pos(item_index);
            }
          } else if (selected && current_entered) {
            item_index = LMEncoder::read_bounded_encoder_pos(0, item_count - 1);
            MIDIDevice *lumi = LMUSBHost::get_lumi_dev();
            if (lumi != NULL) {
              LumiSysex::set_scale(*lumi, item_index);
            }
            

            if (enc_button_state) {
              current_entered = false;
              LMEncoder::reset_encoder_pos(menu_select);
            }
          }

          name = (char *)LumiSysex::scale_name[item_index];
        } else if (KSOption(i) == KSOption::QUANTIZER_MODE) {
          item_index = (uint8_t)Quantizer::quantizer.get_mode();
          item_count = (uint8_t)Quantizer::Quantizer::Mode::__COUNT;

          if (selected && !current_entered) {
            if (enc_button_state) {
              current_entered = enc_button_state;
              LMEncoder::reset_encoder_pos(item_index);
            }
          } else if (selected && current_entered) {
            item_index = LMEncoder::read_bounded_encoder_pos(0, item_count - 1);
            Quantizer::quantizer.set_mode(
                Quantizer::Quantizer::Mode(item_index));

            if (enc_button_state) {
              current_entered = false;
              LMEncoder::reset_encoder_pos(menu_select);
            }
          }

          name = (char *)Quantizer::quantizer.mode_name[item_index];
        }

        sh1107.printf("%c %s: %s\n", sel_char, ks_option_names[i], name);

        threads.yield();
      }
    }

    last_menu_select = menu_select;

    sh1107.display();
  }
  threads.yield();
}

} // namespace Display