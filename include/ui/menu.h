#ifndef MENU_H
#define MENU_H

#include <stdint.h>
#include <stdbool.h>

//For ui code
void MENU_setup();
void MENU_update(uint8_t buttons);
bool MENU_readyToExit();
void MENU_clearExitFlag();

#endif//MENU_H
