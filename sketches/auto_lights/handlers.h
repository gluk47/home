#pragma once

#include "common.h"

#include <WString.h>

#include <chrono>
#include <functional>
#include <map>
#include <vector>

struct Handlers {
    static Handlers& the() {
        static Handlers h;
        return h;
    }

    static void delay() {
        ::delay(the().get_delay().count());
    }

    static void add(std::function<void(std::chrono::milliseconds)>&& f, std::chrono::milliseconds period = 250ms) {
        auto& h = the();
        h.handlers[period].functions.push_back(std::move(f));
        h.delay_ = h.handlers.begin()->first;
    }

    static void addInit(std::function<void()>&& f) {
        the().initters.push_back(std::move(f));
    }

    static void addDebug(String name, std::function<std::map<String,String>()>&& f) {
        the().debugHandlers.push_back({std::move(name), std::move(f)});
    }

    static void init() {
        const Handlers& h = the();
        for (const auto& handle : h.initters)
            handle();
    }

    static void handle() {
        std::chrono::milliseconds now(millis());
        const Handlers& h = the();
        for (const auto& handle : h.handlers) {
            std::chrono::milliseconds period = handle.first;
            if (!handle.second.run(now, period))
                return;
        }
    }

    static String debug() {
        String resp;
        const Handlers& h = the();
        for (const auto& p : h.debugHandlers) {
            const auto& values = p.second();
            resp += p.first + ":\n";
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
        std::vector<std::function<void(std::chrono::milliseconds)>> functions;
        mutable std::chrono::milliseconds last_execution = -1024ms;
        bool run(std::chrono::milliseconds now, std::chrono::milliseconds period) const {
            if (now > last_execution && now - last_execution < period)
                return false;
            last_execution = now;
            for (const auto& handle : functions)
                handle(now);
            return true;
        }
    };

    Handlers() = default;
    std::map<std::chrono::milliseconds, THandlersSamePeriod> handlers;
    std::vector<std::function<void()>> initters;
    std::vector<std::pair<String, std::function<std::map<String,String>()>>> debugHandlers;
    std::chrono::milliseconds delay_ = 250ms;
};
