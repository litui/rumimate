#pragma once

#include <Arduino.h>
#include <TeensyThreads.h>

#define LUMI_DEVICE_CABLE 0
#define EWI_DEVICE_CABLE 1

namespace LMUSBDevice {

void begin();
void tick();

} // namespace LMUSBDevice