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
    std::chrono::milliseconds StepDuration = 5ms;

    TPwm(
        int pin,
         std::vector<bool>&& enabled,
         bool& httpSwitch,
         const TLightSensor& light,
         int id,
         const char* description,
         std::chrono::milliseconds period = 5ms
    )
    : pin(pin)
    , enabled(std::move(enabled))
    , httpSwitch(httpSwitch)
    , light(light)
    , StepDuration(period) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);

        Handlers::add([this](std::chrono::milliseconds){
            (++*this).write();
        }, StepDuration);

        Handlers::addDebug("PWM_"_str + pin, [=]{
            return std::map<String, String> {
                {"Type", "PWM"},
                {"Pin", ToString(pin)},
                {"HttpId", ToString(id)},
                {"Name", description},
                {"Value", String(value)},
                {"Step", String(step)},
                {"Range", String(range)},
                {"HttpEnabled", OnOff(this->httpSwitch)},
                {"Light", this->light.IsDark() ? "dark" : "not dark"},
                {"EnabledPeriod", YesNo(this->enabled[enabled_i])},
                {"SwitchedOn", YesNo(this->isSwitchedOn())}
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
