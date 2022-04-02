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

//STATION模式的默认IP网关
#define DEFAULT_STATION_IP1 192
#define DEFAULT_STATION_IP2 168
#define DEFAULT_STATION_IP3 1
#define DEFAULT_STATION_IP4 100

#define DEFAULT_STATION_GW1 192
#define DEFAULT_STATION_GW2 168
#define DEFAULT_STATION_GW3 1
#define DEFAULT_STATION_GW4 100

#define DEFAULT_STATION_NETMASK1 255
#define DEFAULT_STATION_NETMASK2 255
#define DEFAULT_STATION_NETMASK3 255
#define DEFAULT_STATION_NETMASK4 0


//AP模式的默认IP网关
#define DEFAULT_AP_IP1 192
#define DEFAULT_AP_IP2 168
#define DEFAULT_AP_IP3 75
#define DEFAULT_AP_IP4 100

#define DEFAULT_AP_GW1 192
#define DEFAULT_AP_GW2 168
#define DEFAULT_AP_GW3 75
#define DEFAULT_AP_GW4 100

#define DEFAULT_AP_NETMASK1 255
#define DEFAULT_AP_NETMASK2 255
#define DEFAULT_AP_NETMASK3 255
#define DEFAULT_AP_NETMASK4 0

#define AUTO_SELECT_NTP_STRING  "Auto"
#define AUTO_SELECT_NTP_STRING1 "auto"
#define AUTO_SELECT_NTP_STRING2 "AUTO"

//默认的NTP服务器
#define DEFAULT_NTP_IP1 118
#define DEFAULT_NTP_IP2  24
#define DEFAULT_NTP_IP3   4
#define DEFAULT_NTP_IP4  66

#define GLOBAL_DEBUG
 
#if defined(GLOBAL_DEBUG)
#define DNS_SERVER_DEBUG(format, ...) os_printf("[%s:%d]" format "", __FILE__,__LINE__,##__VA_ARGS__)
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
void ICACHE_FLASH_ATTR ConfigEsp82663(struct ip_info *info1);
#endif /* APP_INCLUDE_INIT_H_ */
