#pragma once

#include "common.h"
#include "handlers.h"

#include <chrono>
#include <tuple>
#include <utility>

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
class TController : public Handler {
public:
    std::chrono::milliseconds MinimalDelay;

    TController(const String& name, const TLightSensors& sensors, TSwitches& switches, std::chrono::milliseconds minimalDelay = 1000ms)
    : Handler(name, minimalDelay)
    , Sensors(sensors)
    , Switches(switches) {
    }

    void handle(std::chrono::milliseconds now) override {
        if (BoardTimeDifference(LastSwitch, now) < Period)
            return;

        using namespace NController;
        const bool desired = std::apply(TAllOn(), Sensors);
        const bool actual = std::apply(TIsChangeNeeded{desired}, Switches);

        if (desired != actual) {
            LastSwitch = now;
            std::apply(TTurnOn{desired}, Switches);
        }
    }

    std::map<String, String> debug() const override {
        return {
            {"LastSwitch", ToString(LastSwitch)}
        };
    }

private:
    std::chrono::milliseconds LastSwitch{0};
    const TLightSensors& Sensors;
    TSwitches& Switches;
};

// TODO use deduction guides as soon as xtensa toolchain suports them
template <typename TLightSensors, typename TSwitches>
TController<TLightSensors, TSwitches> MakeController(const String& name, TLightSensors&& sensors, TSwitches&& switches) {
    return TController<TLightSensors, TSwitches>(name, std::forward<const TLightSensors&>(sensors), std::forward<TSwitches&>(switches));
}
