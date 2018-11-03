#pragma once

#include "handlers.h"

struct TPwm {
    int value = 0;
    int step = 1;
    uint64_t ms_per_step = 5;
    int range = 1024;
    int pin;
    // which half-periods to enable light (vector of any length)
    std::vector<bool> enabled = {true};
    int enabled_i = 0;
    const bool& httpSwitch;
    const TLightSensor& light;

    TPwm(int pin, std::vector<bool>&& enabled, bool& httpSwitch, const TLightSensor& light)
    : pin(pin)
    , enabled(std::move(enabled))
    , httpSwitch(httpSwitch)
    , light(light) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);

        Handlers::add([this](int){
            (++*this).write();
        });
    }

    TPwm& operator++() {
        value += step;
        if (value == range || value == 0) {
            step = -step;
            enabled_i++;
            enabled_i %= enabled.size();
        }
        return *this;
    }

    void write() {
        if (httpSwitch and light.IsDark() and enabled[enabled_i])
            analogWrite(pin, value);
        else
            analogWrite(pin, 0);
    }
};
