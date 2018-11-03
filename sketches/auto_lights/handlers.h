#pragma once

#include <functional>
#include <vector>

struct Handlers {
    static Handlers& the() {
        static Handlers h;
        return h;
    }

    static void add(std::function<void(int)>&& f) {
        the().all.push_back(std::move(f));
    }

    static void handle() {
        int when = millis();
        const Handlers& h = the();
        for (const auto& handle : h.all)
            handle(when);
    }

private:
    Handlers() = default;
    std::vector<std::function<void(int)>> all;
};
