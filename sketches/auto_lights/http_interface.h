#pragma once

#include "handlers.h"
#include "http.h"
#include "light_sensor.h"

#include "common.h"

#include <ESP8266WebServer.h>

class THttpInterface {
public:
    THttpInterface(THttpSensor& httpSensor, TLightSensor& lightSensor, uint16_t port = 80)
    : httpSensor(httpSensor)
    , lightSensor(lightSensor)
    , Server(port) {
#define HANDLE(handle,method,f) Server.on("/" #handle, HTTP_##method, [this]{f();})
        HANDLE(      ,  GET,          root);
        HANDLE(toggle, POST,        toggle);
        HANDLE(   set, POST,  set_darkness);
        HANDLE(   get,  GET,     get_light);
        HANDLE( debug,  GET,     get_debug);
#undef HANDLE

        Handlers::add([this](int){
            Server.handleClient();
        });

        Server.begin();
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
        Server.send(303);
    }

    void set_darkness() {
        int threshold = Server.arg("threshold").toInt();
        Serial.println("set_darkness: "_str + threshold);
        if (threshold <= 0 || threshold > 1024) {
            Server.send(400, "text/plain", "out of range");
            return;
        }
        lightSensor.Darkness = threshold;
        Server.sendHeader("Location","/");
        Server.send(303);
    }

    void get_light() {
        Server.send(200, "text/plain", ""_str + lightSensor.value() + "\n");
    }

    void get_debug() {
        Server.send(200, "text/plain", Handlers::debug());
    }

    THttpSensor& httpSensor;
    TLightSensor& lightSensor;
    ESP8266WebServer Server;
};
