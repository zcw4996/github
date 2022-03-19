#ifndef APP_INCLUDE_RGBLED_H_
#define APP_INCLUDE_RGBLED_H_
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"

#define REDLED  0
#define GREENLED  1
#define BLUELED  2
extern uint8_t LedChoose ;
void  RgbInit(void);
void ICACHE_FLASH_ATTR LedBlue(uint8_t State);
void ICACHE_FLASH_ATTR LedGreen(uint8_t State);
void ICACHE_FLASH_ATTR LedRed(uint8_t State);
void ICACHE_FLASH_ATTR LedCloseALL(void);
void ICACHE_FLASH_ATTR LedOpenChoose(uint8_t State);
void ICACHE_FLASH_ATTR LedCloseChoose(uint8_t State);
#endif /* APP_INCLUDE_INIT_H_ */
