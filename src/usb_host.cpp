#include "usb_host.hpp"
#include "usb_device.hpp"
#include "led.hpp"
#include "lumi_sysex.hpp"
#include "quantizer.hpp"
#include "settings.hpp"

namespace LMUSBHost {
using namespace LumiSysex;
using namespace Quantizer;

USBHost usb_host;
USBHub hub1(usb_host);
USBHub hub2(usb_host);
USBHub hub3(usb_host);
MIDIDevice usb_midi_dev[MIDI_HOST_DEVICE_POOL] = {MIDIDevice(usb_host), MIDIDevice(usb_host)};
bool usb_midi_dev_connected[MIDI_HOST_DEVICE_POOL] = {false, false};

uint32_t last_millis = 0;

int8_t lumi_dev_index = -1;
int8_t ewi_dev_index = -1;

// 15 channels total; reserve channel 1 (MPE 'lower')
bool channel_in_use[16] = {false, false, false, false, false, false,
                           false, false, false, false, false, false,
                           false, false, false, false};
int8_t channel_remap[16] = {-1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1};

uint8_t remap_channel(uint8_t orig_channel) {
  bool remap = false;
  uint8_t orig_index = orig_channel - 1;

  if (orig_channel == 1 && RESERVE_CHANNEL_1) {
    remap = true;
  } else if (orig_channel == 16 && RESERVE_CHANNEL_16) {
    remap = true;
  } else if (channel_remap[orig_index] > -1) {
    remap = true;
  } else {
    // Remap if a remap has already claimed a channel
    for (uint8_t c = 0; c < 16; c++) {
      if (c != orig_index && channel_remap[c] == orig_index) {
        remap = true;
        break;
      }
    }
  }

  if (remap) {
    if (channel_remap[orig_index] > -1) {
      // Keep reusing remapped channel until it goes away
      Serial.printf("Remapped(1) channel %u to %u\n", orig_channel,
                    channel_remap[orig_index] + 1);
      return channel_remap[orig_index];
    }
    // find free channel to map to
    for (uint8_t c = 15 - RESERVE_CHANNEL_16; c >= RESERVE_CHANNEL_1; c--) {
      bool c_already_mapped = false;
      // Make sure another channel isn't mapping to this channel
      for (uint8_t c2 = 0; c2 < 16; c2++) {
        if (channel_remap[c2] == c) {
          c_already_mapped = true;
          break;
        }
      }
      if (channel_remap[c] < 0 && !c_already_mapped) {
        channel_remap[orig_index] = c;
        Serial.printf("Remapped(2) channel %u to %u\n", orig_channel, c + 1);
        return c + 1;
      }
    }

    // If we make it here there are no free channels to remap to.
    return -1;
  }

  return Serial.printf("Using original channel %u\n", orig_channel);
  return orig_channel;
}

void clear_channel_remap(uint8_t orig_channel) {
  channel_remap[orig_channel - 1] = -1;
}

void empty_real_time_handler(unsigned char realtimebyte) {
}

void lumi_real_time_handler(unsigned char realtimebyte) {
#if defined(MIDI_DEBUG)
  Serial.print("Lumi Real Time Message, code=");
  Serial.println(realtimebyte, HEX);
#endif
  usbMIDI.sendRealTime(realtimebyte, LUMI_DEVICE_CABLE);
}

void empty_active_sensing_handler() {
}

void lumi_active_sensing_handler() {
#if defined(MIDI_DEBUG)
  Serial.println("Lumi Active Sensing");
#endif
}

void empty_system_reset_handler() {
}

void lumi_system_reset_handler() {
#if defined(MIDI_DEBUG)
  Serial.println("Lumi System Reset");
#endif
}

void empty_note_on_handler(byte channel, byte note, byte velocity) {
  // do nothing
}

void lumi_note_on_handler(byte channel, byte note, byte velocity) {
  // When a USB device with multiple virtual cables is used,
  // midi1.getCable() can be used to read which of the virtual
  // MIDI cables received this message.

#if defined(MIDI_DEBUG)
  Serial.print("Lumi Note On, ch=");
  Serial.print(channel, DEC);
  Serial.print(", note=");
  Serial.print(note, DEC);
  Serial.print(", velocity=");
  Serial.println(velocity, DEC);
#endif
  byte qnote = quantizer.quantize_note(note);
  byte new_channel = remap_channel(channel);
  if (qnote >= 0)
    usbMIDI.sendNoteOn(qnote, velocity, new_channel, LUMI_DEVICE_CABLE);
  LED::blink();
}

void ewi_note_on_handler(byte channel, byte note, byte velocity) {
#if defined(MIDI_DEBUG)
  Serial.print("EWI Note On, ch=");
  Serial.print(channel, DEC);
  Serial.print(", note=");
  Serial.print(note, DEC);
  Serial.print(", velocity=");
  Serial.println(velocity, DEC);
#endif
  byte qnote = quantizer.quantize_note(note);
  if (qnote >= 0)
    usbMIDI.sendNoteOn(qnote, velocity, channel, EWI_DEVICE_CABLE);
  LED::blink();
}

void empty_note_off_handler(byte channel, byte note, byte velocity) {
  // do nothing
}

void lumi_note_off_handler(byte channel, byte note, byte velocity) {
#if defined(MIDI_DEBUG)
  Serial.print("Lumi Note Off, ch=");
  Serial.print(channel, DEC);
  Serial.print(", note=");
  Serial.print(note, DEC);
  Serial.print(", velocity=");
  Serial.println(velocity, DEC);
#endif
  byte qnote = quantizer.quantize_note(note);
  byte new_channel = remap_channel(channel);
  if (qnote >= 0) {
    usbMIDI.sendNoteOff(qnote, velocity, new_channel, LUMI_DEVICE_CABLE);
    clear_channel_remap(channel);
  }
  LED::blink();
}

void ewi_note_off_handler(byte channel, byte note, byte velocity) {
#if defined(MIDI_DEBUG)
  Serial.print("EWI Note Off, ch=");
  Serial.print(channel, DEC);
  Serial.print(", note=");
  Serial.print(note, DEC);
  Serial.print(", velocity=");
  Serial.println(velocity, DEC);
#endif
  byte qnote = quantizer.quantize_note(note);
  if (qnote >= 0) {
    usbMIDI.sendNoteOff(qnote, velocity, channel, EWI_DEVICE_CABLE);
  }
  LED::blink();
}

void empty_aftertouch_poly_handler(byte channel, byte note, byte velocity) {
}

void lumi_aftertouch_poly_handler(byte channel, byte note, byte velocity) {
#if defined(MIDI_DEBUG)
  Serial.print("Lumi AfterTouch (Poly) Change, ch=");
  Serial.print(channel, DEC);
  Serial.print(", note=");
  Serial.print(note, DEC);
  Serial.print(", velocity=");
  Serial.println(velocity, DEC);
#endif
  byte qnote = quantizer.quantize_note(note);
  byte new_channel = remap_channel(channel);
  if (qnote >= 0)
    usbMIDI.sendAfterTouchPoly(qnote, velocity, new_channel, LUMI_DEVICE_CABLE);
}

void empty_control_change_handler(byte channel, byte control, byte value) {
}

void lumi_control_change_handler(byte channel, byte control, byte value) {
#if defined(MIDI_DEBUG)
  Serial.print("Lumi Control Change, ch=");
  Serial.print(channel, DEC);
  Serial.print(", control=");
  Serial.print(control, DEC);
  Serial.print(", value=");
  Serial.println(value, DEC);
#endif
  usbMIDI.sendControlChange(control, value, channel, LUMI_DEVICE_CABLE);
}

void ewi_control_change_handler(byte channel, byte control, byte value) {
#if defined(MIDI_DEBUG)
  Serial.print("EWI Control Change, ch=");
  Serial.print(channel, DEC);
  Serial.print(", control=");
  Serial.print(control, DEC);
  Serial.print(", value=");
  Serial.println(value, DEC);
#endif
  usbMIDI.sendControlChange(control, value, channel, EWI_DEVICE_CABLE);
}

void empty_program_change_handler(byte channel, byte program) {
}

void lumi_program_change_handler(byte channel, byte program) {
#if defined(MIDI_DEBUG)
  Serial.print("Lumi Program Change, ch=");
  Serial.print(channel, DEC);
  Serial.print(", program=");
  Serial.println(program, DEC);
#endif
  usbMIDI.sendProgramChange(program, channel, LUMI_DEVICE_CABLE);
}

void ewi_program_change_handler(byte channel, byte program) {
#if defined(MIDI_DEBUG)
  Serial.print("EWI Program Change, ch=");
  Serial.print(channel, DEC);
  Serial.print(", program=");
  Serial.println(program, DEC);
#endif
  usbMIDI.sendProgramChange(program, channel, EWI_DEVICE_CABLE);
}

void empty_aftertouch_channel_handler(byte channel, byte pressure) {
}

void lumi_aftertouch_channel_handler(byte channel, byte pressure) {
#if defined(MIDI_DEBUG)
  Serial.print("Lumi After Touch, ch=");
  Serial.print(channel, DEC);
  Serial.print(", pressure=");
  Serial.println(pressure, DEC);
#endif
  usbMIDI.sendAfterTouch(pressure, channel, LUMI_DEVICE_CABLE);
}

void ewi_aftertouch_channel_handler(byte channel, byte pressure) {
#if defined(MIDI_DEBUG)
  Serial.print("EWI After Touch, ch=");
  Serial.print(channel, DEC);
  Serial.print(", pressure=");
  Serial.println(pressure, DEC);
#endif
  usbMIDI.sendAfterTouch(pressure, channel, EWI_DEVICE_CABLE);
}

void empty_pitch_change_handler(byte channel, int pitch) {
}

void lumi_pitch_change_handler(byte channel, int pitch) {
#if defined(MIDI_DEBUG)
  Serial.print("Lumi Pitch Change, ch=");
  Serial.print(channel, DEC);
  Serial.print(", pitch=");
  Serial.println(pitch, DEC);
#endif
  byte new_channel = remap_channel(channel);
  usbMIDI.sendPitchBend(pitch, new_channel, LUMI_DEVICE_CABLE);
}

void ewi_pitch_change_handler(byte channel, int pitch) {
#if defined(MIDI_DEBUG)
  Serial.print("EWI Pitch Change, ch=");
  Serial.print(channel, DEC);
  Serial.print(", pitch=");
  Serial.println(pitch, DEC);
#endif
  usbMIDI.sendPitchBend(pitch, channel, EWI_DEVICE_CABLE);
}

void empty_sysex_chunk_handler(const byte *data, uint16_t length, bool last) {
}

bool is_lumi(MIDIDevice midi_dev) {
  for (uint8_t v = 0; v < LUMI_VENDOR_COUNT; v++) {
    if (midi_dev.idVendor() == lumi_vendor_id[v]) {
      for (uint8_t p = 0; p < LUMI_PRODUCT_COUNT; p++) {
        if (midi_dev.idProduct() == lumi_product_id[p]) {
          return true;
        }
      }
    }
  }
  return false;
}

bool is_ewi(MIDIDevice midi_dev) {
  for (uint8_t v = 0; v < EWI_VENDOR_COUNT; v++) {
    if (midi_dev.idVendor() == ewi_vendor_id[v]) {
      for (uint8_t p = 0; p < EWI_PRODUCT_COUNT; p++) {
        if (midi_dev.idProduct() == ewi_product_id[p]) {
          return true;
        }
      }
    }
  }
  return false;
}

MIDIDevice *get_lumi_dev() {
  if (lumi_dev_index != -1) {
    return &usb_midi_dev[lumi_dev_index];
  }
  return NULL;
}

MIDIDevice *get_ewi_dev() {
  if (ewi_dev_index != -1) {
    return &usb_midi_dev[ewi_dev_index];
  }
  return NULL;
}

void begin() {
  usb_host.begin();
}

void tick() {
  uint32_t current_millis = millis();

  usb_host.Task();
  for (uint8_t i = 0; i < MIDI_HOST_DEVICE_POOL; i++) {
    usb_midi_dev[i].read();
  }

  if (current_millis > last_millis + 500) {
    for (uint8_t i = 0; i < MIDI_HOST_DEVICE_POOL; i++) {
      if (usb_midi_dev_connected[i] != (bool)usb_midi_dev[i]) {
        if (usb_midi_dev_connected[i] == false) {
          if (is_lumi(usb_midi_dev[i])) {
            usb_midi_dev_connected[i] = true;
            lumi_dev_index = i;
            MIDIDevice *lumi_dev = &usb_midi_dev[i];

            Serial.println("USBHost: lumi device connected.");

            lumi_dev->setHandleNoteOn(lumi_note_on_handler);
            lumi_dev->setHandleNoteOff(lumi_note_off_handler);
            lumi_dev->setHandleAfterTouchChannel(lumi_aftertouch_channel_handler);
            lumi_dev->setHandleAfterTouchPoly(lumi_aftertouch_poly_handler);
            lumi_dev->setHandlePitchChange(lumi_pitch_change_handler);
            lumi_dev->setHandleSystemExclusive(lumi_sysex_chunk_handler);

            Serial.println("Fetching Enumeration.");
            send_enumerate(*lumi_dev);

            Serial.println("Fetching Configuration.");
            send_read_configuration(*lumi_dev);

            set_mpe_mode(*lumi_dev, get_current_mpe_mode());
            set_key(*lumi_dev, get_current_key());
            set_scale(*lumi_dev, get_current_scale());
            Quantizer::quantizer.set_mode(Quantizer::quantizer.get_mode());

            set_global_color(*lumi_dev, Settings::last_values.global_r, Settings::last_values.global_g, Settings::last_values.global_b);
            set_root_color(*lumi_dev, Settings::last_values.root_r, Settings::last_values.root_g, Settings::last_values.root_b);

            lumi_dev->send_now();
          } else if (is_ewi(usb_midi_dev[i])) {
            usb_midi_dev_connected[i] = true;
            ewi_dev_index = i;
            MIDIDevice *ewi_dev = &usb_midi_dev[i];

            Serial.println("USBHost: ewi device connected.");

            ewi_dev->setHandleNoteOn(ewi_note_on_handler);
            ewi_dev->setHandleNoteOff(ewi_note_off_handler);
            ewi_dev->setHandleAfterTouchChannel(ewi_aftertouch_channel_handler);
            ewi_dev->setHandlePitchChange(ewi_pitch_change_handler);
            ewi_dev->setHandleControlChange(ewi_control_change_handler);
          }
        }

        if (usb_midi_dev_connected[i]) {
          if (usb_midi_dev[i].idVendor() == 0 &&
              usb_midi_dev[i].idProduct() == 0) {
            usb_midi_dev_connected[i] = false;
            if (lumi_dev_index == i) {
              lumi_dev_index = -1;
              Serial.println("USBHost: lumi device disconnected.");
            } else if (ewi_dev_index == i) {
              ewi_dev_index = -1;
              Serial.println("USBHost: ewi device disconnected.");
            }

            usb_midi_dev[i].setHandleNoteOn(empty_note_on_handler);
            usb_midi_dev[i].setHandleNoteOff(empty_note_off_handler);
            usb_midi_dev[i].setHandleAfterTouchChannel(empty_aftertouch_channel_handler);
            usb_midi_dev[i].setHandleAfterTouchPoly(empty_aftertouch_poly_handler);
            usb_midi_dev[i].setHandlePitchChange(empty_pitch_change_handler);
            usb_midi_dev[i].setHandleProgramChange(empty_program_change_handler);
            usb_midi_dev[i].setHandleControlChange(empty_control_change_handler);
            usb_midi_dev[i].setHandleActiveSensing(empty_active_sensing_handler);
            usb_midi_dev[i].setHandleSystemReset(empty_system_reset_handler);
            usb_midi_dev[i].setHandleRealTimeSystem(empty_real_time_handler);
            usb_midi_dev[i].setHandleSystemExclusive(empty_sysex_chunk_handler);
          }
        }
      }
    }
  }
}

} // namespace LMUSBHost
