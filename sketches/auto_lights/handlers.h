#pragma once

#include <functional>
#include <map>
#include <vector>

struct Handlers {
    static Handlers& the() {
        static Handlers h;
        return h;
    }

    static void delay() {
        ::delay(the().get_delay());
    }

    static void add(std::function<void(int)>&& f, int period_ms = 250) {
        auto& h = the();
        h.handlers[period_ms].functions.push_back(std::move(f));
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
        int now = millis();
        const Handlers& h = the();
        for (const auto& handle : h.handlers) {
            int period = handle.first;
            if (!handle.second.run(now, period))
                return;
        }
    }

    static String debug() {
        String resp;
        const Handlers& h = the();
        for (const auto& p : h.debugHandlers) {
            const auto& values = p.second();
            for (const auto& v : values) {
                resp += p.first + "." + v.first + ": " + v.second + "\n";
            }
            resp += "\n";
        }
        return resp;
    }

    int get_delay() const {
        return delay_;
    }

private:
    struct THandlersSamePeriod {
        std::vector<std::function<void(int)>> functions;
        mutable int last_execution = -1024;
        bool run(int now, int period) const {
            if (now > last_execution && now - last_execution < period)
                return false;
            last_execution = now;
            for (const auto& handle : functions)
                handle(now);
            return true;
        }
    };

    Handlers() = default;
    std::map<int, THandlersSamePeriod> handlers;
    std::vector<std::function<void()>> initters;
    std::vector<std::pair<String, std::function<std::map<String,String>()>>> debugHandlers;
    int delay_ = 250;
};
