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
        Handlers::add([this](std::chrono::milliseconds){ reconnect(); });
    }

    void reconnect() {
        if (WiFi.status() == WL_CONNECTED)
            return;

        Serial.printf("Connecting to %s... ", essid);
        WiFi.begin(essid, password);

        for (int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; ++i) {
            delay(1000);
            Serial.printf("%d ", i);
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.printf("\r\nConnection established!\r\nIP address:\t");
            Serial.println(ip_ = WiFi.localIP().toString());
        } else {
            Serial.println("\r\nConnection failed");
        }
    }

    const String& ip() const {
        return ip_;
    }

private:
    const char* essid;
    const char* password;
    String ip_;
};
