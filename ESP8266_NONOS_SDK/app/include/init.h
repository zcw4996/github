/*
 * init.h
 *
 *  Created on: 2019年7月28日
 *      Author: Administrator
 */

#ifndef APP_INCLUDE_INIT_H_
#define APP_INCLUDE_INIT_H_
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"

#define GLOBAL_DEBUG
 
#if defined(GLOBAL_DEBUG)
#define DNS_SERVER_DEBUG(format, ...) os_printf("[%s:%d]" format "\r\n", __FILE__,__LINE__,##__VA_ARGS__)
#else
#define DNS_SERVER_DEBUG(format, ...) 
#endif
#define SYS_LOG_SW  1

void ICACHE_FLASH_ATTR GetTcpClientInif(void);
void ICACHE_FLASH_ATTR user_set_station_config(char *ssid,char *password,uint32 ssidLen,uint32 PswdLen);
void  ICACHE_FLASH_ATTR ipv4_to_str(uint8 if_index, unsigned int ipv4_addr);
void ICACHE_FLASH_ATTR ConfigEsp8266(char *SSID,char *PSWD,uint16 SSIDlen,uint16 PSWDlen,struct ip_info *info1,uint8 opmode);
void ICACHE_FLASH_ATTR Get_APid_Link_AP();
void ICACHE_FLASH_ATTR vLink_AP(uint32 AP_ID);
void ICACHE_FLASH_ATTR vGetStaticIP(uint8 mode);
void ICACHE_FLASH_ATTR RecoveryData();
void ICACHE_FLASH_ATTR ConfigEsp82662(struct ip_info *info1,uint8 opmode);
void ICACHE_FLASH_ATTR ConfigEsp82663(struct ip_info *info1);
#endif /* APP_INCLUDE_INIT_H_ */
