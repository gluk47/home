#pragma once

#include "common.h"

#include <WString.h>

#include <chrono>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include <cassert>

class Handler {
public:
    Handler(String name, std::chrono::milliseconds period = 250ms);

    virtual void init() {};
    virtual std::map<String, String> debug() const = 0;
    virtual void handle(std::chrono::milliseconds now) = 0;

    void pinMode(int pin, int mode, bool allow_override = false) {
        if (PinModes().count(pin) and not allow_override)
            Serial.println("WARNING! Duplicate usage of the pin "_str + pin);

        PinModes()[pin] = mode;
        ::pinMode(pin, mode);
    }

    const std::chrono::milliseconds Period;
    const String Name;
    static std::map<int, int>& PinModes() {
        static std::map<int, int> modes;
        return modes;
    }
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
