## Toolchain installation

https://docs.espressif.com/projects/esp-idf/en/stable/get-started/

An example for linux64 for some versions of libs&toolchain:
```
mkdir -p ~/bin/esp
cd ~/bin/esp/

wget https://dl.espressif.com/dl/xtensa-esp32-elf-linux64-1.22.0-80-g6c4433a-5.2.0.tar.gz
tar xf xtensa-esp32-elf-linux64-1.22.0-80-g6c4433a-5.2.0.tar.gz
```

## For Arduino IDE

Edit `~/.arduino15/packages/esp8266/hardware/esp8266/2.5.2/platform.txt`:
add `-std=c++1y` to `compiler.cpp.flags=` (and make sure install scrips won't override this).

