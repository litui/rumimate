#include "encoder.hpp"
#include <Bounce2.h>

namespace LMEncoder {
Encoder knob(6, 5);
Bounce bouncer[4] = {Bounce(), Bounce(), Bounce(), Bounce()};
long position = 0;
bool triggered[4] = {false, false, false, false};

void begin() {
  knob.write(position);

  // Encoder button
  bouncer[0].attach(7, INPUT_PULLUP);
  bouncer[0].interval(60);

  // Button A
  bouncer[1].attach(8, INPUT_PULLUP);
  bouncer[1].interval(60);

  // Button B
  bouncer[2].attach(9, INPUT_PULLUP);
  bouncer[2].interval(60);

  // Button C
  bouncer[3].attach(10, INPUT_PULLUP);
  bouncer[3].interval(60);
}

void tick() {
  long current_pos = knob.read();
  if (position != current_pos / 4 && current_pos % 4 == 0) {
    position = current_pos / 4;
  }

  for (uint8_t i = 0; i < 4; i++) {
    bouncer[i].update();
    if (bouncer[i].fell()) {
      triggered[i] = true;
    }
  }
  threads.yield();
}

long read_encoder_pos() {
  return position;
}

long read_bounded_encoder_pos(long lower, long upper) {
  if (position < lower) {
    reset_encoder_pos(lower);
  } else if (position > upper) {
    reset_encoder_pos(upper);
  }
  return position;
}

bool read_reset_button_state(Button button) {
  if (triggered[(uint8_t)button]) {
    triggered[(uint8_t)button] = false;
    return true;
  }

  return false;
}

void reset_encoder_pos(long value) {
  position = value;
  knob.write(value * 4);
}

} // namespace LMEncoder