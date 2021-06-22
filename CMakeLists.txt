#Thanks https://cmake.org/cmake/help/latest/guide/tutorial/
#Also useful:https://kubasejdak.com/how-to-cross-compile-for-embedded-with-cmake-like-a-champ

cmake_minimum_required(VERSION 3.10)

#Project name and version
project(atmegaclock2 VERSION 0.1 LANGUAGES C)

#Cross compiler stuffs
set(CMAKE_TRY_COMPILE_TARGET_TYPE   STATIC_LIBRARY)
set(CMAKE_SYSTEM_PROCESSOR          avr)
set(CMAKE_AR                        avr-ar)
set(CMAKE_ASM_COMPILER              avr-as)
set(CMAKE_C_COMPILER                avr-gcc)
set(CMAKE_LINKER                    avr-ld)
set(CMAKE_OBJCOPY                   avr-objcopy)
set(CMAKE_RANLIB                    avr-ranlib)
set(CMAKE_SIZE                      avr-size)
set(CMAKE_STRIP                     avr-strip)

#Compiler flags
set(CMAKE_C_FLAGS           "-Wall -mmcu=atmega328p -std=gnu17")#Common flags
set(CMAKE_C_FLAGS_DEBUG     "-Og -g")
set(CMAKE_C_FLAGS_RELEASE   "-Ofast -fomit-frame-pointer -pipe -flto -fuse-linker-plugin -fgraphite-identity -floop-nest-optimize -fipa-pta -fno-semantic-interposition -fdevirtualize-at-ltrans -fno-common -fno-plt -DNDEBUG")
#set(CMAKE_EXE_LINKER_FLAGS )
add_compile_definitions(F_CPU=16000000)

#CMake config header for atmegaclock2 to reference
configure_file(include/cmake_config_info.h.in cmake_config_info.h)

#Sources and final executable name
add_executable(atmegaclock2 include/cmake_config_info.h.in include/eeprom.h include/buzzer.h include/i2c.h include/lcd.h include/rtc.h include/ui/alarm.h include/ui/clock.h include/ui/menu.h include/ui/ui.h src/main.c src/eeprom.c src/buzzer.c src/i2c.c src/lcd.c src/rtc.c src/ui/alarm.c src/ui/clock.c src/ui/menu.c src/ui/ui.c)

#Include directories
target_include_directories(atmegaclock2 PUBLIC "build/" "include/")

#Custom commands/targets
add_custom_command(TARGET atmegaclock2 POST_BUILD COMMAND ${CMAKE_OBJCOPY} -O ihex atmegaclock2 atmegaclock2.hex)
add_custom_target(flash DEPENDS atmegaclock2.hex COMMAND avrdude -c stk500v1 -P /dev/ttyUSB0 -p atmega328p -U flash:w:atmegaclock2.hex:i -U eeprom:w:0x00,0x05:m -b19200 -v)
add_custom_target(showSize DEPENDS atmegaclock2 COMMAND avr-size -Ax ./atmegaclock2)
