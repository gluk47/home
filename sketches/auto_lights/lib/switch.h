#pragma once

#include "common.h"
#include "handlers.h"
#include "light_sensor.h"

struct TSwitch : public Handler {
    int pin;

    TSwitch(int pin, int httpId, const char* description)
    : Handler("Switch "_str + description)
    , pin(pin)
    , HttpID(httpId)
    , Description(description)
    {
        pinMode(pin, OUTPUT);
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

    void handle(std::chrono::milliseconds) override {}

    std::map<String, String> debug() const override {
        return std::map<String, String> {
            {"Id", ToString(HttpID)},
            {"Name", Description},
            {"Pin", ToString(pin)},
            {"State", OnOff(SwitchedOn)},
            {"Type", "Switch"},
        };
    }

    const int HttpID;
    const char* Description;

private:
    bool SwitchedOn = false;
};

