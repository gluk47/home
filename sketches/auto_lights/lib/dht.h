#pragma once
#include "handlers.h"
#include <DHT.h>

struct TDht {
    TDht(uint8_t pin = D4, uint8_t type = DHT22)
    : sensor(pin, type) {
        Handlers::addDebug("DHT", [this]{
            const float t = sensor.readTemperature();
            const float h = sensor.readHumidity();
            return std::map<String, String>{
                {"FeelsLike", String(sensor.computeHeatIndex(t, h))},
                {"Humidity", String(h)},
                {"Temperature", String(t)}
            };
        });
    }

    DHT sensor;
};

struct TTemperatureController {
    TDht& Dht;
    float DesiredTemp = 22;
    float Hysteresis = 2; //degrees

    TTemperatureController(TDht& dht, float desiredTemp = 22.f, float hysteresis = 2.f)
    : Dht(dht), DesiredTemp(desiredTemp), Hysteresis(hysteresis)
    {
        Handlers::add([this](std::chrono::milliseconds){
            float margin = SwitchedOn ? +Hysteresis : -Hysteresis;
            SwitchedOn = Dht.sensor.readTemperature() > DesiredTemp + margin;
        });

        Handlers::addDebug("LightSensor", [this]{ return
            std::map<String, String>{
                {"Temperature", ToString(Dht.sensor.readTemperature())},
                {"DesiredTemp", ToString(DesiredTemp)},
                {"Hysteresis", ToString(Hysteresis)}
            };
        });
    }

    bool ShouldSwitchOn() const {
        return SwitchedOn;
    }

private:
    bool SwitchedOn = false;
};
