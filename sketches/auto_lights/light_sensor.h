#pragma once

#include "common.h"
#include "handlers.h"
#include <chrono>
#include <map>

struct TLightSensor {
    int pin = A0;
    int Darkness = 500; // 0 .. 1024
    int Hysteresis = 50; // distance to darkness to switch the reported state
    std::chrono::milliseconds StabilizationDelay = 250ms;

    TLightSensor() {
        pinMode(pin, INPUT);
        Handlers::add([this](std::chrono::milliseconds when){
            Update(when);
        });
        Handlers::addDebug("LightSensor", [this]{ return GetState(); });
    }

    int Value() const noexcept { return Value_; }
    void Update(std::chrono::milliseconds when);
    // with stabilizationDelay
    bool IsDark() const;
    bool IsDarkNow() const {
        if (IsDark_)
            return Value_ < Darkness + Hysteresis;
        else
            return Value_ > Darkness - Hysteresis;
    }
    std::map<String, String> GetState() const {
        return {
            {"Darkness", String(Darkness)},
            {"DelayLeft", String(TimeToStabilize)},
            {"Hysteresis", String(Hysteresis)},
            {"IsDark()", YesNo(IsDark())},
            {"IsDarkNow()", YesNo(IsDarkNow())}
            {"LastUpdate", String(LastUpdate) + " ms"},
            {"Value", String(Value_)},
        };
    }

private:
    int Value_ = 1024;
    int TimeToStabilize = 0;
    mutable bool IsDark_ = true;
    std::chrono::milliseconds LastUpdate{0};
};

void TLightSensor::Update(std::chrono::milliseconds now) {
    int timePassed = BoardTimeDifference(LastUpdate, now);
    TimeToStabilize = timePassed > TimeToStabilize ? 0 : TimeToStabilize - timePassed;
    LastUpdate = now;
    bool wasDark = IsDarkNow();
    int oldValue = Value_;
    Value_ = analogRead(pin);
    bool becameDark = IsDarkNow();
    if (wasDark ^ becameDark) {
        int diff = abs(Value_ - oldValue);
        TimeToStabilize = Stabilization_ms + Stabilization_ms * (diff < 50) * 4 + Stabilization_ms * (diff < 100) * 2 + Stabilization_ms * (diff < 150);
    }
    //Serial.printf("light: %d\n", Value_);
    //if (TimeToStabilize == 0)
    //  Serial.println("Is dark -> %s" + becameDark ? "yes" : "no");
}

bool TLightSensor::IsDark() const {
    if (TimeToStabilize <= 0)
        IsDark_ = IsDarkNow();
    return IsDark_;
}

struct TNightLightController {
    bool ShouldSwitchOn() const {
        return sensor.IsDark();
    };

    const TLightSensor& sensor;
}
