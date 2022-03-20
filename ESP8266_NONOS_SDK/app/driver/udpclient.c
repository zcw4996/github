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
char esp_udp_server_ip[4] = {118,24,4,66};     // 服务器IP       根据实际情况修改
#define SERVRT_PORT    123                              // 服务器端口号        根据实际情况修改

LOCAL os_timer_t send_timer;  //定义一个定时器结构

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
/* number of seconds between 1900 and 1970 */
#define DIFF_SEC_1900_1970         (2208988800UL)
uint32_t  current_stamp = 0;

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

	DNS_SERVER_DEBUG("udp rece len = %d\r\n", length);                         //打印收到的数据
	current_stamp = (uint32_t) (((uint8_t)pusrdata[40] << 24) | ((uint8_t)pusrdata[41] << 16) |( (uint8_t)pusrdata[42] << 8) | ((uint8_t)pusrdata[43] << 0)) ;
	current_stamp = current_stamp - DIFF_SEC_1900_1970;

    os_timer_disarm(&current_stamp_timer);
    os_timer_setfn(&current_stamp_timer, (os_timer_func_t *)set_current_stamp, NULL);      //注册定时器的回调函数
    os_timer_arm(&current_stamp_timer, 1000, 1);
}


/*-------------------------------------------------------------*/
/*函数功能：定时发送                                                                                           */
/*参       数：无                                                                                                      */
/*返       回：无                                                                                                     */
/*-------------------------------------------------------------*/
void ICACHE_FLASH_ATTR client_send(void *arg)
{

	DNS_SERVER_DEBUG("sent ntp ip:%d,%d,%d,%d\n",esp_udp_server_ip[0],esp_udp_server_ip[1],esp_udp_server_ip[2],esp_udp_server_ip[3]);	
	os_memcpy(udpclient_esp_conn.proto.udp->remote_ip, esp_udp_server_ip, 4);   //拷贝服务器ip
	udpclient_esp_conn.proto.udp->remote_port = SERVRT_PORT;                    //拷贝服务器端口号
	espconn_sent(&udpclient_esp_conn, NtpSendData, 48);                      //发送数据
	

	os_timer_disarm(&send_timer);
    os_timer_setfn(&send_timer, (os_timer_func_t *)client_send, NULL);          //注册定时器的回调函数

	if(AlreadStartPutTime == 1) //若已经同步了时间，则10s获取一次ntp数据
	{
		os_timer_arm(&send_timer, 10000, 0);                                         //10s定时，非自动模式	
	}
	else
	{
   		os_timer_arm(&send_timer, 1000, 0);                                         //1s定时，非自动模式	 	
	}

}

