#pragma once
#include <array>

struct THttpSensor {
    enum light_t {
        EInvalid,
        EIndoor,
        EOutdoor,
        EHeater,
        EUnused
    };
    std::array<bool, EUnused + 1> enabled;

    static THttpSensor& the() {
        static THttpSensor sensor;
        return sensor;
    }

    const bool& operator[] (int i) const {
        return valid(i) ? enabled[i] : enabled[EUnused];
    }

    bool& operator[] (int i) {
        return const_cast<bool&>(const_cast<const THttpSensor&>(*this)[i]);
    }

    void toggle(int i, bool state) {
        if (not valid(i))
            return;
        enabled[i] = state;
    }

    static bool valid(int i) {
        return EInvalid < i and i < EUnused;
    }

private:
    THttpSensor() {
        enabled.fill(true);
    }
};

struct THttpController {
    bool ShouldSwitchOn() const {
        return sensor;
    };

    const bool& sensor;
    const String Name;
};
