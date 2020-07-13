#include "lib/controller.h"
#include "lib/dht.h"
#include "lib/flash.h"
#include "lib/main.h"
#include "lib/ntp.h"
#include "lib/pins.h"
#include "lib/switch.h"

const char* NConfig::hostname = "heater";
TDefaultSetup dc;
TSwitch heater {Pins::Heater, THttpSensor::EHeater, "Heater"};
THttpController httpHeaterSwitch {dc.HttpSensor[heater.HttpID], "Http for heater"};
TDht dht(Pins::DHT, DHTesp::DHT11, .9f);
TTemperatureThresholdSensor sensor(dht, 34.f, 1.5f);
NTP ntp(3 * 3600, NConfig::ntp_server);


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
                server.send(200, "text/plain", ""_str + dht.getTemperature() + "\n");
            }, "Get temperature");

            dc.Http.on("/set", HTTP_POST, [&](ESP8266WebServer& server) {
                String ok;
                String err;
                auto apply_arg = [&](const String& argname, float min, float max, const std::function<void(float)>& f) {
                    if (!server.hasArg(argname))
                        return;
                    float value = server.arg(argname).toFloat();
                    if (value < min) {
                        err += argname + " is less than " + String(min) + ", ignored\n";
                        return;
                    }
                    if (value > max) {
                        err += argname + " is greater than " + String(max) + ", ignored\n";
                        return;
                    }
                    f(value);
                    ok += argname + " set to " + String(value) + "\n";
                };
                apply_arg("threshold", -10, 30, [&](float value){
                    sensor.DesiredTemp = flash.data.DesiredTemp = value;
                });
                apply_arg("hysteresis", .2, 5, [&](float value){
                    sensor.Hysteresis = flash.data.Hysteresis = value;
                });
                int code = 400;
                if (ok) {
                    ok += "\n";
                    code = 200;
                } else if (!err) {
                    err = "No valid parameters found";
                }
                ok += err;
                server.send(code, "text/plain", ok);
                Serial.println(ok);
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

