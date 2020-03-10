#pragma once

#include "handlers.h"

#include <EEPROM.h>

template <typename TData>
struct TFlash : public Handler {
    const unsigned BaseAddress = 0;

    TFlash(unsigned base = 0)
    : Handler("Flash", 5000ms)
    , BaseAddress(base)
    {}

    void init() override {
        EEPROM.begin(sizeof(TData));
        EEPROM.get(BaseAddress, data);
    }

    void handle(std::chrono::milliseconds) override {
        EEPROM.put(BaseAddress, data);
        EEPROM.commit();  // does nothing if nothing changed
    }

    std::map<String, String> debug() const override {
        return {
            {"Data size", ToString(sizeof(TData))}
        };
    }

    TData data;
};

