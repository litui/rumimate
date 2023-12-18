#include <Arduino.h>
#include <TeensyThreads.h>
#include "settings.hpp"
#include "display.hpp"
#include "usb_device.hpp"
#include "usb_host.hpp"
#include "encoder.hpp"
#include "led.hpp"

void setup() {
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);

  Serial.begin(115200);
  Settings::begin();
  LED::begin();
  LMUSBDevice::begin();
  LMUSBHost::begin();
  Display::drv.begin();
}

void loop() {
  Settings::tick();
  LED::tick();
  LMUSBDevice::tick();
  LMUSBHost::tick();
  Display::drv.tick();
  LMEncoder::tick();
}

