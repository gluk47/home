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

namespace NController {
    // TODO move these helpers as lambdas with ops folding as soon as C++17 is fully supported
    struct TAllOn {
        bool operator()() const { return true; }

        template <typename TSensor, typename... TSensors>
        bool operator()(TSensor&& s, TSensors&&... sensors) const {
            return s.ShouldSwitchOn() and operator()(sensors...);
        }
    };

    struct TIsChangeNeeded {
        bool Desired DEFAULT_VALUE_FOR_AGGREGATE_TYPE(false);

        bool operator()() const { return true; }

        template <typename TSwitch, typename... TSwitches>
        bool operator()(TSwitch&& s, TSwitches&&... switches) const {
            return s.TurnedOn() != Desired and operator()(switches...);
        }
    };

    struct TTurnOn {
        bool Desired DEFAULT_VALUE_FOR_AGGREGATE_TYPE(false);

        void operator()() const {}

        template <typename TSwitch, typename... TSwitches>
        void operator()(TSwitch& s, TSwitches&&... ss) const {
            s.TurnOn(Desired);
            operator()(ss...);
        }
    };
}

template <typename TLightSensors, typename TSwitches>
class TController : public IController {
public:
    std::chrono::milliseconds MinimalDelay;

    TController(const TLightSensors& sensors, TSwitches& switches, std::chrono::milliseconds minimalDelay = 1000ms)
    : MinimalDelay(minimalDelay)
    , Sensors(sensors)
    , Switches(switches) {
    }

    void Update(std::chrono::milliseconds now) override {
        if (BoardTimeDifference(LastSwitch, now) < MinimalDelay)
            return;

        using namespace NController;
        const bool desired = std::apply(TAllOn(), Sensors);
        const bool actual = std::apply(TIsChangeNeeded{desired}, Switches);

        if (desired != actual) {
            LastSwitch = now;
            std::apply(TTurnOn{desired}, Switches);
        }
    }

private:
    std::chrono::milliseconds LastSwitch{0};
    const TLightSensors& Sensors;
    TSwitches& Switches;
};

// TODO use deduction guides as soon as xtensa toolchain suports them
template <typename TLightSensors, typename TSwitches>
TController<TLightSensors, TSwitches> MakeController(TLightSensors&& sensors, TSwitches&& switches) {
    return TController<TLightSensors, TSwitches>(std::forward<const TLightSensors&>(sensors), std::forward<TSwitches&>(switches));
}
