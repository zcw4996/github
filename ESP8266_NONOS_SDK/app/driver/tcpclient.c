/*
 * tcpcilent.c
 *
 *  Created on: 2017骞�2鏈�8鏃�
 *      Author: dell
 */

#include "tcpclient.h"
#include "tcpserver.h"
#include "uart.h"
#include "init.h"
struct espconn tcpcilent_esp_conn;   //建立一个espconn结构体
esp_tcp tcpcilent_esptcp;            //建立一个esp_tcp结构体

const char esp_tcp_server_ip[4] = {192, 168, 0, 102};    // 服务器IP       根据实际情况修改
#define SERVRT_PORT    5000                             // 服务器端口号        根据实际情况修改

LOCAL os_timer_t connect_timer;  //定义一个定时器结构
//LOCAL os_timer_t send_timer;     //定义一个定时器结构

char *sentdata="<!DOCTYPE html><html lang=\"en\"><head>    <meta charset=\"UTF-8\">    <title>CSS</title>    <link rel=\"stylesheet\" type=\"text/css\" href=\"a.css\"></head><body><a>GXDZ</a></body></html>\r\n";
LOCAL void  ICACHE_FLASH_ATTR tcp_cilent_Send_cb(void *arg)
{
	   struct espconn *pespconn = arg;
	   uint32 TcpType = 0;
	   Spi_FlashRead(TCP_SERVERIP_Erase,TCP_SERVER_TPYE_ERASE_OFFSET,&TcpType,1);
	   if(TcpType != 'L') /* 类型不是长连接，则发送数据成功后直接断开 */
	   {
		   espconn_disconnect(pespconn);
	   }
}
/*-------------------------------------------------------------*/
/*函数功能：TCP客户端初始化函数                                                                       */
/*参       数：port：本地端口号                                                                            */
/*返       回：无                                                                                                     */
/*-------------------------------------------------------------*/
void ICACHE_FLASH_ATTR AP_tcpclient_init(char *TCP_ServerIP,uint32 TCP_ServerPort)
{
	tcpcilent_esp_conn.type = ESPCONN_TCP;            //espconn的类型是TCP
	tcpcilent_esp_conn.state = ESPCONN_NONE;          //状态，初始的时候是空
	tcpcilent_esp_conn.proto.tcp = &tcpcilent_esptcp; //因为是TCP类型，所以指向esptcp结构体

	espconn_disconnect(&tcpcilent_esp_conn);

	os_memcpy(tcpcilent_esp_conn.proto.tcp->remote_ip, TCP_ServerIP, 4);   //拷贝服务器ip
	tcpcilent_esp_conn.proto.tcp->remote_port = TCP_ServerPort;                    //拷贝服务器端口号
	tcpcilent_esp_conn.proto.tcp->local_port = espconn_port();                  //自动获取可用的端口号

    DNS_SERVER_DEBUG("AP_tcpclient_init\n");
	espconn_regist_connectcb(&tcpcilent_esp_conn, tcp_cilent_connect_cb); //注册连接成功后的回调函数
	espconn_regist_reconcb(&tcpcilent_esp_conn, tcp_client_recon_cb);     //注册连接异常断开的回调函数
	espconn_connect(&tcpcilent_esp_conn);                                 //连接服务器
}

/*-------------------------------------------------------------*/
/*函数功能：连接成功后的回调函数                                                                     */
/*参       数：arg：指向espconn的指针                                                                 */
/*返       回：无                                                                                                     */
/*-------------------------------------------------------------*/
LOCAL void ICACHE_FLASH_ATTR tcp_cilent_connect_cb(void *arg)
{
    struct espconn *pespconn = arg;
	int32 TcpType = 0;
    Spi_FlashRead(TCP_SERVERIP_Erase,TCP_SERVER_TPYE_ERASE_OFFSET, &TcpType,1);
    DNS_SERVER_DEBUG("connect succeed !!! \r\n");    //提示连接服务器成功

    espconn_regist_recvcb(pespconn, tcp_client_recv_cb);     //接收到数据的回调函数
    espconn_regist_disconcb(pespconn, tcp_cilent_discon_cb); //连接正常断开的回调函数
    espconn_regist_sentcb(pespconn,   tcp_cilent_Send_cb);     //注册数据发送成功后的回调函数
    if(TcpType != 'L')/* 若是短连接 则连接成功后就发送数据 */
    {
      espconn_sent(&tcpcilent_esp_conn, UART0_RX_BUFF, data_len);
    }
}

/*-------------------------------------------------------------*/
/*函数功能：连接异常断开的回调函数                                                                   */
/*参       数：arg：指向espconn的指针                                                                 */
/*参       数：err：错误代码                                                                                  */
/*返       回：无                                                                                                     */
/*-------------------------------------------------------------*/
LOCAL void ICACHE_FLASH_ATTR tcp_client_recon_cb(void *arg, sint8 err)
{
	   struct espconn *pespconn = arg;
	   uint32 TcpType = 0;

	   Spi_FlashRead(TCP_SERVERIP_Erase,TCP_SERVER_TPYE_ERASE_OFFSET,&TcpType,1);
	   if(TcpType != 'L') /* 类型不是长连接，则发送数据成功后直接断开 */
	   {

	   }
	   else
	   {
		DNS_SERVER_DEBUG("Connection aborted, error code:%d !!! \r\n",err);   //提示连接是异常断开的
		DNS_SERVER_DEBUG("Reconnect after 2 seconds\r\n");                                //提示2秒后重连
		os_timer_disarm(&connect_timer);
		os_timer_setfn(&connect_timer, (os_timer_func_t *)connect_servre, NULL);   //注册定时器的回调函数
		os_timer_arm(&connect_timer, 2000, 0);                                     //2s定时，非自动模式
	   }
}

/*-------------------------------------------------------------*/
/*函数功能：连接正常断开的回调函数                                                                   */
/*参       数：arg：指向espconn的指针                                                                 */
/*返       回：无                                                                                                     */
/*-------------------------------------------------------------*/
LOCAL void ICACHE_FLASH_ATTR tcp_cilent_discon_cb(void *arg)
{
	struct espconn *pespconn = arg;
	uint32 TcpType = 0;
	//os_timer_disarm(&send_timer);  //断开了，取消发送定时
	DNS_SERVER_DEBUG("The connection is normally disconnected\r\n");       //提示连接正常断开

    Spi_FlashRead(TCP_SERVERIP_Erase,TCP_SERVER_TPYE_ERASE_OFFSET,&TcpType,1);
	if(TcpType != 'L') /* 类型不是长连接，则发送数据成功后直接断开 */
    {

	}
	else
	{  /* 长连接的话，则2S后重连 */
		DNS_SERVER_DEBUG("Reconnect after 2 seconds\r\n");                                //提示2秒后重连
		os_timer_disarm(&connect_timer);
		os_timer_setfn(&connect_timer, (os_timer_func_t *)connect_servre, NULL);   //注册定时器的回调函数
		os_timer_arm(&connect_timer, 2000, 0);                                     //2s定时，非自动模式
	}
}

/*-------------------------------------------------------------*/
/*函数功能：接收到数据的回调函数                                                                      */
/*参       数：arg：指向espconn的指针                                                                */
/*参       数：pusrdata：收到的数据                                                                    */
/*参       数：length：数据量                                                                               */
/*返       回：无                                                                                                     */
/*-------------------------------------------------------------*/
LOCAL void ICACHE_FLASH_ATTR tcp_client_recv_cb(void *arg, char *pusrdata, unsigned short length)
{
   struct espconn *pespconn = arg;

   uart0_tx_buffer(pusrdata,length);    //打印收到的数据
   //espconn_sent(pespconn, pusrdata, length);         //把收到的数据，原样发回
}

/*-------------------------------------------------------------*/
/*函数功能：断线重连                                                                                           */
/*参       数：无                                                                                                      */
/*返       回：无                                                                                                     */
/*-------------------------------------------------------------*/
void ICACHE_FLASH_ATTR connect_servre(void)
{
	espconn_connect(&tcpcilent_esp_conn);  //连接服务器
}
void ICACHE_FLASH_ATTR SendTcpClientData(uint8 *TxBuffer,uint32 BufLen)
{
	int32 TcpServerIP_Len = 0;

	struct ip_info dynamic_ipconfig;
	wifi_get_ip_info(STATION_IF, &dynamic_ipconfig);  //获取动态IP
    if (dynamic_ipconfig.ip.addr == 0)                 //调用查询 状态函数，ipconfig结构体中ip值为0则未获取到IP地址
    {
    	return;
    }
	Spi_FlashRead(TCP_SERVERIP_Erase,TCP_SERVERIPLen_ERASE_OFFSET,&TcpServerIP_Len,1);
	if(TcpServerIP_Len < 4)/*则说明未配置TcpClient*/
	{
		return;
	}
	else
	{
		int32 TcpType = 0,i = 0;
	    Spi_FlashRead(TCP_SERVERIP_Erase,TCP_SERVER_TPYE_ERASE_OFFSET, &TcpType,1);

	    if(TcpType != 'L')
	    {
		    int32_t TcpPort = 0;
		    Spi_FlashRead(TCP_SERVERIP_Erase,TCP_SERVER_PORT_ERASE_OFFSET, &TcpPort,1);
	    	AP_tcpclient_init(TcpClientToServer,TcpPort);/* 短连接 则每次都重新连接TCP服务器 */
	    }
	    else
	    {
	    	espconn_sent(&tcpcilent_esp_conn, TxBuffer, BufLen);
	    }
	}

}

