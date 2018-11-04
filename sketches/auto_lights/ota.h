#pragma once

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

class Ota {
public:
    Ota(const char* hostname, const char* pass) {
        Handlers::addInit([hostname, pass]{
            ArduinoOTA.setHostname(hostname);
            ArduinoOTA.setPasswordHash(pass);

            ArduinoOTA.onStart([]{ Serial.println("[OTA update] Started" ); });
            ArduinoOTA.onEnd  ([]{ Serial.println("[OTA update] Finished"); });
            ArduinoOTA.onProgress([](unsigned progress, unsigned total) {
                Serial.printf("[OTA update] %u%%\n", (progress / (total / 100)));
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
        });

        Handlers::add([this](int){
            ArduinoOTA.handle();
        });
    }
};
