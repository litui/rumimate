#include "quantizer.hpp"
#include "settings.hpp"

namespace Quantizer {

Quantizer quantizer;

byte Quantizer::quantize_note(byte midi_note) {
  if (Settings::last_values.quantizer_mode == Mode::NONE) {
    Serial.println("Quantizer note passed through (quantizer off).");
    return midi_note;
  }

  uint8_t root = LumiSysex::get_current_key();
  uint8_t scale = LumiSysex::get_current_scale();

  // uint8_t octave = midi_note / 12;
  uint8_t oct_note = midi_note % 12;

  // Serial.print("Quantizer Note Octave: ");
  // Serial.println(octave);
  // Serial.print("Quantizer Note in Octave: ");
  // Serial.println(oct_note);

  uint8_t shifted_note =
      oct_note >= root ? oct_note - root : oct_note + 12 - root;
  // int8_t shifted_octave = oct_note >= root ? octave : octave - 1;

  // Serial.print("Quantizer Shifted Octave: ");
  // Serial.println(shifted_octave);
  // Serial.print("Quantizer Shifted Note: ");
  // Serial.println(shifted_note);

  // Serial.print("Quantizer bitmask: ");
  // Serial.println(quantized_scales[scale], BIN);

  // Determine if note is played or not.
  if (bitRead(quantized_scales[scale], shifted_note)) {
    // If bitRead returns 1, return original note.
    Serial.println("Quantizer note passed bitmask.");
    return midi_note;

  } else {
    if (Settings::last_values.quantizer_mode == Mode::ROUND_UP) {
      uint8_t filter_offset = 1;

      while (!bitRead(quantized_scales[scale],
                      ((filter_offset + shifted_note) % 12))) {
        filter_offset += 1;
      }

      if (midi_note + filter_offset < 128) {
        Serial.print("Quantizer note rounded up: ");
        Serial.println(midi_note + filter_offset, DEC);
        return midi_note + filter_offset;
      }
    } else if (Settings::last_values.quantizer_mode == Mode::ROUND_DOWN) {
      uint8_t filter_offset = 1;
      uint8_t reshifted_note = shifted_note - filter_offset < 0 ? 12 + shifted_note - filter_offset : shifted_note - filter_offset;

      // Serial.print("Quantizer reshifted note: ");
      // Serial.println(reshifted_note, DEC);

      while (!bitRead(quantized_scales[scale], reshifted_note)) {
        filter_offset += 1;
        reshifted_note = shifted_note - filter_offset < 0 ? 12 + shifted_note - filter_offset : shifted_note - filter_offset;

        // Serial.print("Quantizer reshifted note: ");
        // Serial.println(reshifted_note, DEC);
      }

      if (midi_note - filter_offset >= 0) {
        Serial.print("Quantizer note rounded down: ");
        Serial.println(midi_note - filter_offset, DEC);
        return midi_note - filter_offset;
      }
    }
  }

  return -1;
}

Quantizer::Mode Quantizer::get_mode() {
  return Settings::last_values.quantizer_mode;
}

void Quantizer::set_mode(Mode mode) {
  if (Settings::last_values.quantizer_mode != mode) {
    Settings::last_values.quantizer_mode = mode;
    Settings::save_settings(Settings::last_values);
  }
}

} // namespace Quantizer