#include "led.hpp"

namespace LED {

int LEDThread;

void led_thread() {
  while(1) {
    digitalWriteFast(LED_BUILTIN, LOW);
    threads.delay(50);
    digitalWriteFast(LED_BUILTIN, HIGH);
    threads.delay(50);
    threads.suspend(LEDThread);
  }
}

void blink() {
  threads.restart(LEDThread);
}

void begin() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  LEDThread = threads.addThread(led_thread);
  threads.start(LEDThread);
}

void tick() {
  threads.yield();
}

} // namespace LED