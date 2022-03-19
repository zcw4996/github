/*
 * led.h
 *
 *  Created on: 2017年1月24日
 *      Author: dell
 */

#ifndef APP_DRIVER_LED_H_
#define APP_DRIVER_LED_H_

#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"

#define LED1_OFF   GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 1)    //LED1熄灭
#define LED2_OFF   GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 0)    //LED2熄灭
#define LED3_OFF   GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 1)    //LED3熄灭

#define LED1_ON    GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 0)    //LED1点亮
#define LED2_ON    GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 1)    //LED2点亮
#define LED3_ON    GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 0)    //LED3点亮

void ICACHE_FLASH_ATTR LED_Init(void);
void ICACHE_FLASH_ATTR LED_AllOn(void);
void ICACHE_FLASH_ATTR LED_AllOff(void);

#endif /* APP_DRIVER_LED_H_ */
