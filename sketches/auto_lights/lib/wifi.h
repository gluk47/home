#pragma once
#include "handlers.h"
#include <ESP8266WiFi.h>

struct WifiClient : public Handler {
    WifiClient(const char* essid, const char* password)
    : Handler("wifi", 60s)
    , essid(essid)
    , password(password)
    {
        WiFi.mode(WIFI_STA);
    }

    void init() override {
        reconnect();
    }

    void handle(std::chrono::milliseconds when) override {
        reconnect();
        print_ip(when);

    }

    std::map<String, String> debug() const override {
        return std::map<String, String> {
            {"IP", ip_}
        };
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
            Serial.printf("\r\nConnection established!\r\n");
            ip_ = WiFi.localIP().toString();
        } else {
            Serial.println("\r\nConnection failed");
        }
    }

    const String& ip() const {
        return ip_;
    }

private:
    void print_ip(std::chrono::milliseconds now, std::chrono::milliseconds period = 5000ms) const {
        if (BoardTimeDifference(lastPrint, now) < period || ip().isEmpty())
            return;
        Serial.printf("IP: %s\n", ip().c_str());
        lastPrint = now;
    }

    const char* essid;
    const char* password;
    String ip_;
    mutable std::chrono::milliseconds lastPrint{0};
};
