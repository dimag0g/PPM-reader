# PPM Reader for Arduino

PPM Reader is an interrupt based [pulse-position modulation](https://en.wikipedia.org/wiki/Pulse-position_modulation) (PPM) signal reading library for Arduino. Its purpose is to provide an easy to use, non-blocking solution for decoding the signal from an RC receiver that is able to encode data from multiple channels as PPM.

A typical PPM signal which PPM Reader is able to decode is shown below:

<p align="center">
    <img src="PPM-signal.png?raw=true" width="800">
</p>

The signal consists of pulses (usually 300-500 microseconds) which delimit time intervals. Smaller intervals correspond to individual channel values (in microseconds) inside the frame, and a larger interval (blank time) delimits separate frames.

The number of channels in a frame is supposed to be constant, however, the library will accept frames with a variable number of channels:

- if fewer channels are transmitted, non-transmitted channels will keep their old values
- if more channels are transmitted than the library was configured to accept, extra channels will be discarded

Since the PPM reader works with pulse edges, it should be able to accept signals of either polarity, provided that the pulses have constant width. Alternatively, the input signal can have varying pulse widths, but then the pulse positions will be determined by the **rising** edges. The resolution of the channel values is the same as the resolution of `micros()`.

Using interrupts (instead of `pulseIn()` or some equivalent) for measurement means that decoding the signal happens in a non-blocking way. Using PPM Reader doesn't significantly slow down your program's code and you can do any other timing sensitive processes in your program meanwhile reading the incoming PPM signals.

This version is a fork of https://github.com/Nikkilae/PPM-reader/ with modifications.

## Usage

### Installation
If you use Arduino IDE, you should install PPM reader via the built-in library manager.

If you prefer to install it manually, download the release zip file and place the files in the Arduino library directory, e.g. `Documents\Arduino\libraries\PPM-reader`.

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
    delay(20);
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

If you don't have any RC equipment, you should be able to produce a test PPM signal using [PPMEncoder](https://github.com/schinken/PPMEncoder).

Please mention any issues, bugs or improvement ideas.