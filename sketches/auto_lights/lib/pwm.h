#pragma once

#include "handlers.h"

struct TPwm : public Handler {
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
    : Handler("PWM_"_str + pin, period)
    , pin(pin)
    , enabled(std::move(enabled))
    , httpSwitch(httpSwitch)
    , light(light)
    , StepDuration(period)
    , Description(description)
    , HttpID(id)
    {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    }

    void handle(std::chrono::milliseconds) override {
        (++*this).write();
    }

    std::map<String, String> debug() const override {
        return {
            {"Type", "PWM"},
            {"Pin", ToString(pin)},
            {"HttpId", ToString(HttpID)},
            {"Name", Description},
            {"Value", String(value)},
            {"Step", String(step)},
            {"Range", String(range)},
            {"HttpEnabled", OnOff(this->httpSwitch)},
            {"Light", this->light.IsDark() ? "dark" : "not dark"},
            {"EnabledPeriod", YesNo(this->enabled[enabled_i])},
            {"SwitchedOn", YesNo(this->isSwitchedOn())}
        };
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

private:
    const char* Description;
    int HttpID;
};
