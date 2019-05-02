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

        Handlers::addDebug("Switch (pin="_str + pin + ")", [this]{
            return std::map<String, String> {
                {"State", state == LOW ? "off" : "on"},
                {"HttpEnabled", httpSwitch ? "on" : "off"},
                {"Light", this->light ? (this->light->IsDark() ? "dark" : "not dark") : "no sensor"}
            };
        });
    }

    void write(int now) const {
        if (now != last_report && now % 5000 < 10) {
            last_report = now;
            Serial.printf("[pin %d] IsDark? %s; http? %s\n", pin,
                (light ? (light->IsDark() ? "yes" : "no") : "null"),
                (httpSwitch ? "on" : "off")
            );
        }
        const int new_state = (httpSwitch and (!light or light->IsDark())) ? HIGH : LOW;
        if (new_state != state) {
            digitalWrite(pin, new_state);
            state = new_state;
        }
    }

private:
    mutable int last_report = -1;
    mutable int state = LOW;
};

