/******************************************************************************
 * FileName: espconn.c
 *
 * Description: a dns server dome, just for a little doname.
 *
 * Modification history:
 *     2018/10/22, zhangpeng, v1.0 create this file.
 * 
 * DNS Protocol:
 *  DNS server use UDP protocol and the port is 53, so we just need creat a UDP 
 * server on port 53 then receive and response data.
 * 
 * DNS闁靛棴鎷穜equst data :
 * 
 *  -------------------------------------------------
 *    | Transaction ID | Flags                  |
 * ----------------------------------------------------
 *    |    Questions |Anser RRS            |
 * --------------------------------------------
 * |Authority | Additional RRs|
 * -----------------------------------------------
 * |Queries|
 * ---------------------------------------------------
 * |Answers|
 * ---------------------------------------------------
 * |Au
 * 
*******************************************************************************/
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"
#include "espconn.h"
#include "upgrade.h"
#include "tcpserver.h"
//#include "cJSON.h"
//#include "base64.h"
//#include "esp_spiffs.h"
#include "uart.h"
#include "init.h"
//#include "sysparam.h"


char dns_buf[128];
unsigned char dns_i = 0;
static struct espconn dns_esp_conn;
struct espconn Dns_Client_conn;     //瀵よ櫣鐝涙稉鈧稉鐚爏pconn缂佹挻鐎担锟�
static struct espconn *client_esp_conn;
/******************************************************************************
 * FunctionName : dns_server_recv
 * Description  : Processing the received data from the server
 * Parameters   : arg -- Additional argument to pass to the callback function
 *                pusrdata -- The received data (or NULL when the connection has been closed!)
 *                length -- The length of received data
 * Returns      : none
*******************************************************************************/
void  dns_server_recv(void *arg, char *pusrdata, unsigned short len)
{
	uint32 IP_addr[4] = {0,0,0,0};      //闁烩偓鍔嬫禍鎺斺偓娑櫭崑宥囨喆閿濆棛鈧姤绺介悾灞剧暠IP闁革附婢樺锟�
    uint32 Auto_Web_State = 0;
 #if 0
    DNS_SERVER_DEBUG("dns data:");
    for(dns_i = 0;dns_i < len; dns_i ++)
    {
        DNS_SERVER_DEBUG("%c ", pusrdata[dns_i]);
    }
    DNS_SERVER_DEBUG("\r\n");
#endif
/* Iphone: captive.apple.com/hotspot_detect.html
 * XiaoMI: connect.rom.miui.com/generate_204
 * HuaWei: clients3.google.com/generate_204
 *
*/

    if(strstr(pusrdata+0xc,"taobao")||
      strstr(pusrdata+0xc,"qq")  ||
      //strstr(pusrdata+0xc,"sogou")  ||
      strstr(pusrdata+0xc,"amap")  ||
      strstr(pusrdata+0xc,"alipay") ||
      strstr(pusrdata+0xc,"youku")  ||
      strstr(pusrdata+0xc,"iqiyi")  ||
      (strstr(pusrdata+0xc,"st") )
    		  //&& strstr(pusrdata+0xc,"vivo") )
	  //||strstr(pusrdata+0xc,"baidu")
	  )
    {
        return;
    }

    Spi_FlashRead(IS_AUTO_WEB_Erase,IS_AUTO_WEB_ERASE_OFFSET,&Auto_Web_State,4);
    if(Auto_Web_State == NO_AUTO_ACCESS_WEB)
    {
    	//DNS_SERVER_DEBUG("NO_AUTO_ACCESS_WEB\n");
    	if(strstr(pusrdata+0xc,"www.GX_NTP.com"))
    	{
         // DNS_SERVER_DEBUG("www.GX_NTP.com\n");
    	}
    	else if(strstr(pusrdata+0xc,"GX_NTP"))
    	{
    	//	DNS_SERVER_DEBUG("GX_NTP\n");
    	}
    	else if(strstr(pusrdata+0xc,"www.gx_ntp.com"))
    	{
    	//	DNS_SERVER_DEBUG("www.gx_ntp.com\n");
    	}
    	else if(strstr(pusrdata+0xc,"gx_ntp"))
    	{
    	//	DNS_SERVER_DEBUG("gx_ntp\n");
    	}
    	else
    	{

    	  return;
    	}
    }

    client_esp_conn = arg;
    remot_info* pcon_info = NULL;

	os_memcpy(dns_buf, pusrdata, len);

    espconn_get_connection_info(&dns_esp_conn, &pcon_info, 0);

    client_esp_conn->proto.udp->remote_ip[0] = pcon_info->remote_ip[0];
    client_esp_conn->proto.udp->remote_ip[1] = pcon_info->remote_ip[1];
    client_esp_conn->proto.udp->remote_ip[2] = pcon_info->remote_ip[2];
    client_esp_conn->proto.udp->remote_ip[3] = pcon_info->remote_ip[3];

    client_esp_conn->proto.udp->remote_port =  pcon_info->remote_port;

	//os_printf("remote ip: %d.%d.%d.%d \r\n",pcon_info->remote_ip[0],pcon_info->remote_ip[1],
	//		                                    pcon_info->remote_ip[2],pcon_info->remote_ip[3]);
	//os_printf("remote port: %d \r\n",pcon_info->remote_port);

    dns_buf[2] |= 0x80;
    dns_buf[3] |= 0x80;
    dns_buf[7] =1;

    dns_buf[len++] =0xc0;
    dns_buf[len++] =0x0c;

    dns_buf[len++] =0x00;
    dns_buf[len++] =0x01;
    dns_buf[len++] =0x00;
    dns_buf[len++] =0x01;

    dns_buf[len++] =0x00;
    dns_buf[len++] =0x00;
    dns_buf[len++] =0x00;
    dns_buf[len++] =0x0A;

    dns_buf[len++] =0x00;
    dns_buf[len++] =0x04;
	Spi_FlashRead(LOCAL_Erase,LOCAL_IP_ERASE_OFFSET,IP_addr,4);  //濞寸姴绌砽ash濞戞搩鍙€椤曚即宕ｉ弽銊︽嫳闁革妇妾篜 缂傚啯鍨甸崣锟� 閻庢稒鍔楃紞澶愬箳閳哄啰鍨�
    dns_buf[len++] = (char)IP_addr[0];
    dns_buf[len++] = (char)IP_addr[1];
    dns_buf[len++] = (char)IP_addr[2];
    dns_buf[len++] = (char)IP_addr[3];

    espconn_send(client_esp_conn, dns_buf, len);

    //DNS_SERVER_DEBUG("DNS respons request");
}

ip_addr_t esp_server_ip;
void ICACHE_FLASH_ATTR
user_esp_platform_dns_found(const char *name, ip_addr_t *ipaddr, void *arg)
{
 struct espconn *pespconn = (struct espconn *)arg;
 if (ipaddr != NULL)
 {
	int32_t TcpPort = 0,TcpType;
 	Spi_FlashRead(TCP_SERVERIP_Erase,TCP_SERVER_PORT_ERASE_OFFSET, &TcpPort,1);
	Spi_FlashRead(TCP_SERVERIP_Erase,TCP_SERVER_TPYE_ERASE_OFFSET, &TcpType,1);/* 闁兼儳鍢茶ぐ鍥⒐閼愁垳绠鹃柟鎭掑劥缁绘洟寮伴婊呭彋閺夆晝鍋炵敮锟� */

    //DNS_SERVER_DEBUG("user_esp_platform_dns_found %d.%d.%d.%d\n",
	//	 ipaddr->addr & 0xFF,(ipaddr->addr >> 8) & 0xFF,(ipaddr->addr >> 16) & 0xFF,(ipaddr->addr >> 24) & 0xFF);
         TcpClientToServer[0] =(char) (ipaddr->addr);
         TcpClientToServer[1] =(char) ((ipaddr->addr >> 8) & 0xFF);
         TcpClientToServer[2] =(char) ((ipaddr->addr >> 16) & 0xFF);
         TcpClientToServer[3] =(char) ((ipaddr->addr >> 24) & 0xFF);
         if(TcpType != 'L')
         {
              /* 闁活収鍙€缁绘盯骞掗妷銉ョ仧闁革负鍔嬬憰鍡涘矗閿濆嫯鍘悗鐟版湰閸ㄦ碍娼婚悙鏉戝 */
         }
         else
         {
           AP_tcpclient_init(TcpClientToServer,TcpPort);/* 闂傗偓閼愁垳绠鹃柟鎭掑劚閸垳绮╃€ｎ亜鐓濋弶鈺冨仦鐢拷 */
         }
 }

}



/******************************************************************************
 * FunctionName : user_webserver_init
 * Description  : parameter initialize as a server
 * Parameters   : port -- server port
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR user_dns_server_init()
{
	static esp_udp s_esptcp;

	dns_esp_conn.type = ESPCONN_UDP;
	dns_esp_conn.state = ESPCONN_NONE;
	dns_esp_conn.reverse = NULL;
	dns_esp_conn.proto.udp = &s_esptcp;
	dns_esp_conn.proto.udp->local_port = 53;

    espconn_regist_recvcb(&dns_esp_conn, dns_server_recv);
	espconn_create(&dns_esp_conn);
}
