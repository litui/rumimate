#include "usb_device.hpp"
#include "led.hpp"
#include "lumi_sysex.hpp"
#include "usb_host.hpp"

namespace LMUSBDevice {

const uint8_t color_cc[6] = {20, 36, 56, 19, 35, 55};
uint8_t colors[6] = {0, 0, 255, 255, 0, 255};
bool color_changed = true;

uint32_t last_millis = 0;

void control_change_handler(byte channel, byte control, byte value) {
#if defined(MIDI_DEBUG)
  Serial.print("USBDevice Control Change, ch=");
  Serial.print(channel, DEC);
  Serial.print(", control=");
  Serial.print(control, DEC);
  Serial.print(", value=");
  Serial.println(value, DEC);
#endif
  // LMUSBHost::midi_dev.sendControlChange(control, value, channel);
  for (uint8_t i = 0; i < 6; i++) {
    if (control == color_cc[i]) {
      colors[i] = value * 2;
      color_changed = true;
    }
  }
  LED::blink();
}

void real_time_handler(unsigned char realtimebyte) {
#if defined(MIDI_DEBUG)
  Serial.print("Real Time Message, code=");
  Serial.println(realtimebyte, HEX);
#endif
  // usbMIDI.sendRealTime(realtimebyte);
  LED::blink();
}

void begin() {
  usbMIDI.setHandleControlChange(control_change_handler);
  usbMIDI.begin();
}

void tick() {
  uint32_t current_millis = millis();

  if (current_millis > 50 + last_millis) {
    last_millis = current_millis;

    if (color_changed) {
      MIDIDevice *lumi = LMUSBHost::get_lumi_dev();
      if (lumi != NULL) {
        LumiSysex::set_global_color(*lumi, colors[0], colors[1], colors[2]);
        LumiSysex::set_root_color(*lumi, colors[3], colors[4], colors[5]);
      }
    }
    color_changed = false;
  }
  usbMIDI.send_now();
  usbMIDI.read();
  threads.yield();
}

} // namespace LMUSBDevice