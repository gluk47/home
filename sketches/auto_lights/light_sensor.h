#pragma once

#include "handlers.h"
#include <map>

struct TLightSensor {
    int pin = A0;
    int Darkness = 400; // 0 .. 1024
    static constexpr int Hysteresis_s = 10;

    TLightSensor() {
        pinMode(pin, INPUT);
        Handlers::add([this](int when){
            update(when);
        });
    }

    int value() const noexcept { return value_; }
    void update(int when);
    // with hysteresis
    bool IsDark() const;
    bool IsDarkNow() const { return value_ < Darkness; }
    std::map<String, int> GetState() {
        return {
            {"value", value_},
            {"darkness", Darkness},
            {"lastUpdate", lastUpdate},
            {"hysteresis", hysteresis},
            {"IsDark()", IsDark()},
            {"IsDarkNow()", IsDarkNow()}
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
    Serial.printf("light: %d\n", value_);
    //if (hysteresis == 0)
    //  Serial.println("Is dark -> %s" + becameDark ? "yes" : "no");
}

bool TLightSensor::IsDark() const {
    if (hysteresis <= 0)
        isDark_ = IsDarkNow();
    return isDark_;
}
