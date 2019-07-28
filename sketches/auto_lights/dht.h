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
                {"FeelsLike", sensor.computeHeatIndex(t, h)},
                {"Humidity", String(h)},
                {"Temperature", String(t)}
            };
        });
    }

    DHT sensor;
};
