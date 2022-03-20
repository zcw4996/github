/*
 * ESPRSSIF MIT License
 *
 * Copyright (c) 2016 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__
#include "mem.h"

#define Init_Erase  124  //此扇区数据记录是否要复位到出厂设置

#define AP_NAME_PSWD_Erase   125  //此扇区数据记录客户配置的AP名称和密码
#define AP_NAME_ERASE_OFFSET   0 //此扇区偏移地址用来存储AP名称
#define AP_NAME_LEN_ERASE_OFFSET   240  //此扇区偏移地址用来存储AP名称长度
#define AP_PSWD_ERASE_OFFSET   148  //此扇区偏移地址用来存储AP密码
#define AP_PSWD_LEN_ERASE_OFFSET   280  //此扇区偏移地址用来存储AP密码长度

#define AP1_NAME_PSWD_Erase   131  //此扇区数据记录客户配置的第二个AP名称和密码

#define NO_APID 0   //表示还未存入AP数据
#define APID1   1
#define APID2   2

#define AP_NUM_Erase   132  //此扇区数据记录客户配置的AP个数
#define AP_NUM_ERASE_OFFSET   0 //此扇区偏移地址用来存储AP个数

#define AP_NUM_MAX       2     //最大记录几个AP数据

#define TIMEZONE_ERASE  123
#define TIMEZONE_ERASE_OFFSET  0 //此扇区偏移地址用来存储时区

#define TIME_Interva_ERASE  133  //输出NTP时间数据的时间间隔
#define TIME_Interva_ERASE_OFFSET  0

#define LOCAL_Erase   126  //此扇区数据记录客户配置的本地IP，网关，掩码
#define LOCAL_IP_ERASE_OFFSET   0 //此扇区偏移地址用来存储本地IP地址
#define LOCAL_Mask_ERASE_OFFSET   96     //此扇区偏移地址用来存储本地子网掩码
#define LOCAL_GATAWAY_ERASE_OFFSET   220     //此扇区偏移地址用来存储本地网关

#define FIRST_START_Erase   136  ///* 此扇区记录是否第一次启动设备 */
#define FIRST_START_ERASE_OFFSET   0 //

#define IS_AUTO_WEB_Erase   137  ///* 此扇区记录是是否自动弹出网页 */
#define IS_AUTO_WEB_ERASE_OFFSET   0 //

#define TCP_PORT_Erase   138  ///* 此扇区记录TCP服务器端口号  */
#define TCP_PORT_ERASE_OFFSET   0 //

#define TCP_SERVERIP_Erase   139            ///* 此扇区记录TCP客户端参数  */
#define TCP_SERVERIP_ERASE_OFFSET   0       ///* 此扇区记录TCP客户端IP或域名  */
#define TCP_SERVERIPLen_ERASE_OFFSET   96    ///* 此扇区记录TCP客户端IP或域名长度  */
#define TCP_SERVER_PORT_ERASE_OFFSET   120   ///* 此扇区记录TCP客户端IP或域名的端口  */
#define TCP_SERVER_TPYE_ERASE_OFFSET   220   ///* 此扇区记录TCP类型，长连接还是短连接  */

#define LANGUAGE_Erase   122            ///* 此扇区记录语言版本参数  */
#define LANGUAGE_ERASE_OFFSET   0       ///* 此扇区记录语言版本参数  */

#define LANGUAGE_EN     0x5a            /*英语版本*/
#define LANGUAGE_CH     0xa5            /*中文版本*/

#define NO_DHCP_Erase   135  //
#define NO_DHCP_IP_ERASE_OFFSET   0 //
#define NO_DHCP_Mask_ERASE_OFFSET   96     //
#define NO_DHCP_GATAWAY_ERASE_OFFSET   220     //
#define DHCP_OPEN_ERASE_OFFSET   320     //

#define LOCALSSID_Erase   134  //此扇区数据记录客户配置的SSID 密码
#define SSID_ERASE_OFFSET   300  //此扇区偏移地址用来存储SSID数据
#define SSID_Len_ERASE_OFFSET   444  //此扇区偏移地址用来存储SSID数据长度
#define PSWD_ERASE_OFFSET   400  //此扇区偏移地址用来存储密码数据
#define PSWD_LEN_ERASE_OFFSET   440  //此扇区偏移地址用来存储密码长度


#define NTP_IP_Erase   127  //此扇区数据记录客户配置的NTP ip
#define NTP_IP_ERASE_OFFSET   0 //此扇区偏移地址用来存储NTP ip
#define NTP_IP_LEN_ERASE_OFFSET   220     //此扇区偏移地址用来NTP IP的长度

#define DST_SET    0x5a
#define DST_NO_SET 0Xa5

#define DHCP_CLOSE 0x5a
#define DHCP_OPEN  0xa5

#define AUTO_ACCESS_WEB     '1'
#define NO_AUTO_ACCESS_WEB  '2'

#define DST_Erase   128  //此扇区数据记录客户配置的夏令时
#define ISSET_DST_ERASE_OFFSET   0    //此扇区偏移地址用来记录是否要设置夏令时
#define DSTSTART_HOUR_ERASE_OFFSET   32 //此扇区偏移地址用来夏令时起始小时
#define DSTSTART_SELE_WEEK_ERASE_OFFSET   64    /*此扇区偏移地址用来存储夏令时星期选择 */
#define DSTSTART_WEEK_ERASE_OFFSET 96      /* 此扇区偏移地址用来存储夏令时星期 */
#define DSTSTART_MON_ERASE_OFFSET  128      /* 此扇区偏移地址用来存储夏令时月份 */

#define DSTEND_HOUR_ERASE_OFFSET   160 //此扇区偏移地址用来夏令时结束小时
#define DSTEND_SELE_WEEK_ERASE_OFFSET   192    /*此扇区偏移地址用来存储夏令时星期选择 */
#define DSTEND_WEEK_ERASE_OFFSET 224      /* 此扇区偏移地址用来存储夏令时星期 */
#define DSTEND_MON_ERASE_OFFSET  256      /* 此扇区偏移地址用来存储夏令时月份 */

#define DST_STRAT  0X5a
#define DST_FINISH 0Xa5
#define DST_TIMEOUT_Erase   129
#define DST_TIMEOUT_ERASE_OFFSET   0      //此扇区偏移地址用来记录夏令时是否开始

#define NTP_MODE    0x5a
#define APP_MODE    0XA5

#define NTPAPP_Erase   130
#define NTPAPP_ERASE_OFFSET   0      //此扇区偏移地址用来记录是NTP模式还是APP模式

#define SEVER1_FLASH_Erase 530    //网页1的存储地址
#define SEVER2_FLASH_Erase 525    //网页2的存储地址
#define SEVER3_FLASH_Erase 510    //网页3的存储地址
#define SEVER4_FLASH_Erase 515    //网页4的存储地址
#define SEVER5_FLASH_Erase 520    //网页5的存储地址
#define INIT_SET    0x5a    //代表需要复位出厂设置
#define INIT_NO_SET 0xa5    //代表不需要复位到出厂设置

#define APPLINK   0x5a
#define NOAPPLINK 0xa5

#define WEB_SET_SUCCESS 0
#define WEB_SET_FAIL    1
#define ERROR   2
typedef struct{
	uint32 NtpseverIPlen;
	int32 Timezone;
	uint32 IP[4];
	uint32 gw[4];
	uint32 netmask[4];
	uint32 IsSetDST;
	uint32 NtpAppMode;
	uint32 SsidLen;
	uint32 SsidPswdLen;
	uint32 AP_num;
	uint32 TimePutInterva;
	uint32 DHCP_State;
	uint32 Auto_Web_State;
	uint32 TCP_Server_Port;
	uint32 TcpServerIP_Len;
	uint32 Language_state;
}AllSetData;
extern uint32 SetDevideCount;
void Spi_FlashWrite(uint32 Erase_Point,uint32 Erase_Offset,uint32 * data_point,uint32 data_len);
void Spi_FlashRead(uint32 Erase_Point,uint32 Erase_Offset,uint32 * data_point,uint32 data_len);

#endif

