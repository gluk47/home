#pragma once

#include "handlers.h"

struct TPwm {
    int value = 0;
    int step = 1;
    int range = 1024;
    int pin;
    // which half-periods to enable light (vector of any length)
    std::vector<bool> enabled = {true};
    int enabled_i = 0;
    const bool& httpSwitch;
    const TLightSensor& light;
    uint64_t ms_per_step = 5;

    TPwm(int pin, std::vector<bool>&& enabled, bool& httpSwitch, const TLightSensor& light, int id, const char* description, int period = 5)
    : pin(pin)
    , enabled(std::move(enabled))
    , httpSwitch(httpSwitch)
    , light(light)
    , ms_per_step(period) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);

        Handlers::add([this](int){
            (++*this).write();
        }, ms_per_step);

        String whoami = "PWM '"_str + description + "' (pin=" + pin + ", http id=" + id + ")";
        Handlers::addDebug(whoami, [this]{
            return std::map<String, String> {
                {"Value", String(value)},
                {"Step", String(step)},
                {"Range", String(range)},
                {"HttpEnabled", this->httpSwitch ? "on" : "off"},
                {"Light", this->light.IsDark() ? "dark" : "not dark"},
                {"EnabledPeriod", this->enabled[enabled_i] ? "yes" : "no"},
                {"SwitchedOn", this->isSwitchedOn() ? "yes" : "no"}
            };
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

    bool isSwitchedOn() const {
        return httpSwitch and light.IsDark() and enabled[enabled_i];
    }

    void write() {
        if (isSwitchedOn())
            analogWrite(pin, value);
        else
            analogWrite(pin, 0);
    }
};
