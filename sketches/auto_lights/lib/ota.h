#pragma once

#include "common.h"
#include "handlers.h"
#include <ArduinoOTA.h>
#include <unordered_map>

namespace std {
    template <>
    struct hash<ota_error_t> {
        unsigned operator()(ota_error_t e) const {
            return std::hash<unsigned>()(static_cast<unsigned>(e));
        }
    };
}

class Ota : public Handler {
public:
    Ota(const char* hostname, const char* pass)
    : Handler("OTA")
    , hostname(hostname)
    , pass(pass)
    {}

    void init() override {
        ArduinoOTA.setHostname(hostname);
        Serial.println("[OTA update] set up mDNS to "_str + hostname + ".local (install avahi/bonjour and ping that host)");
        ArduinoOTA.setPasswordHash(pass);

        ArduinoOTA.onStart([this]{ Serial.println("[OTA update] Started" ); this->perc = -1; });
        ArduinoOTA.onEnd  ([]{ Serial.println("[OTA update] Finished"); });
        ArduinoOTA.onProgress([this](unsigned progress, unsigned total) {
            int perc = progress / (total / 100);
            if (perc != this->perc) {
                Serial.printf("\r[OTA update] %u%%", perc);
                this->perc = perc;
                if (perc == 100)
                    Serial.println("");
            }
        });

        ArduinoOTA.onError([](ota_error_t error) {
            static const std::unordered_map<ota_error_t, const char*> msgs = {
                {OTA_AUTH_ERROR, "Auth Failed"},
                {OTA_BEGIN_ERROR, "Begin Failed"},
                {OTA_CONNECT_ERROR, "Connect Failed"},
                {OTA_RECEIVE_ERROR, "Receive Failed"},
                {OTA_END_ERROR, "End Failed"}
            };
            const auto& i = msgs.find(error);
            const char* msg = (i != msgs.end() ? i->second : "unknown");
            Serial.printf("[OTA update] Error %u: %s\n", error, msg);
        });
        ArduinoOTA.begin();

        Serial.println("[OTA update] Setup done");
    }

    void handle(std::chrono::milliseconds) override {
        ArduinoOTA.handle();
    }

    std::map<String, String> debug() const override {
        return {};
    }

    private:
        int perc = -1;
        const char* const hostname;
        const char* const pass;
};
