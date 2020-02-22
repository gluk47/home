#pragma once

#include "common.h"
#include "handlers.h"
#include <chrono>
#include <map>

struct TLightSensor : public Handler {
    int pin = A0;
    int Darkness = 500; // 0 .. 1024
    int Hysteresis = 50; // distance to darkness to switch the reported state
    std::chrono::milliseconds StabilizationDelay = 250ms;

    TLightSensor()
    : Handler("Light sensor")
    {
        pinMode(pin, INPUT);
    }

    void handle(std::chrono::milliseconds now) override;

    int Value() const noexcept { return Value_; }
    // with stabilizationDelay
    bool IsDark() const;
    bool IsDarkNow() const {
        if (IsDark_)
            return Value_ < Darkness + Hysteresis;
        else
            return Value_ > Darkness - Hysteresis;
    }

    std::map<String, String> debug() const override {
        return {
            {"Darkness", String(Darkness)},
            {"DelayLeft", ToString(TimeToStabilize)},
            {"Hysteresis", String(Hysteresis)},
            {"IsDark()", YesNo(IsDark())},
            {"IsDarkNow()", YesNo(IsDarkNow())},
            {"LastUpdate", ToString(LastUpdate)},
            {"Value", String(Value_)}
        };
    }

private:
    int Value_ = 1024;
    std::chrono::milliseconds TimeToStabilize{0};
    bool IsDark_ = true;
    std::chrono::milliseconds LastUpdate{0};
};

inline void TLightSensor::handle(std::chrono::milliseconds now) {
    std::chrono::milliseconds timePassed = BoardTimeDifference(LastUpdate, now);
    TimeToStabilize = timePassed > TimeToStabilize ? 0ms : TimeToStabilize - timePassed;
    LastUpdate = now;
    bool wasDark = IsDarkNow();
    int oldValue = Value_;
    Value_ = analogRead(pin);
    bool becameDark = IsDarkNow();
    if (wasDark ^ becameDark) {
        int diff = abs(Value_ - oldValue);
        TimeToStabilize = StabilizationDelay + StabilizationDelay * (diff < 50) * 4 + StabilizationDelay * (diff < 100) * 2 + StabilizationDelay * (diff < 150);
    }
    if (TimeToStabilize <= 0ms)
        IsDark_ = IsDarkNow();
    //Serial.printf("light: %d\n", Value_);
    //if (TimeToStabilize == 0)
    //  Serial.println("Is dark -> %s" + becameDark ? "yes" : "no");
}

inline bool TLightSensor::IsDark() const {
    return IsDark_;
}

struct TNightLightController {
    bool ShouldSwitchOn() const {
        return sensor.IsDark();
    };

    const TLightSensor& sensor;
};
