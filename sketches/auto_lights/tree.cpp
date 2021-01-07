#include <unordered_set>

#include "lib/controller.h"
#include "lib/aleds.h"
#include "lib/flash.h"
#include "lib/light_sensor.h"
#include "lib/main.h"
#include "lib/ntp.h"

const char* NConfig::hostname = "tree";
TDefaultSetup dc;
TLightSensor light;

using namespace std;

constexpr unsigned NLeds = 50;

struct TTreeRenderer : public TAddressableLedStrip<> {
    float Step = .05;
    int MaxBright = 255;
    float progress[NLeds] = {};
    RgbColor src[NLeds];
    RgbColor target[NLeds];

    using TAddressableLedStrip<>::TAddressableLedStrip;

    void update() override {
        MaxBright = min(256, light.Value());
        MaxBright = max(MaxBright, 8);

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

    int rcolor(int low, int high, int bright) {
        if (low > high)
            low = 0;
        if (high > 255)
            high = 255;
        bright = bright * MaxBright / 255;
        return random(low, high) * bright / 255;
    }

    void gen_led(int i) {
        unsigned r, g, b, bright;
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
        src[i] = Strip.GetPixelColor(i);
        target[i] = RgbColor(r, g, b);
        progress[i] = 0;
    }

    void adjust(int led) {
        auto adjust1 = [](uint8_t& actual, uint8_t desired, float progress) {
            if (actual == desired)
                return;
            uint8_t diff = max(desired, actual) - min(desired, actual);
            if (actual > desired)
                actual = roundf(actual - diff * progress);
            else
                actual = roundf(actual + diff * progress);
        };
        RgbColor actual = src[led];
        adjust1(actual.R, target[led].R, progress[led]);
        adjust1(actual.G, target[led].G, progress[led]);
        adjust1(actual.B, target[led].B, progress[led]);
        if (progress[led] < 1)
            progress[led] += Step;
        Strip.SetPixelColor(led, actual);
    }

    std::map<String, String> debug() const override {
        return {
            {"MaxBright", String(MaxBright)}
        };
    }
};

TTreeRenderer renderer(NLeds, 30ms);
