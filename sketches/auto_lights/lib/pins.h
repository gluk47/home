#pragma once

#include <Arduino.h>

namespace Pins {
    constexpr int
        No = -1,
        Indoor[] = { D5, D2 },
        Outdoor = D1,
        DHT = D4,
        Heater = D1;
};
