#include "lib/dht.h"
#include "lib/main.h"
#include "lib/pins.h"
#include "lib/switch.h"
#include "lib/controller.h"
#include "lib/flash.h"

TDefaultSetup dc;
TSwitch heater {Pins::Heater, THttpSensor::EHeater, "Heater"};
THttpController httpHeaterSwitch {dc.HttpSensor[heater.HttpID], "Http for heater"};
TDht dht(Pins::DHT, DHT11);
TTemperatureThresholdSensor sensor(dht, 34.f, .5f);


auto controller = MakeController(
    "Temperature trigger",
    std::tie(sensor, httpHeaterSwitch),
    std::tie(heater),
    100ms
);


struct TData {
    float DesiredTemp = 24;
};

TFlash <TData> flash;

namespace {
    struct TSetup : Handler {
        TSetup()
        : Handler("setup", 60000ms)
        {}

        void init() override {
            dc.Http.on("/get", HTTP_GET, [&](ESP8266WebServer& server) {
                server.send(200, "text/plain", ""_str + dht.getTemperature() + "°C\n");
            }, "Get temperature");

            dc.Http.on("/set", HTTP_POST, [&](ESP8266WebServer& server) {
                const char* argname = "threshold";
                float threshold = server.arg(argname).toFloat();
                if (!server.hasArg(argname) || threshold < -10 || threshold > 30) {
                    server.send(400, "text/plain", "'threshold' out of range [-10, +30] or missing\n");
                    return;
                }
                Serial.println("Set temperature threshold to: "_str + threshold);
                sensor.DesiredTemp = flash.data.DesiredTemp = threshold;
            }, "arg: float threshold. Set, when it is dark (see also 'get').");

            sensor.DesiredTemp = flash.data.DesiredTemp;
        };

        void handle(std::chrono::milliseconds) override {}
        std::map<String, String> debug() const override { return {}; }
    };
}

TSetup cfg;

