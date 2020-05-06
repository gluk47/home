#pragma once
#include "handlers.h"
#include <DHTesp.h>
#include <unordered_map>

struct TDht : public Handler {
    TDht(uint8_t pin, DHTesp::DHT_MODEL_t type, float inertion = 0.f)
    : Handler("DHT (raw)", 5s)
    , Inertion(inertion)
    {
        sensor.setup(pin, type);
    }

    void init() override {
        getTemperature();
    }

    void handle(std::chrono::milliseconds) override {
        getTemperature();
    }

    std::map<String, String> debug() const override {
        const float t = getTemperature();
        DHTesp& s = const_cast<DHTesp&>(sensor);
        const float h = s.getHumidity();
        static const std::unordered_map<unsigned, String> model = {
#define S(x) {DHTesp::x, #x}
            S(AUTO_DETECT),
            S(DHT11),
            S(DHT22),
            S(AM2302),  // Packaged DHT22
            S(RHT03)    // Equivalent to DHT22
#undef S
        };
        return {
            {"Humidity", String(h)},
            {"Temperature", String(t)},
            {"Sensor", model.at(s.getModel())},
            {"Status", s.getStatusString()}
        };
    }

    float getTemperature() const {
        return temperature;
    }

    float getTemperature() {
        const float newTemp = sensor.getTemperature();
        if (!isnan(newTemp)) {
            const float mean = isnan(temperature) ? newTemp : (Inertion * temperature + (1 - Inertion) * newTemp);
            if (mean != newTemp)
                Serial.printf("%.1f°C → %.1f°C\n", newTemp, mean);
            temperature = mean;
        }
        return temperature;
    }

    const float Inertion;

private:
    mutable float temperature = NAN, humidity = NAN;
    DHTesp sensor;
};

struct TTemperatureThresholdSensor : public Handler {
    const TDht& Dht;
    float DesiredTemp = 22;  //°C
    float Hysteresis = 2; //°C

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
