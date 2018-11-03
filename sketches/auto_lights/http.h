#pragma once
#include "pins.h"

struct THttpSensor {
    enum light_t {
        EInvalid,
        EIndoor,
        EOutdoorDoor,
        EOutdoorPass,
        EUnused
    };
    bool enabled[EUnused + 1] = {};
    static const int pins[EUnused];

    bool& operator[] (int i) {
        return valid(i) ? enabled[i] : enabled[EUnused];
    }

    void toggle(int i, bool state) {
        if(not valid(i))
            return;
        enabled[i] = state;
    }

    THttpSensor() {
        for (int i = 1; i < EUnused; ++i) {
            enabled[i] = true;
            if (pins[i] != Pins::No) {
                pinMode(pins[i], OUTPUT);
                digitalWrite(pins[i], LOW);
            }
        }
    }

    static THttpSensor& the() {
        static THttpSensor sensor;
        return sensor;
    }

    static bool valid(int i) {
        return EInvalid < i and i < EUnused;
    }
};

const int THttpSensor::pins[THttpSensor::EUnused] = {
    // Switches directly controlled by Http should be listed here
    Pins::No, // EInvalid
    Pins::No,
    Pins::No,
    Pins::No
};
