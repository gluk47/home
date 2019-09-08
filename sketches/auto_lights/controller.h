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
    // TODO move these helpers as lambdas as soon as C++17 is fully supported
    struct TAllOn {
        template <typename... TSensors>
        bool operator()(TSensors&&... sensors) {
            return TurnedOn(sensors...);
        }

    private:
        template <typename T>
        bool TurnedOn(const T& s) {
            return s.TurnedOn();
        }

        template <typename S, typename... Ss>
        bool TurnedOn(const S& s, const Ss&... ss) {
            return s.TurnedOn() and TurnedOn(ss...);
        }
    };

    struct TIsChangeNeeded {
        bool Desired = false;

        TIsChangeNeeded(bool desired) : Desired(desired){}

        template <typename... T>
        bool operator()(T&&...) {
            return true;
        }
    };

    struct TTurnOn {
        bool Desired = false;

        TTurnOn(bool desired) : Desired(desired){}

        template <typename... T>
        void operator()(T&&...) {
            return;
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
            std::apply(TTurnOn{desired}, Sensors);
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
