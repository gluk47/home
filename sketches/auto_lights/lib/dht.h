#pragma once
#include "handlers.h"
#include <DHT.h>

struct TDht : public Handler {
    TDht(uint8_t pin = D4, uint8_t type = DHT22)
    : Handler("DHT (raw)")
    , sensor(pin, type)
    { }

    std::map<String, String> debug() const override {
        const float t = const_cast<DHT&>(sensor).readTemperature();
        const float h = const_cast<DHT&>(sensor).readHumidity();
        return {
            {"FeelsLike", String(const_cast<DHT&>(sensor).computeHeatIndex(t, h))},
            {"Humidity", String(h)},
            {"Temperature", String(t)}
        };
    }

    DHT sensor;
};

struct TTemperatureController : public Handler {
    TDht& Dht;
    float DesiredTemp = 22;
    float Hysteresis = 2; //degrees

    TTemperatureController(TDht& dht, float desiredTemp = 22.f, float hysteresis = 2.f)
    : Handler("DHT Controller")
    , Dht(dht), DesiredTemp(desiredTemp), Hysteresis(hysteresis)
    {
    }

    void handle(std::chrono::milliseconds) override {
        float margin = SwitchedOn ? +Hysteresis : -Hysteresis;
        SwitchedOn = Dht.sensor.readTemperature() > DesiredTemp + margin;
    }

    std::map<String, String> debug() const override {
        return {
            {"Temperature", ToString(Dht.sensor.readTemperature())},
            {"DesiredTemp", ToString(DesiredTemp)},
            {"Hysteresis", ToString(Hysteresis)}
        };
    }

    bool ShouldSwitchOn() const {
        return SwitchedOn;
    }

private:
    bool SwitchedOn = false;
};
