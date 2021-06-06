# PPM Reader for Arduino

PPM Reader is an interrupt based [pulse-position modulation](https://en.wikipedia.org/wiki/Pulse-position_modulation) (PPM) signal reading library for Arduino. Its purpose is to provide an easy to use, non-blocking solution for reading PPM signals from an RC receiver that is able to output channel data as PPM.

Using interrupts (instead of pulseIn or some equivalent) to detect pulses means that reading the signal can be done in a non-blocking way. This means that using PPM Reader doesn't significantly slow down your program's code and you can do any other timing sensitive processes in your program meanwhile reading the incoming PPM signals.

This version is a fork of https://github.com/Nikkilae/PPM-reader/ with the following modifications:

- the dependency on InterruptHandler library (limited to AVR architecture) has been removed, so PPM Reader should be compatible with non-AVR boards such as ESP32. On the downside, only a single PPMReader object can receive interrupts (the library could be extended to support several objects).
- variables are reduced to 16 bit where possible, saving some RAM. Most radios have PPM signals and timeouts in the range of 500-5000 microseconds, so values exceeding 65000 are essentially non-existent in practice.
- `latestValidChannelValue()` no longer returns zero if no PPM frame is ever received (it returns `defaultValue` instead). Returning zero is considered a bug because the valid value is supposed to be in the min/max range.
- added a new parameter, `failsafeTimeout`, which makes `latestValidChannelValue()` return `defaultValue` if no PPM signal is detected for a certain time (0.5s by default). Note that this doesn't help with radios which keep sending data over PPM even if the RF signal is lost.

## Usage

### Installation
Download the contents of this repository on your computer. Move everything in the PPM-reader directory in your Arduino library directory.

### Arduino setup and code

Connect your RC receiver's PPM pin to your Arduino board's digital pin. Make sure that you use a pin that supports interrupts. You can find information on that from the [Arduino language reference](https://www.arduino.cc/en/Reference/AttachInterrupt).

* Include the library PPMReader.h to your program.
* Initialize a PPMReader object with its constructor `PPMReader(interruptPin, channelAmount);`.
* Read channel values from the PPMReader object's public methods
	* Use `latestValidChannelValue(channel, defaultValue)` to read the latest value for the channel that was considered valid (in between the predetermined minimum and maximum channel values).
	* Alternatively use `rawChannelValue(channel)` to read the latest raw (not necessarily valid) channel value. The contents of the raw channel values may differ depending on your RC setup. For example some RC devices may output "illegal" channel values in the case of signal loss or failure and so you may be able to detect the need for a failsafe procedure.

When referring to channel numbers in the above methods, note that channel numbers start from 1, not 0.

### Example Arduino sketch
```c++

#include <PPMReader.h>

// Initialize a PPMReader on digital pin 3 with 6 expected channels.
byte interruptPin = 3;
byte channelAmount = 6;
PPMReader ppm(interruptPin, channelAmount);

void setup() {
    Serial.begin(115200);
}

void loop() {
    // Print latest valid values from all channels
    for (byte channel = 1; channel <= channelAmount; ++channel) {
        unsigned value = ppm.latestValidChannelValue(channel, 0);
        Serial.print(String(value) + "\t");
    }
    Serial.println();
}

```

### Fine tuning
The PPMReader class should work with default settings for many regular RC receivers that output PPM. The default settings are as follows (all of them represent time in microseconds):
* `minChannelValue = 1000` The minimum possible channel value. Should be smaller than maxChannelValue.
* `maxChannelValue = 2000` The maximum possible channel value. Should be greater than minChannelValue.
* `channelValueMaxError = 10` The maximum error in channel value to either direction for still considering the channel value as valid. This leeway is required because your PPM output may have tiny errors and also the Arduino board's refresh rate only allows a [limited resolution for timing functions](https://www.arduino.cc/en/Reference/Micros).
* `blankTime = 2100` The time between pulses that will be considered the end to a signal frame. This should be greater than maxChannelValue + channelValueMaxError.
* `failsafeTimeout = 500000L` The timeout after which the signal is considered lost, and `latestValidChannelValue()` starts returning `defaultValue` instead of the last received value. Should be greater than the total time it takes to transmit several complete frames.

You can modify any of the above settings directly from the PPMReader object. They are public unsigned variables.

## Testing
The library has been tested and proven to work on the following setup:
* An Arduino Nano board
* FlySky FS-i6X transmitter with the FS-iA8S receiver
* Up to 8 channels

Please mention any issues, bugs or improvement ideas.