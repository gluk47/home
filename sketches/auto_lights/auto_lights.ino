#include "handlers.h"
#include "http.h"
#include "http_interface.h"
#include "lcd.h"
#include "light_sensor.h"
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

TLightSensor LightSensor;
THttpSensor& HttpSensor = THttpSensor::the();
THttpInterface Http(HttpSensor, LightSensor, 80);
WifiClient wifi(NConfig::essid, NConfig::wifi_password);
Ota ota(NConfig::hostname, NConfig::ota_pass_md5);
// TLcd lcd(wifi);
#define PWM_INDOOR
const TSwitch Switches[] = {
#ifndef PWM_INDOOR
  {Pins::Indoor[0], HttpSensor[THttpSensor::EIndoor], &LightSensor, THttpSensor::EIndoor, "Indoor"},
  {Pins::Indoor[1], HttpSensor[THttpSensor::EIndoor], &LightSensor, THttpSensor::EIndoor, "Indoor"},
#endif
  {Pins::OutdoorPass, HttpSensor[THttpSensor::EOutdoorPass], &LightSensor, THttpSensor::EOutdoorPass, "Дом у сарая"},
  {Pins::OutdoorDoor, HttpSensor[THttpSensor::EOutdoorDoor], &LightSensor, THttpSensor::EOutdoorDoor, "Дом у двери"}
};

TPwm Pwms[] = {
#ifdef PWM_INDOOR
  {Pins::Indoor[0], {1, 1, 0, 0, 1, 0}, HttpSensor[THttpSensor::EIndoor], LightSensor, THttpSensor::EIndoor, "Indoor"},
  {Pins::Indoor[1], {0, 0, 1, 1, 0, 1}, HttpSensor[THttpSensor::EIndoor], LightSensor, THttpSensor::EIndoor, "Indoor"}
#endif
};

void setup() {
    Serial.begin(115200);
    delay(50);
    Serial.println('\n');
    Serial.printf("Set delay to %d\r\n", Handlers::the().get_delay());
    Handlers::init();
    Serial.println("Setup done\n");
}

void loop() {
    Handlers::handle();
    Handlers::delay();
}
