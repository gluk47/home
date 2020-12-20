#pragma once

#include "common.h"

#include <WString.h>

#include <chrono>
#include <functional>
#include <map>
#include <vector>

#include <cassert>

class Handler {
public:
    Handler(String name, std::chrono::milliseconds period = 250ms);

    virtual void init() {};
    virtual std::map<String, String> debug() const = 0;
    virtual void handle(std::chrono::milliseconds now) = 0;

    void pinMode(uint8_t pin, uint8_t mode) {
            Serial.println("WARNING! Duplicate usage of the pin "_str + pin);

        PinUsages()[pin].emplace_back(Name);
        ::pinMode(pin, mode);
    }

    static std::vector<String> ReportPinConflicts() {
        std::vector<String> report;
        for (const auto& p : PinUsages()) {
            if (p.second.size() > 1)
                report.emplace_back(
                    "\033[33;40;1mWARNING!\033[0m Pin "_str + p.first
                    + " is used more that once, by: " + p.second
                );
        }
        return report;
    }

    const std::chrono::milliseconds Period;
    const String Name;

protected:
    static std::map<int, std::vector<String>>& PinUsages() {
        static std::map<int, std::vector<String>> users;
        return users;
    }
};

class TDebugHandler : public Handler {
public:
    TDebugHandler() : Handler("debug", 1min) {}
    std::map<String, String> debug() const override {
        String conflicts;
        // TODO format more nicely, do not hardcode it here
        for (const auto& c : PinUsages()) {
            conflicts += "    "_str + c.first + ": [\"" + ToString(c.second, R"(", ")") + "\"]\n";
        }
        return {
            {"Pins", "\n" + conflicts}
        };
    };
    void handle (std::chrono::milliseconds) override {
        Serial.println(ToString(ReportPinConflicts(), "\n"));
    }
    void init() override {
        handle(0ms);
    };
};

struct Handlers {
    static Handlers& the() {
        static Handlers h;
        return h;
    }

    static void delay() {
        ::delay(the().get_delay().count());
    }

    /**
     * The delay precision will be that of the most frequent handler.
     * E.g. if we have handlers for each 5 and 6 ms, we will actually run the first one once per 1×5 ms,
     * and the second one once per 2×5 ms, i.e. once per 10 ms instead of 6 ms requested.
     * As for now this is not an issue, but beware.
     */
    static void add(Handler& h) {
        auto& all = the();
        all.handlersByDelay[h.Period].backends.push_back(&h);
        all.handlers.push_back(&h);
        all.delay_ = all.handlersByDelay.begin()->first;
    }

    static void init() {
        const Handlers& h = the();
        for (const auto& handle : h.handlers) {
            Serial.printf("+++ init %s\n", handle->Name.c_str());
            handle->init();
            Serial.printf("−−− init %s\n", handle->Name.c_str());
        }
    }

    static void handle() {
        std::chrono::milliseconds now(millis());
        Handlers& h = the();
        for (auto& handle : h.handlersByDelay) {
            if (!handle.second.run(now))
                return;
        }
    }

    static String debug() {
        String resp;
        const Handlers& h = the();
        for (const auto& p : h.handlers) {
            const auto& values = p->debug();
            resp += p->Name + ":\n";
            resp += "  Period: " + ToString(p->Period) + "\n";
            for (const auto& v : values) {
                resp += "  " + v.first + ": " + v.second + "\n";
            }
            resp += "\n";
        }
        return resp;
    }

    std::chrono::milliseconds get_delay() const {
        return delay_;
    }

private:
    struct THandlersSamePeriod {
        std::vector<Handler*> backends;
        std::chrono::milliseconds last_execution = -1024ms;
        bool run(std::chrono::milliseconds now) {
            assert(backends.size() > 0);
            if (now > last_execution && now - last_execution < backends[0]->Period)
                return false;
            last_execution = now;
            for (const auto& b : backends) {
                b->handle(now);
            }
            return true;
        }
    };

    Handlers() = default;
    std::map<std::chrono::milliseconds, THandlersSamePeriod> handlersByDelay;
    std::vector<Handler*> handlers;
    std::chrono::milliseconds delay_ = 250ms;
};

Handler::Handler(String name, std::chrono::milliseconds period)
: Period(period)
, Name(name)
{
    Handlers::add(*this);
}
