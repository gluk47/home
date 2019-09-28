#include "lib/dht.h"
#include "lib/main.h"
#include "lib/pins.h"
#include "lib/switch.h"
#include "lib/controller.h"

const TSwitch heater {Pins::Heater, THttpSensor::EHeater, "Heater"};
TDht dht(Pins::DHT, DHT11);
auto controller = MakeController(
    std::make_tuple(TTemperatureController(dht, 22.f, 2.f)),
    std::make_tuple(heater)
);
