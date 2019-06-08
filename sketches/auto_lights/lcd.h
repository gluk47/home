#include "handlers.h"
#include "pins.h"
#include "wifi.h"

#include <LiquidCrystal_I2C.h>

class TLcd {
public:
    TLcd(WifiClient& wifi) {
        Handlers::addInit([this]{
            lcd.begin();
            lcd.printstr("  I like wire!");
        });

        Handlers::add([this, &wifi](int){
            const String& new_ip = wifi.ip();
            if (new_ip == last_ip)
                return;
            last_ip = new_ip;
            lcd.home();
            char out[17] = {};
            snprintf(out, sizeof(out), "%16s", new_ip.c_str());
            Serial.println(out);
            lcd.printstr(out);
        }, 1000);
    }

private:
    LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x3f, 16, 2);
    String last_ip;
};
