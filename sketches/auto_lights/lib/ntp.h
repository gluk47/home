#pragma once
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "handlers.h"

class NTP : public NTPClient, Handler {
public:
    static WiFiUDP& UDP() {
        static WiFiUDP udp;
        return udp;
    }

    NTP(unsigned offset, const char* server = "pool.ntp.org")
    : NTPClient(UDP(), server, offset)
    , Handler("NTP")
    {}

    void init() override {
        NTPClient::begin();
    }

    void handle (std::chrono::milliseconds) override {
        NTPClient::update();
    }

    std::map<String, String> debug() const override {
        return {
            {"Time", NTPClient::getFormattedTime()},
        };
    }
};
