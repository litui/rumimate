#pragma once

#include <Arduino.h>
#include <TeensyThreads.h>

namespace LED {

void begin();
void tick();

void blink();

} // namespace LED