#pragma once
#include "handlers.h"
#include <ESP8266WiFi.h>

struct WifiClient {
    WifiClient(const char* essid, const char* password)
    : essid(essid)
    , password(password)
    {
        WiFi.mode(WIFI_STA);
        Handlers::addInit([this] { reconnect(); });
        Handlers::add([this](int){ reconnect(); });
    }

    void reconnect() {
        if (WiFi.status() == WL_CONNECTED)
            return;

        Serial.printf("Connecting to %s...\n", essid);
        WiFi.begin(essid, password);

        for (int i = 0; WiFi.status() != WL_CONNECTED; ++i) {
            delay(1000);
            Serial.printf("%d ", i);
        }

        Serial.printf("\nConnection established!\nIP address:\t");
        Serial.println(WiFi.localIP());
    }

private:
    const char* essid;
    const char* password;
};
