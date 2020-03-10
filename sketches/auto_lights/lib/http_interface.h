#pragma once

#include "handlers.h"
#include "http.h"
#include "light_sensor.h"

#include "common.h"

#include <ESP8266WebServer.h>
#include <unordered_map>
#include <vector>

class THttpInterface : public Handler {
public:
    THttpInterface(THttpSensor& httpSensor, uint16_t port = 80, TLightSensor* lightSensor = nullptr)
    : Handler("HTTP server", 100ms)
    , httpSensor(httpSensor)
    , lightSensor(lightSensor)
    , Server(port) {
#define HANDLE(handle,method,f) \
    do {\
        Server.on("/" #handle, HTTP_##method, [this]{f();});\
        helps.push_back({#method, "/" #handle, help_##f });\
    } while(false);
        HANDLE(      ,  GET,          root);
        HANDLE(toggle, POST,        toggle);
        HANDLE( debug,  GET,     get_debug);
        HANDLE(  help,  GET,      get_help);
#undef HANDLE
    }

    void init() override {
        Server.begin();
    }

    void handle(std::chrono::milliseconds) override {
        Server.handleClient();
    }

    std::map<String, String> debug() const override {
        std::map<String, String> ret;
        for (const auto& h : helps)
            ret[h.path] = h.method;
        return ret;
    }

    void SetLightSensor(TLightSensor* lightSensor) noexcept {
        lightSensor = lightSensor;
    }

    void on(const char* handle, HTTPMethod method, std::function<void(ESP8266WebServer& server)>&& func, const char* help) {
        handlers[handle] = [this, method, handleFunc = std::move(func)]{
            handleFunc(Server);
            if (method == HTTP_POST) {
                // note: it might happen after a response has been sent. No big deal.
                Server.sendHeader("Location", "/");
                Server.send(303, "text/plain", "OK\n");
            }
        };
        Server.on(handle, method, handlers.at(handle));
        const char* name = "UNKNOWN";
        switch (method) {
#define METHOD(x) case HTTP_##x: name = #x; break;
            METHOD(ANY);
            METHOD(GET);
            METHOD(HEAD);
            METHOD(POST);
            METHOD(PUT);
            METHOD(PATCH);
            METHOD(DELETE);
            METHOD(OPTIONS);
#undef METHOD
        }
        helps.push_back({name, handle, help});
    }

private:
    String HtmlToggle(int id, const char* label) {
        return
        "<h3>"_str + label + "</h3>"                            "\n"
        "<p>"                                                   "\n"
        R"( <form action="/toggle" method="POST">)"             "\n"
        R"(  <input type="hidden" name="id" value=)" + id + ">" "\n"
        R"(  <input type="hidden" name="state" value="1">)"     "\n"
        R"(  <input type="submit" value="On">)"                 "\n"
        R"( </form>)"                                           "\n"
        R"( <form action="/toggle" method="POST">)"             "\n"
        R"(  <input type="hidden" name="id" value=)" + id + ">" "\n"
        R"(  <input type="hidden" name="state" value="2">)"     "\n"
        R"(  <input type="submit" value="Off">)"                "\n"
        R"( </form>)"                                           "\n"
        "</p>"                                                  "\n";
    }

    static constexpr const char* help_root = "An HTML page with controls";
    void root() {
        static String html = [&]{
            const char* labels[] = {
                "Invalid (should not be seen)",
                "Indoor",
                "Outdoor",
                "Heater",
            };
            static_assert(sizeof(labels) / sizeof(labels[0]) == THttpSensor::EUnused, "...");
            String ret;
            for (int i = 1; i < THttpSensor::EUnused; ++i)
                ret += HtmlToggle(i, labels[i]);
            return ret;
        }();
        Server.send(200, "text/html", html);
    }

    static constexpr const char* help_toggle = "args: int id, int state (1 = on, 2 = off). Allow or not a switch to turn on.\n  A light sensor may make the final decision.";
    void toggle() {
        int id = Server.arg("id").toInt();
        int state = Server.arg("state").toInt();
        Serial.println("toggle (id: "_str + id + ", state: " + state + ")");
        if (!httpSensor.valid(id) or !state or state > 3) {
            Server.send(400, "text/plain", "out of range\n");
            return;
        }
        state &= 1;
        httpSensor[id] = state;
        // Behave nice with web-interface
        Server.sendHeader("Location","/");
        Server.send(303, "text/plain", "OK\n");
    }

    static constexpr const char* help_get_debug = "Get all available debug info";
    void get_debug() {
        Server.send(200, "text/plain", Handlers::debug());
    }

    static constexpr const char* help_get_help = "===> You are here <===";
    void get_help() {
        String help = "To POST args: curl /handle -XPOST --data 'arg1=valu1;arg2=value2'\n\n";
        for (const auto& h : helps) {
            help += String(h.method) + " " + h.path + ": " + h.description + "\n\n";
        }
        Server.send(200, "text/plain", help);
    }

    bool no_light_sensor() {
        if (!lightSensor)
            Server.send(500, "text/plain", "there is no light sensor\n");
        return not lightSensor;
    }

    THttpSensor& httpSensor;
    TLightSensor* lightSensor;
    ESP8266WebServer Server;
    struct THelpItem {
        const char* method;
        const char* path;
        const char* description;
    };
    std::vector<THelpItem> helps;
    std::unordered_map<const char*, std::function<void()>> handlers;
};
