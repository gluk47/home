#pragma once

struct Pins {
    constexpr static int
        No = -1,
        Indoor[] = { D1, D2 },
        LCD_SCL = D3,
        LCD_SDA = D4,
        OutdoorDoor = D5,
        OutdoorPass = D6;
};
