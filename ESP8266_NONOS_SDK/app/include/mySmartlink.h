/*
 * mySmartlink.h
 *
 *  Created on: 2017年2月8日
 *      Author: dell
 */

#ifndef APP_INCLUDE_MYSMARTLINK_H_
#define APP_INCLUDE_MYSMARTLINK_H_

#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "smartconfig.h"
//extern struct espconn Dns_esp_conn;     //建立一个espconn结构体
//extern ip_addr_t esp_server_ip;
extern uint8 AlreadStartPutTime;
void ICACHE_FLASH_ATTR smartconfig_done(sc_status status, void *pdata);
void ICACHE_FLASH_ATTR smartconfig_led(void);
unsigned char ICACHE_FLASH_ATTR Get_Week(unsigned int Year,unsigned char Month,unsigned char Date);
void ICACHE_FLASH_ATTR user_esp_platform_dns_found(const char *name, ip_addr_t *ipaddr, void *arg);
void ICACHE_FLASH_ATTR user_check_sntp_stamp(void *arg);
#endif /* APP_INCLUDE_MYSMARTLINK_H_ */
