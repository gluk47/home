#include "controller.h"
#include "dht.h"
#include "handlers.h"
#include "http.h"
#include "http_interface.h"
#include "lcd.h"
#include "light_sensor.h"
#include "main.h"
#include "ota.h"
#include "pins.h"
#include "pwm.h"
#include "switch.h"
#include "wifi.h"

#include "common.h"
#include "config.h"

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

TDefaultSetup dc;

TLightSensor LightSensor;
TNightLightController LightControl{LightSensor};
TSwitch DoorLight(Pins::Outdoor, THttpSensor::EOutdoor, "Внешний свет");

auto FrontLights = MakeController(
    std::make_tuple(DoorLight),
    std::make_tuple(LightControl, THttpController{dc.HttpSensor[THttpSensor::EOutdoor]})
);

#define PWM_INDOOR
const TSwitch Switches[] = {
    #ifndef PWM_INDOOR
    {Pins::Indoor[0], dc.HttpSensor[THttpSensor::EIndoor], "Indoor"},
    {Pins::Indoor[1], dc.HttpSensor[THttpSensor::EIndoor], "Indoor"},
    #endif
    {Pins::Outdoor, dc.HttpSensor[THttpSensor::EOutdoor], "Внешний свет"},
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
        }
    };
}

TSetup cfg;
