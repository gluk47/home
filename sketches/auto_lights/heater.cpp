#include "lib/dht.h"
#include "lib/main.h"
#include "lib/pins.h"
#include "lib/switch.h"
#include "lib/controller.h"
#include "lib/flash.h"

TDefaultSetup dc;
TSwitch heater {Pins::Heater, THttpSensor::EHeater, "Heater"};
THttpController httpHeaterSwitch {dc.HttpSensor[heater.HttpID], "Http for heater"};
TDht dht(Pins::DHT, DHT11, .9f);
TTemperatureThresholdSensor sensor(dht, 34.f, 1.5f);


auto controller = MakeController(
    "Temperature trigger",
    std::tie(sensor, httpHeaterSwitch),
    std::tie(heater),
    5min
);


struct TData {
    float DesiredTemp = 24;
    float Hysteresis = 2;

    void apply_defaults() {
        TData defaults;
        if (isnan(DesiredTemp))
            DesiredTemp = defaults.DesiredTemp;
        if (isnan(Hysteresis))
            Hysteresis = defaults.Hysteresis;
    }
};

TFlash <TData> flash;

namespace {
    struct TSetup : Handler {
        TSetup()
        : Handler("setup", 360min)
        {}

        void init() override {
            dc.Http.on("/get", HTTP_GET, [&](ESP8266WebServer& server) {
                server.send(200, "text/plain", ""_str + dht.getTemperature() + "°C\n");
            }, "Get temperature");

            dc.Http.on("/set", HTTP_POST, [&](ESP8266WebServer& server) {
                const char* argname = "threshold";
                float threshold = server.arg(argname).toFloat();
                if (!server.hasArg(argname) || threshold < -10 || threshold > 30) {
                    server.send(400, "text/plain", "'threshold' out of range [-10, +30], ignored\n");
                    Serial.println("Set temperature threshold to: "_str + threshold);
                    sensor.DesiredTemp = flash.data.DesiredTemp = threshold;
                }

                argname = "hysteresis";
                float hysteresis = server.arg(argname).toFloat();
                if (!server.hasArg(argname) || hysteresis < .25 || hysteresis > 10) {
                    server.send(400, "text/plain", "'hysteresis' out of range [.25, 10], ignored\n");
                    Serial.println("Set temperature hysteresis to: "_str + hysteresis);
                    sensor.Hysteresis = flash.data.Hysteresis = hysteresis;
                }

            }, "threshold=24;hysteresis=2 : set target temperature and on-off margin\n  on = threshold − hysteresis, off = threshold + hysteresis.");

            flash.data.apply_defaults();
            sensor.DesiredTemp = flash.data.DesiredTemp;
            sensor.Hysteresis = flash.data.Hysteresis;
        };

        void handle(std::chrono::milliseconds) override {}
        std::map<String, String> debug() const override { return {}; }
    };
}

TSetup cfg;

