#pragma once

#include "handlers.h"
#include <map>

struct TLightSensor {
    int pin = A0;
    int Darkness = 450; // 0 .. 1024
    static constexpr int Hysteresis_s = 10;

    TLightSensor(String&& name) {
        pinMode(pin, INPUT);
        Handlers::add([this](int when){
            update(when);
        });
        Handlers::addDebug("LightSensor." + name, [this]{ return GetState(); });
    }

    int value() const noexcept { return value_; }
    void update(int when);
    // with hysteresis
    bool IsDark() const;
    bool IsDarkNow() const { return value_ < Darkness; }
    std::map<String, String> GetState() const {
        return {
            {"value", String(value_)},
            {"darkness", String(Darkness)},
            {"lastUpdate", String(lastUpdate) + " ms"},
            {"hysteresis", String(hysteresis)},
            {"IsDark()", IsDark() ? "yes" : "no"},
            {"IsDarkNow()", IsDarkNow() ? "yes" : "no"}
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
    value_ = analogRead(pin);
    bool becameDark = IsDarkNow();
    if (wasDark ^ becameDark)
        hysteresis = Hysteresis_s * 1000;
    //Serial.printf("light: %d\n", value_);
    //if (hysteresis == 0)
    //  Serial.println("Is dark -> %s" + becameDark ? "yes" : "no");
}

bool TLightSensor::IsDark() const {
    if (hysteresis <= 0)
        isDark_ = IsDarkNow();
    return isDark_;
}
