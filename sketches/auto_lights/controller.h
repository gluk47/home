#pragma once

#include "common.h"
#include "handlers.h"

#include <chrono>
#include <tuple>
#include <utility>

// A controller that decides whether a switch should be turned on or off

class IController {
public:
    IController() {
        Handlers::add([this](std::chrono::milliseconds now){
            Update(now);
        });
    }

    virtual void Update(std::chrono::milliseconds when) = 0;
};

template <typename TLightSensors, typename TSwitches>
class TController : public IController {
public:
    std::chrono::milliseconds MinimalDelay;

    TController(const TLightSensors& sensors, TSwitches& switches, std::chrono::milliseconds minimalDelay = 1000ms)
    : Sensors(sensors)
    , Switches(switches)
    , MinimalDelay(minimalDelay) {
    }

    void Update(std::chrono::milliseconds now) override {
        if (BoardTimeDifference(LastSwitch, now) < MinimalDelay)
            return;

        const bool desired = std::apply(AllSensorsAreOn, Sensors);
        const bool actual = std::apply(ChangeNeeded, std::tuple_cat(desired, Switches));
        if (desired != actual) {
            LastSwitch = now;
            std::apply(TurnOn, std::tuple_cat(desired, Sensors));
        }
    }

private:
    template <typename... Sensors>
    static bool AllSensorsAreOn(Sensors&&... s) {
        return (s.ShouldSwitchOn() && ...);
    }

    template <typename... Switches>
    static bool ChangeNeeded(bool desired, Switches&&... s) {
        return ((s.TurnedOn() != desired) && ...);
    }

    template <typename... Switches>
    static void TurnOn(bool state, Switches&&... s) {
        (s.TurnOn(state), ...);
    }

    std::chrono::milliseconds LastSwitch{0};
    const TLightSensors& Sensors;
    TSwitches& Switches;
};
