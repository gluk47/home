#pragma once

#include "common.h"
#include "handlers.h"
#include <map>

struct TLightSensor {
    int pin = A0;
    int Darkness = 450; // 0 .. 1024
    static constexpr int Hysteresis_ms = 250;

    TLightSensor() {
        pinMode(pin, INPUT);
        Handlers::add([this](int when){
            update(when);
        });
        Handlers::addDebug("LightSensor", [this]{ return GetState(); });
    }

    int value() const noexcept { return value_; }
    void update(int when);
    // with hysteresis
    bool IsDark() const;
    bool IsDarkNow() const { return value_ < Darkness; }
    std::map<String, String> GetState() const {
        return {
            {"Value", String(value_)},
            {"Darkness", String(Darkness)},
            {"LastUpdate", String(lastUpdate) + " ms"},
            {"Hysteresis", String(hysteresis)},
            {"IsDark()", YesNo(IsDark())},
            {"IsDarkNow()", YesNo(IsDarkNow())}
        };
    }

private:
    int value_ = 1024;
    int hysteresis = 0;
    mutable bool isDark_ = true;
    int lastUpdate = -1;
};

void TLightSensor::update(int ms) {
    int timePassed = ms > lastUpdate ? ms - lastUpdate : ms;
    hysteresis = max(0, hysteresis - timePassed);
    lastUpdate = ms;
    bool wasDark = IsDarkNow();
    int oldValue = value_;
    value_ = analogRead(pin);
    bool becameDark = IsDarkNow();
    if (wasDark ^ becameDark) {
        int diff = abs(value_ - oldValue);
        hysteresis = Hysteresis_ms + Hysteresis_ms * (diff < 50) * 2 + Hysteresis_ms * (diff < 100) + Hysteresis_ms * (diff < 150) * .5;
    }
    //Serial.printf("light: %d\n", value_);
    //if (hysteresis == 0)
    //  Serial.println("Is dark -> %s" + becameDark ? "yes" : "no");
}

bool TLightSensor::IsDark() const {
    if (hysteresis <= 0)
        isDark_ = IsDarkNow();
    return isDark_;
}
