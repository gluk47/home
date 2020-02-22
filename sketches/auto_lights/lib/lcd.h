#include "handlers.h"
#include "pins.h"
#include "wifi.h"

#include <LiquidCrystal_I2C.h>

class TLcd : public Handler {
public:
    TLcd(const WifiClient& wifi)
    : Handler("LCD", 1000ms)
    , wifi(wifi)
    { }

    void init() override {
        lcd.begin();
        lcd.printstr("  I like wire!");
    }

    void handle(std::chrono::milliseconds) override {
        const String& new_ip = wifi.ip();
        if (new_ip == last_ip)
            return;
        last_ip = new_ip;
        lcd.home();
        char out[17] = {};
        snprintf(out, sizeof(out), "%16s", new_ip.c_str());
        Serial.println(out);
        lcd.printstr(out);
    }

    std::map<String, String> debug() const override {
        return {
            {"Last IP", last_ip}
        };
    }

private:
    LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x3f, 16, 2);
    String last_ip;
    const WifiClient& wifi;
};
