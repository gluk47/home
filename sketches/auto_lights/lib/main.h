#pragma once
// These are sensors needed by all installations

#include "config.h"
#include "handlers.h"
#include "http.h"
#include "http_interface.h"
#include "ota.h"
#include "switch.h"
#include "wifi.h"

#include <chrono>

struct TDefaultSetup {
    THttpSensor& HttpSensor = THttpSensor::the();
    THttpInterface Http{HttpSensor, 80};
    WifiClient wifi{NConfig::essid, NConfig::wifi_password};
    Ota ota{NConfig::hostname, NConfig::ota_pass_md5};
    TDebugHandler debug;
};

void setup() {
    Serial.begin(115200);
    while (!Serial)
        delay(1);
    Serial.println('\n');
    Serial.printf(
        "Set delay to %d ms\r\n",
        static_cast<int>(std::chrono::milliseconds(Handlers::the().get_delay()).count())
    );
    Handlers::init();
    Serial.println("Setup done\n");
}

void loop() {
    Handlers::handle();
    Handlers::delay();
}
