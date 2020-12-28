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

template <typename Feature = NeoGrbFeature, typename Method = NeoEsp8266DmaWs2812xMethod>
class TAddressableLedStrip : public Handler {
public:
    TAddressableLedStrip(unsigned led_count, std::chrono::milliseconds period = 30ms)
    : Handler("NeoPixel", period)
    , Strip(led_count)
    {
        // only RX aka D9 aka GPIO3 is supported by NeoPixelBus at the moment.
        // FastLEDs support more pins but causes occasional glittering, rather frequent
        constexpr unsigned D9 = 3;
        pinMode(D9, OUTPUT);  // indicate our intentions to be able to detect pin conflicts

        randomSeed((analogRead(0) << 1) ^ (analogRead(0) << 3) ^ (analogRead(0) << 5) ^ (analogRead(0) << 7));
    }

    void init() override {
        Strip.Begin();
    }

    virtual void update() = 0;

    std::map<String, String> debug() const override { return {}; }
    void handle (std::chrono::milliseconds) final override {
        update();
        Strip.Show();
    }

protected:
    NeoPixelBus<Feature, Method> Strip;
};
