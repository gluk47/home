#pragma once

#include <functional>
#include <map>
#include <vector>

struct Handlers {
    static Handlers& the() {
        static Handlers h;
        return h;
    }

    static void add(std::function<void(int)>&& f) {
        the().all.push_back(std::move(f));
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
        int when = millis();
        const Handlers& h = the();
        for (const auto& handle : h.all)
            handle(when);
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

private:
    Handlers() = default;
    std::vector<std::function<void(int)>> all;
    std::vector<std::function<void()>> initters;
    std::vector<std::pair<String, std::function<std::map<String,String>()>>> debugHandlers;
};
