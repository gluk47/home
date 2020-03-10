#pragma once
#include "handlers.h"
#include <DHT.h>

struct TDht : public Handler {
    TDht(uint8_t pin = D4, uint8_t type = DHT22)
    : Handler("DHT (raw)", 5000ms)
    , sensor(pin, type)
    { }

    void init() override {
        getTemperature();
    }

    void handle(std::chrono::milliseconds) override {
        getTemperature();
    }

    std::map<String, String> debug() const override {
        const float t = getTemperature();
        const float h = const_cast<DHT&>(sensor).readHumidity();
        return {
            {"FeelsLike", String(const_cast<DHT&>(sensor).computeHeatIndex(t, h))},
            {"Humidity", String(h)},
            {"Temperature", String(t)}
        };
    }

    float getTemperature() const {
        return temperature;
    }

    float getTemperature() {
        float newTemp = sensor.readTemperature();
        if (!isnan(newTemp))
            temperature = newTemp;
        return temperature;
    }

private:
    mutable float temperature = NAN, humidity = NAN;
    DHT sensor;
};

struct TTemperatureThresholdSensor : public Handler {
    const TDht& Dht;
    float DesiredTemp = 22;
    float Hysteresis = 2; //degrees

    TTemperatureThresholdSensor(TDht& dht, float desiredTemp = 22.f, float hysteresis = 2.f)
    : Handler("DHT threshold sensor")
    , Dht(dht), DesiredTemp(desiredTemp), Hysteresis(hysteresis)
    {
    }

    void handle(std::chrono::milliseconds) override {
        float margin = SwitchedOn ? +Hysteresis : -Hysteresis;
        SwitchedOn = Dht.getTemperature() < DesiredTemp + margin;
    }

    std::map<String, String> debug() const override {
        return {
            {"Temperature", ToString(Dht.getTemperature())},
            {"DesiredTemp", ToString(DesiredTemp)},
            {"Hysteresis", ToString(Hysteresis)},
            {"SwitchedOn", YesNo(SwitchedOn)}
        };
    }

    bool ShouldSwitchOn() const {
        return SwitchedOn;
    }

private:
    bool SwitchedOn = false;
};
