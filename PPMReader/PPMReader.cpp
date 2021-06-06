/*
Copyright 2016 Aapo Nikkilä
Copyright 2021 Dmitry Grigoryev

This file is part of PPM Reader.

PPM Reader is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

PPM Reader is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with PPM Reader.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "PPMReader.h"

PPMReader::PPMReader(byte interruptPin, byte channelAmount):
    interruptPin(interruptPin), channelAmount(channelAmount) {
    // Setup an array for storing channel values
    rawValues = new unsigned [channelAmount];
    validValues = new unsigned [channelAmount];
    for (int i = 0; i < channelAmount; ++i) {
        rawValues[i] = 0;
        validValues[i] = 0;
    }
    // Attach an interrupt to the pin
    pinMode(interruptPin, INPUT);
    attachInterrupt(digitalPinToInterrupt(interruptPin), RISING);
}

PPMReader::~PPMReader() {
    detachInterrupt(digitalPinToInterrupt(interruptPin));
    delete [] rawValues;
    delete [] validValues;
}

void PPMReader::handleInterrupt(int8_t interruptNum) {
    // Remember the current micros() and calculate the time since the last pulseReceived()
    unsigned long previousMicros = microsAtLastPulse;
    microsAtLastPulse = micros();
    unsigned long time = microsAtLastPulse - previousMicros;

    if (time > blankTime) {
        /* If the time between pulses was long enough to be considered an end
         * of a signal frame, prepare to read channel values from the next pulses */
        pulseCounter = 0;
    }
    else {
        // Store times between pulses as channel values
        if (pulseCounter < channelAmount) {
            rawValues[pulseCounter] = time;
            if (time >= minChannelValue - channelValueMaxError && time <= maxChannelValue + channelValueMaxError) {
                validValues[pulseCounter] = constrain(time, minChannelValue, maxChannelValue);
            }
        }
        ++pulseCounter;
    }
}

unsigned PPMReader::rawChannelValue(byte channel) {
    // Check for channel's validity and return the latest raw channel value or 0
    unsigned value = 0;
    if (channel >= 1 && channel <= channelAmount) {
        noInterrupts();
        value = rawValues[channel-1];
        interrupts();
    }
    return value;
}

unsigned PPMReader::latestValidChannelValue(byte channel, unsigned defaultValue) {
    // Check for channel's validity and return the latest valid channel value or defaultValue.
    unsigned value = defaultValue;
    if (channel >= 1 && channel <= channelAmount) {
        noInterrupts();
        value = validValues[channel-1];
        interrupts();
    }
    return value;
}
