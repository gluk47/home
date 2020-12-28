#pragma once

#include <esp_camera.h>

#include "handlers.h"

struct TAiThinkerCamConfig : camera_config_t {
    TAiThinkerCamConfig() {
        pin_d0 = 5;
        pin_d1 = 18;
        pin_d2 = 19;
        pin_d3 = 21;
        pin_d4 = 36;
        pin_d5 = 39;
        pin_d6 = 34;
        pin_d7 = 35;
        pin_xclk = 0;
        pin_pclk = 22;
        pin_vsync = 25;
        pin_href = 23;
        pin_sscb_sda = 26;
        pin_sscb_scl = 27;
        pin_pwdn = 32;
        pin_reset = -1;
    }

    int pin_led = 4;
};

template <typename TConfig = TAiThinkerCamConfig>
class TEspCam : public Handler {
public:
    TEspCam()
    : Handler("camera", 5ms) {
        config.jpeg_quality = 8;
        config.pixel_format = PIXFORMAT_JPEG;
        config.fb_count = 1;
        config.xclk_freq_hz = 20000000;
        config.frame_size = FRAMESIZE_SXGA;
    }

    TConfig config;

    void init() override {
        esp_err_t result = esp_camera_init(&config);
        if (result != ESP_OK) {
            Serial.printf("Espcam init failed: %s (code %d)", esp_err_to_name(result), result);
        }

        pinMode (config.pin_led, OUTPUT);
    }

    void handle (std::chrono::milliseconds) override {}

    std::map<String, String> debug() const override {
        return {};
    }

    void SwitchLed(bool enable) {
        digitalWrite(config.pin_led, enable? HIGH : LOW);
    }

    const camera_fb_t* GetFb() const {
        return esp_camera_fb_get();
    }

private:
};
