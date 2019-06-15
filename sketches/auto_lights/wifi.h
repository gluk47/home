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

        Serial.printf("Connecting to %s... ", essid);
        WiFi.begin(essid, password);

        for (int i = 0; WiFi.status() != WL_CONNECTED; ++i) {
            delay(1000);
            Serial.printf("%d ", i);
        }

        Serial.printf("\r\nConnection established!\r\nIP address:\t");
        Serial.println(ip_ = WiFi.localIP().toString());
    }

    const String& ip() const {
        return ip_;
    }

private:
    const char* essid;
    const char* password;
    String ip_;
};
