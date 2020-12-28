#include "lib/camera.h"
#include "lib/main.h"

TDefaultSetup dc;
TEspCam<> cam;

const char* NConfig::hostname = "camera";

namespace {
    struct TSetup : Handler {
        TSetup()
        : Handler("setup", 360min)
        {}

        void init() override {
            dc.Http.on("/jpg", HTTP_GET, [&](THttpInterface<>::TServer& server){
                camera_fb_t* buffer = nullptr;
                while (!buffer) {
                    cam.SwitchLed(server.arg("led") == "1");
                    delay(1);
                    buffer = esp_camera_fb_get();
                    cam.SwitchLed(false);
                }

                if(buffer->format != PIXFORMAT_JPEG) {
                    server.send(500, "text/plain", "not a jpeg (this is a bug in firmware)");
                    return;
                }
                server.sendHeader("Content-Type", "image/jpeg");
                server.sendContent_P(reinterpret_cast<const char*>(buffer->buf), buffer->len);
                esp_camera_fb_return(buffer);
            }, "Get camshot ('?led=1' for flashlight)");
        }

        void handle(std::chrono::milliseconds) override {}
        std::map<String, String> debug() const override { return {}; }
    };
}

TSetup cfg;
