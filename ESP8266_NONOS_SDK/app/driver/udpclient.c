/*
 * udpclient.c
 *
 *  Created on: 2017年2月10日
 *      Author: dell
 */

#include "udpclient.h"
#include "sntp.h"
#include "mySmartlink.h"
#include "init.h"
struct espconn udpclient_esp_conn;     //建立一个espconn结构体
struct espconn Dns_esp_conn;     //建立一个espconn结构体
esp_udp  udpclient_esp_udp;            //建立一个esp_udp结构体
LOCAL os_timer_t current_stamp_timer;  //此定时器用以累加时间 
uint32_t ntpserver_ack_flag = 0;   //此标志位用来记录，ntp服务器是否有回应，判断ntp服务器是否有效

#define SERVRT_PORT    123                              // 服务器端口号        根据实际情况修改
#define NTP_SERVER_IP_MAX    13
#define NTP_ACK_TIMEOUT_COUNT 10   //NTP服务器 超时响应的最大次数

char esp_udp_server_ip_list[NTP_SERVER_IP_MAX][4] = 
{ 
	{118,24,4,66},
	{118,24,195,65},
	{58,220,133,132},
	{103,18,128,60},
	{216,218,254,202},
	{202,73,57,107},
	{128,199,134,40},
	{211,233,40,78},
	{106,247,248,106},
	{129,250,35,251},
	{131,188,3,220},
	{131,188,3,223},
	{203,114,74,17},
};     // 服务器IP       根据实际情况修改

char esp_udp_server_ip[4] = {DEFAULT_NTP_IP1,DEFAULT_NTP_IP2,DEFAULT_NTP_IP3,DEFAULT_NTP_IP4};

os_timer_t send_timer;  //定义一个定时器结构

void set_sntp_server_ip(ip_addr_t *ipaddr)
{
	esp_udp_server_ip[0] = *((uint8 *)&ipaddr->addr);
	esp_udp_server_ip[1] = *((uint8 *)&ipaddr->addr + 1);
	esp_udp_server_ip[2] = *((uint8 *)&ipaddr->addr + 2);
	esp_udp_server_ip[3] = *((uint8 *)&ipaddr->addr + 3);

	DNS_SERVER_DEBUG("set ntp ip:%d,%d,%d,%d\n",esp_udp_server_ip[0],esp_udp_server_ip[1],esp_udp_server_ip[2],esp_udp_server_ip[3]);
}

//char *sentdata="I am a udp, please send data\r\n";
char   NtpSendData[48]={
0x1B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0xD0, 0xAF, 0x5F, 0xF5, 0x23, 0xD7, 0x08, 0x00
};

ip_addr_t esp_server_ip;

/*-------------------------------------------------------------*/
/*函数功能：UDP客户端初始化函数                                                                       */
/*参       数：port：本地端口号                                                                            */
/*返       回：无                                                                                                     */
/*-------------------------------------------------------------*/
void ICACHE_FLASH_ATTR AP_udpclient_init(void)
{
	udpclient_esp_conn.type = ESPCONN_UDP;                     //espconn的类型是UDP
	udpclient_esp_conn.state = ESPCONN_NONE;                   //状态，初始的时候是空
	udpclient_esp_conn.proto.udp = &udpclient_esp_udp;        //因为是UDP类型，所以指向esp_udp结构体
	udpclient_esp_conn.proto.udp->local_port =123;           //自动获取一个可用的端口号

	espconn_regist_recvcb(&udpclient_esp_conn, udpcilent_recv_cb);   //注册接收到数据的回调函数
    espconn_create(&udpclient_esp_conn);                             //建立UDP

    os_timer_disarm(&send_timer);
    os_timer_setfn(&send_timer, (os_timer_func_t *)client_send, NULL);      //注册定时器的回调函数
    os_timer_arm(&send_timer, 1000, 0);                                                   //1s定时，非自动模式
}
extern os_timer_t sntpPutTime;
/* number of seconds between 1900 and 1970 */
#define DIFF_SEC_1900_1970         (2208988800UL)
uint32_t  current_stamp = 0;
uint32_t  current_ms = 0;  // 记录当前的毫秒数
uint32_t get_current_stamp(void)
{
	return current_stamp;
}

void ICACHE_FLASH_ATTR set_current_stamp(void *arg)
{
	 current_stamp ++;
}
/*-------------------------------------------------------------*/
/*函数功能：接收到数据的回调函数                                                                      */
/*参       数：arg：指向espconn的指针                                                                */
/*参       数：pusrdata：收到的数据                                                                    */
/*参       数：length：数据量                                                                               */
/*返       回：无                                                                                                     */
/*-------------------------------------------------------------*/
LOCAL void ICACHE_FLASH_ATTR udpcilent_recv_cb(void *arg, char *pusrdata, unsigned short length)
{
    struct espconn *pesp_conn = arg;
	remot_info *premot = NULL;

	DNS_SERVER_DEBUG("udp rece len = %d\r\n", length);      
	                   //打印收到的数据    前32bits表示1900年以来的秒数。后32bits用以表示秒以下的部份，是微秒数的4294.967296(=2^32/10^6)倍。
	current_stamp = (uint32_t) (((uint8_t)pusrdata[40] << 24) | ((uint8_t)pusrdata[41] << 16) |( (uint8_t)pusrdata[42] << 8) | ((uint8_t)pusrdata[43] << 0)) ;
	current_stamp = current_stamp - DIFF_SEC_1900_1970;

	current_ms = (uint32_t) (((uint8_t)pusrdata[44] << 24) | ((uint8_t)pusrdata[45] << 16) |( (uint8_t)pusrdata[46] << 8) | ((uint8_t)pusrdata[47] << 0)) ;
	current_ms = current_ms/4295/1000;

    user_check_sntp_stamp(NULL);

	ntpserver_ack_flag = 1;
	//为了让时间精确到毫秒，收到数据后，等到整秒后，再发数据
	os_timer_disarm(&sntpPutTime);
	os_timer_setfn(&sntpPutTime, (os_timer_func_t *)PutSntpTime, NULL);
	os_timer_arm(&sntpPutTime, (uint32_t)(1000-current_ms), 0);

}
#define DEFAULT_NTP_IP_STRING ""
///获取ntp是否被设置过
uint8_t get_ntp_is_setting(void)
{
	uint8_t flag = 0;
	uint32 ntp_server_32[32] = {0};
	uint32 ntp_server_len = 0,i = 0;
	char ntp_server_8[32] = {0};
	char ntp_server_default[32] = {0};

	Spi_FlashRead(NTP_IP_Erase,NTP_IP_LEN_ERASE_OFFSET,&ntp_server_len,1);  //读取NTP IP的长度
	Spi_FlashRead(NTP_IP_Erase,NTP_IP_ERASE_OFFSET,ntp_server_32,ntp_server_len);  //读取NTP IP
	for(i = 0; i < ntp_server_len; i ++)
	{
		ntp_server_8[i] = (char)ntp_server_32[i];
	}
	ntp_server_8[ntp_server_len] = '\0';

	os_sprintf(ntp_server_default,"%d.%d.%d.%d",DEFAULT_NTP_IP1,DEFAULT_NTP_IP2,DEFAULT_NTP_IP3,DEFAULT_NTP_IP4);

	if(os_strcmp(ntp_server_8,ntp_server_default) == 0)
	{
		flag = 0;
	}
	else
	{
		flag = 1;
	}

	DNS_SERVER_DEBUG("ntp ip change flag = %d\r\n",flag);  

	return flag;
}

extern uint32_t TimeOutInterva;
/*-------------------------------------------------------------*/
/*函数功能：定时发送                                                                                           */
/*参       数：无                                                                                                      */
/*返       回：无                                                                                                     */
/*-------------------------------------------------------------*/
void ICACHE_FLASH_ATTR client_send(void *arg)
{
	static uint32_t ntp_no_ack_count;  //此标志位用来记录，ntp服务器没有回应的次数
	DNS_SERVER_DEBUG("sent ntp ip:%d,%d,%d,%d\n",esp_udp_server_ip[0],esp_udp_server_ip[1],esp_udp_server_ip[2],esp_udp_server_ip[3]);	

	os_memcpy(udpclient_esp_conn.proto.udp->remote_ip, esp_udp_server_ip, 4);   //拷贝服务器ip
	udpclient_esp_conn.proto.udp->remote_port = SERVRT_PORT;                    //拷贝服务器端口号
	espconn_sent(&udpclient_esp_conn, NtpSendData, 48);                      //发送数据
	

	if((ntpserver_ack_flag == 0) && (get_ntp_is_setting() == 0) && get_wifi_connect_state() == 1) //按照需求，只有ntp服务器没有被设置过，才会检测切换ntp服务器
	{
		ntp_no_ack_count ++;
		if((ntp_no_ack_count-1) >= NTP_ACK_TIMEOUT_COUNT) //若超过一定次数，未收到ntp服务器的回应，则切换Ntp服务器
		{
			static uint8 index;
			ip_addr_t addr = {0};
			char ntp_server_string[32] = {0};

			ntp_no_ack_count = 0;
			index ++;
			if(index >= NTP_SERVER_IP_MAX)
			{
				index = 0;
			}
			os_sprintf(ntp_server_string,"%d.%d.%d.%d",esp_udp_server_ip_list[index][0],esp_udp_server_ip_list[index][1],esp_udp_server_ip_list[index][2],esp_udp_server_ip_list[index][3]);

			ipaddr_aton(ntp_server_string, &addr);

			set_sntp_server_ip(&addr);
		}
	}
	else 
	{
		ntp_no_ack_count = 0;
	}

	ntpserver_ack_flag = 0;	

	Spi_FlashRead(TIME_Interva_ERASE,TIME_Interva_ERASE_OFFSET,&TimeOutInterva,1);  // 设置定时发送 
	if(TimeOutInterva >= 1 && TimeOutInterva <= 9999)
	{
		os_timer_disarm(&send_timer);
		os_timer_setfn(&send_timer, (os_timer_func_t *)client_send, NULL);          //注册定时器的回调函数
		os_timer_arm(&send_timer, TimeOutInterva * 1000, 0);  
	}
	else if(TimeOutInterva == 0)
	{
		os_timer_disarm(&send_timer);
	}
           
	if(AlreadStartPutTime == 0)                      //如果开机后 未同步过时间，则1秒获取一次sntp时间
	{
		os_timer_disarm(&send_timer);
		os_timer_setfn(&send_timer, (os_timer_func_t *)client_send, NULL);          //注册定时器的回调函数
		os_timer_arm(&send_timer, 1000, 0);  	
	}

}

