#include "lib/controller.h"
#include "lib/dht.h"
#include "lib/handlers.h"
#include "lib/http.h"
#include "lib/http_interface.h"
#include "lib/lcd.h"
#include "lib/light_sensor.h"
#include "lib/main.h"
#include "lib/ota.h"
#include "lib/pins.h"
#include "lib/pwm.h"
#include "lib/switch.h"
#include "lib/wifi.h"

#include "lib/common.h"
#include "lib/config.h"

#include <cstdlib>
#include <vector>

/*
 * An ESP8266 project:
 * Inputs:
 * - Http handles
 * - Light sensor
 *
 * Outputs:
 * - 2 street lights
 * - 2 channels of indoor lights
 *
 * If enabled by http and it is dark,
 * switch on all lights.
 * Lights can be turned off independently.
 */

const char* NConfig::hostname = "lights";
TDefaultSetup dc;

TLightSensor LightSensor;
TNightLightSensor LightControl{LightSensor, "Night light sensor"};
TSwitch DoorLight(Pins::Outdoor, THttpSensor::EOutdoor, "Внешний свет");
THttpController httpLightSwitch {dc.HttpSensor[THttpSensor::EOutdoor], "Http Outdoor light"};

auto FrontLights = MakeController(
    "Front lights",
    std::tie(LightControl, httpLightSwitch),
    std::tie(DoorLight)
);

#define PWM_INDOOR
const TSwitch Switches[] = {
    #ifndef PWM_INDOOR
    {Pins::Indoor[0], dc.HttpSensor[THttpSensor::EIndoor], "Indoor"},
    {Pins::Indoor[1], dc.HttpSensor[THttpSensor::EIndoor], "Indoor"},
    #endif
    DoorLight,
};

TPwm Pwms[] = {
    #ifdef PWM_INDOOR
    {Pins::Indoor[0], {1, 1, 0, 0, 1, 0}, dc.HttpSensor[THttpSensor::EIndoor], LightSensor, THttpSensor::EIndoor, "Indoor"},
    {Pins::Indoor[1], {0, 0, 1, 1, 0, 1}, dc.HttpSensor[THttpSensor::EIndoor], LightSensor, THttpSensor::EIndoor, "Indoor"}
    #endif
};

// TDht dht(Pins::DHT);
// TLcd lcd(wifi);

namespace {
    struct TSetup {
        TSetup () {
            dc.Http.SetLightSensor(&LightSensor);

            dc.Http.on("/get", HTTP_GET, [&](ESP8266WebServer& server) {
                server.send(200, "text/plain", ToString(LightSensor.Value()) + "\n");
            }, "Get current brightness level (see also 'set').");

            dc.Http.on("/set", HTTP_POST, [&](ESP8266WebServer& server) {
                int threshold = server.arg("threshold").toInt();
                const char* argname = "threshold";
                if (!server.hasArg(argname) || threshold < 0 || threshold > 1200) {
                    server.send(400, "text/plain", "out of range [0, 1200]\n");
                    return;
                }
                Serial.println("set_darkness: "_str + threshold);
                LightSensor.Darkness = threshold;
            }, "arg: int threshold. Set, when it is dark (see also 'get').");
        }
    };
}

TSetup cfg;
