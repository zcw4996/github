/*
 * soft_timer.h
 *
 *  Created on: 2019年7月28日
 *      Author: Administrator
 */

#ifndef APP_INCLUDE_SOFT_TIMER_H_
#define APP_INCLUDE_SOFT_TIMER_H_
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"

extern os_timer_t Init_timer,Check_SmartConfig,test_timer,sntpPutTime,LedCloseTime,TcpClientLink;  //定义一个定时器结构
extern int32 Init_Setup;
extern uint32_t timerForLinkAP;
extern uint8 LinkAPstate;
extern uint32_t Init_Setup_timer;
void ICACHE_FLASH_ATTR Check_Smart_Link(void);
void ICACHE_FLASH_ATTR user_check_ip(void);
void ICACHE_FLASH_ATTR Init_Set_up(void);
void ICACHE_FLASH_ATTR SetNtp();
void ICACHE_FLASH_ATTR IsTcpClientLink(void);
uint8_t get_wifi_connect_state(void);
#endif
