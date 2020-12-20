#include <unordered_set>

#include "lib/controller.h"
#include "lib/aleds.h"
#include "lib/flash.h"
#include "lib/main.h"
#include "lib/ntp.h"

const char* NConfig::hostname = "tree";
TDefaultSetup dc;

using namespace std;

constexpr unsigned NLeds = 50;

struct TTreeRenderer {
    const unsigned Step = 1;
    int MaxBright = 255;
    NeoPixelBus<NeoGrbFeature, NeoEsp8266DmaWs2812xMethod>& Strip;

    TTreeRenderer(NeoPixelBus<NeoGrbFeature, NeoEsp8266DmaWs2812xMethod>& strip)
    : Strip(strip) {
        randomSeed((analogRead(0) << 1) ^ (analogRead(0) << 3) ^ (analogRead(0) << 5) ^ (analogRead(0) << 7));
        strip.GetPixelColor(0);
    }

    void operator()() {
        for (int i = 0; i < 2; i++)
            gen_led(random(NLeds));

        for (unsigned i = 0; i < NLeds; i++)
            adjust(i);
    }

    unordered_set<int> branches = {
        6, 7, 8, 12, 13, 17, 18, 19, 22, 23, 27, 28, 31, 32, 33, 37, 38, 42, 43, 44
    };
    unordered_set<int> trunk = {
        3, 4, 5, 45, 46, 47
    };

    RgbColor target[NLeds];

    int rcolor(int low, int high, int bright) {
        if (low > high)
            low = 0;
        if (high > 255)
            high = 255;
        bright = min(bright, MaxBright);
        return random(low, high) * bright / 255;
    }

    void gen_led(int i) {
        int r, g, b, bright;
        if (branches.count(i)) {
            bright = random(0, 256);
            r = rcolor(0, 32, bright);
            g = rcolor(200, 255, bright);
            b = rcolor(0, 32, bright);
        } else if (trunk.count(i)) {
            bright = random(0, 256);
            r = rcolor(60, 80, 255);
            g = rcolor(20, 30, 255);
            b = rcolor(0, 10, 255);
        } else {
            bright = random(0, 256);
            r = rcolor(200, 255, bright);
            g = rcolor(0, 64, bright);
            b = rcolor(0, 32, bright);
        }
        target[i] = RgbColor(r, g, b);
    }

    void adjust(int led) {
        auto adjust1 = [this](uint8_t& actual, uint8_t desired) {
            if (actual == desired)
                return;
            uint8_t diff = max(desired, actual) - min(desired, actual);
            if (diff <= Step)
                actual = desired;
            else if (actual > desired)
                actual -= Step;
            else
                actual += Step;
        };
        auto actual = Strip.GetPixelColor(led);
        adjust1(actual.R, target[led].R);
        adjust1(actual.G, target[led].G);
        adjust1(actual.B, target[led].B);
        Strip.SetPixelColor(led, actual);
    }
};

TAddressableLedStrip<TTreeRenderer> strip(NLeds, 10ms);
