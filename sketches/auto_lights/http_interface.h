#pragma once

#include "handlers.h"
#include "http.h"
#include "light_sensor.h"

#include "common.h"

#include <ESP8266WebServer.h>
#include <vector>

class THttpInterface {
public:
    THttpInterface(THttpSensor& httpSensor, uint16_t port = 80, TLightSensor* lightSensor = nullptr)
    : httpSensor(httpSensor)
    , lightSensor(lightSensor)
    , Server(port) {
#define HANDLE(handle,method,f) \
    do {\
        Server.on("/" #handle, HTTP_##method, [this]{f();});\
        helps.push_back({#method, "/" #handle, help_##f });\
    } while(false);
        HANDLE(      ,  GET,          root);
        HANDLE(toggle, POST,        toggle);
        HANDLE(   set, POST,  set_darkness);
        HANDLE(   get,  GET,     get_light);
        HANDLE( debug,  GET,     get_debug);
        HANDLE(  help,  GET,      get_help);
#undef HANDLE

        Handlers::addInit([this]{ Server.begin(); });
        Handlers::add([this](int){ Server.handleClient(); }, 100);
    }

    void SetLightSensor(TLightSensor* lightSensor) noexcept {
        lightSensor = lightSensor;
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
                "Left (door) light",
                "Right (car) light"
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

    static constexpr const char* help_set_darkness = "arg: int threshold. Set, when it is dark (see also 'get').";
    void set_darkness() {
        if (no_light_sensor())
            return;
        int threshold = Server.arg("threshold").toInt();
        Serial.println("set_darkness: "_str + threshold);
        if (threshold <= 0 || threshold > 1024) {
            Server.send(400, "text/plain", "out of range");
            return;
        }
        lightSensor->Darkness = threshold;
        Server.sendHeader("Location","/");
        Server.send(303);
    }

    static constexpr const char* help_get_light = "get the current brightness level (see also 'set').";
    void get_light() {
        if (no_light_sensor())
            return;
        Server.send(200, "text/plain", ""_str + lightSensor->value() + "\n");
    }

    static constexpr const char* help_get_debug = "Get all available debug info";
    void get_debug() {
        Server.send(200, "text/plain", Handlers::debug());
    }

    static constexpr const char* help_get_help = "===> You are here <===";
    void get_help() {
        String help;
        for (const auto& h : helps) {
            help += String(h.method) + " " + h.path + ": " + h.description + "\n\n";
        }
        Server.send(200, "text/plain", help);
    }

    bool no_light_sensor() {
        if (!lightSensor)
            Server.send(500, "text/plain", "there is no light sensor")
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
};
