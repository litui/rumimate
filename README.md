# RumiMate (+EWI Support)

This code is provided as-is. I wrote this for my own use rather than to distribute or profit from, but I provide it here in case it is of use to others. I'll update as I make revisions for myself but will likely not provide fixes by request.

## Why this exists:

The Roli/Lumi Keys Studio Edition is a great little LED-backlit, polyphonic aftertouch keyboard. That said, most of the controls are in software which is less than ideal for anyone running "DAWless". I needed a way to change the LED root note and scale without having to connect the keyboard to my computer.

## Features:

* Change the root note and scale while "DAWless"
* (in code) set poly notes/aftertouch/pitch bend to stay within channels 2-15 (and remap everything on the fly) to avoid worry about MPE "upper" or "lower" modes.
    * This means you can only hold down max 14 keys at a time. Sorry to all those with 7 fingers per hand.
* (in code) define 6 incoming midi CCs to control the RGB for the root note and global colours.
* Set a quantizer mode to lock outbound notes into the selected scale. Options: "filter", (round) "up", (round) "down"
* Support (on virtual cable #2) for USB connection of an AKAI Electronic Wind Instrument (EWI) alongside the Lumi. The only difference this makes is allowing the quantizer setting to affect notes played on the EWI as well.

## Future features maybe? If I get around to it.

* Arpeggiator (based on inbound midi clock signal) with current playing note reflected on the Lumi

## Required Hardware

* A Teensy 4.0 board with USBHost pads and 5v power connected to a USB A or USB C port
* A 128x64 SH1107 I2C display (I use the [Adafruit OLED FeatherWing](https://www.adafruit.com/product/4650))
* A rotary encoder like [this one from Adafruit](https://www.adafruit.com/product/377)
* Whatever wire, solder, and perfboard you may need to connect it all together

## Required Software

* PlatformIO (works best as a VSCode Extension) to build and deploy to your Teensy

## Debugging

You can enable debug messages as needed by uncommenting the ```MIDI_DEBUG``` or ```USBHOST_PRINT_DEBUG``` build flags in ```platformio.ini``` if you like, and view them using USBSerial to the Teensy.

## Known issues

* The current configuration exposes 16 virtual midi devices over USB even though only two are neccesary. The arduino configuration also allows for a 4 midi device setup but that was causing crashes in AUM on my iPad for some reason. You'll just have to put up with 14 useless virtual midi devices for now.
* My display and rotary encoder handling is a bit sub-optimal. But it works.