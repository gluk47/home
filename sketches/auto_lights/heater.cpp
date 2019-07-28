#include "dht.h"
#include "main.h"
#include "pins.h"
#include "switch.h"

const TSwitch heater {Pins::Heater};
TDht dht(Pins::DHT, DHT11);
