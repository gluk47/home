#pragma once

#include "common.h"
#include "handlers.h"
#include "light_sensor.h"

struct TSwitch {
    int pin;

    TSwitch(int pin, int httpId, const char* description)
    : pin(pin)
    {
        pinMode(pin, OUTPUT);
        Handlers::addDebug("Switch_"_str + pin, [=]{
            return std::map<String, String> {
                {"Id", ToString(httpId)},
                {"Name", description},
                {"Pin", ToString(pin)},
                {"State", OnOff(SwitchedOn)},
                {"Type", "Switch"},
            };
        });
    }

    void TurnOn(bool desired) {
        if (SwitchedOn != desired) {
            digitalWrite(pin, desired ? HIGH : LOW);
            SwitchedOn = desired;
        }
    }

    bool TurnedOn() const noexcept {
        return SwitchedOn;
    }

private:
    bool SwitchedOn = false;
};

