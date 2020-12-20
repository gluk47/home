/*
 * This is an addressable leds library adapter
 */
#pragma once
#ifdef IN_KDEVELOP_PARSER
#   define ARDUINO_ARCH_ESP8266
#endif
#include <NeoPixelBus.h>

#include "common.h"
#include "switch.h"

template <typename TUpdater, typename Feature = NeoGrbFeature, typename Method = NeoEsp8266DmaWs2812xMethod>
class TAddessableLedStrip : public Handler {
public:
    TAddessableLedStrip(unsigned led_count, std::chrono::milliseconds period = 30ms)
    : Handler("NeoPixel", period)
    , strip(led_count)
    {
        // only RX aka D9 is supported by NeoPixelBus at the moment.
        // FastLEDs support more pins but causes occasional glittering, rather frequent
        pinMode(D9, OUTPUT);  // indicate our intentions to be able to detect pin conflicts
    }

    void init() override {
        strip.Begin();
    }

    std::map<String, String> debug() const override { return {} }
    void handle (std::chrono::milliseconds now) override {
        updater(strip);
        strip.show();
    }

private:
    TUpdater& updater;
    NeoPixelBus<Feature, Method> strip;
};
