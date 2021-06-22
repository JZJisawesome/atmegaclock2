# atmegaclock2

A continuation of https://gitlab.com/JZJisawesome/atmegaclock.

## Building

mkdir build

cd build

cmake -DCMAKE_BUILD_TYPE=Release ..

make -j

## Show size + flash to device (ArduinoISP)

make showSize

make flash
