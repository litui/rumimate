#pragma once

#include <Arduino.h>
#include <TeensyThreads.h>
#include <Encoder.h>

namespace LMEncoder {

enum class Button {
  ENC_BUTTON = 0,
  BUTTON_A,
  BUTTON_B,
  BUTTON_C
};

void begin();
void tick();

long read_encoder_pos();
long read_bounded_encoder_pos(long lower, long upper);
void reset_encoder_pos(long value);
bool read_reset_button_state(Button button);

} // namespace LMEncoder