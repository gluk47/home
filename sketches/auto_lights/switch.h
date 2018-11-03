#pragma once

#include "common.h"
#include "handlers.h"
#include "light_sensor.h"

struct TSwitch {
    int pin;
    const bool& httpSwitch;
    const TLightSensor* light = {};

    TSwitch(int pin, const bool& http, const TLightSensor* light = nullptr)
    : pin(pin)
    , httpSwitch(http)
    , light(light) {
        pinMode(pin, OUTPUT);

        Handlers::add([this](int when){
            write(when);
        });
    }

    void write(int now) const {
        if (now != last_report && now % 5000 < 10) {
            last_report = now;
            Serial.println(
                "[pin "_str + pin + "] "
                + (light ? "IsDark? "_str + light->IsDark() + "; " : "")
                + "http? " + httpSwitch
            );
        }
        const int new_state = ((!light or light->IsDark()) and httpSwitch) ? HIGH : LOW;
        if (new_state != state) {
            digitalWrite(pin, new_state);
            state = new_state;
        }
    }

private:
    mutable int last_report = -1;
    mutable int state = LOW;
};

