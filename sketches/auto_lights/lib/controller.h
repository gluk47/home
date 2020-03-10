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
        bool operator()(const TSensor& s, const TSensors&... sensors) const {
            return s.ShouldSwitchOn() and operator()(sensors...);
        }
    };

    struct TAllSwitchesOn {
        bool operator()() const { return true; }

        template <typename TSwitch, typename... TSwitches>
        bool operator()(const TSwitch& s, const TSwitches&... sensors) const {
            return s.TurnedOn() and operator()(sensors...);
        }
    };

    struct TIsChangeNeeded {
        bool Desired DEFAULT_VALUE_FOR_AGGREGATE_TYPE(false);

        bool operator()() const { return true; }

        template <typename TSwitch, typename... TSwitches>
        bool operator()(const TSwitch& s, const TSwitches&... switches) const {
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

    struct GetNames {
        String operator()() const {
            return String();
        }

        template <typename THandler>
        String operator()(const THandler& h) {
            return "\"" + Get(h) + "\"";
        }

        template <typename THandler, typename... THandlers>
        String operator()(const THandler& h, const THandlers&... hh) {
            return "\"" + Get(h) + "\", " + operator()(hh...);
        }

    private:
        const String Null = "<nullptr>";

        template <typename THandler>
        const String& Get(const THandler& h) {
            return &h ? h.Name : Null;
        }
    };
}

template <typename TSensors, typename TSwitches>
class TController : public Handler {
public:
    std::chrono::milliseconds MinimalDelay;

    TController(const String& name, const TSensors&& sensors, TSwitches&& switches, std::chrono::milliseconds minimalDelay = 1000ms)
    : Handler(name, minimalDelay)
    , Sensors(sensors)
    , Switches(switches) {
    }

    void init() override {
        using namespace NController;
        Serial.printf("  controller %s:\n", Name.c_str());
        const String& sensors = std::apply(GetNames(), Sensors);
        const String& switches = std::apply(GetNames(), Switches);
        Serial.printf("    sensors: %s\n    switches: %s\n---\n",
            sensors.c_str(),
            switches.c_str()
        );
    }

    void handle(std::chrono::milliseconds now) override {
        if (BoardTimeDifference(LastSwitch, now) < Period)
            return;

        using namespace NController;
        const bool desired = std::apply(TAllOn(), Sensors);
        if (std::apply(TIsChangeNeeded{desired}, Switches)) {
            LastSwitch = now;
            std::apply(TTurnOn{desired}, Switches);
        }
    }

    std::map<String, String> debug() const override {
        using namespace NController;
        return {
            {"Last switch", ToString(BoardTimeDifference(LastSwitch, std::chrono::milliseconds(::millis()))) + " ago"_str},
            {"Turned on", YesNo(std::apply(TAllSwitchesOn(), Switches))},
            {"Desired state", OnOff(std::apply(TAllOn(), Sensors))},
            {"Sensors", "[ " + std::apply(GetNames(), Sensors) + " ]"},
            {"Switches", "[ " + std::apply(GetNames(), Switches) + " ]"},
        };
    }

private:
    std::chrono::milliseconds LastSwitch{0};
    const TSensors Sensors;
    TSwitches Switches;
};

// TODO use deduction guides as soon as xtensa toolchain supports them
template <typename TSensors, typename TSwitches>
TController<TSensors, TSwitches> MakeController(
    const String& name,
    const TSensors& sensors,
    TSwitches&& switches,
    std::chrono::milliseconds minimalDelay = 1000ms
) {
    return TController<TSensors, TSwitches>(name, std::move(sensors), std::forward<TSwitches&&>(switches), minimalDelay);
}
