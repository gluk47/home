#pragma once
// These are sensors needed by all installations

#include "config.h"
#include "handlers.h"
#include "http.h"
#include "http_interface.h"
#include "ota.h"
#include "switch.h"
#include "wifi.h"

THttpSensor& HttpSensor = THttpSensor::the();
THttpInterface Http(HttpSensor, 80);
WifiClient wifi(NConfig::essid, NConfig::wifi_password);
Ota ota(NConfig::hostname, NConfig::ota_pass_md5);

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
