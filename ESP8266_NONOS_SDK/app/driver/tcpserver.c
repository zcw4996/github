/*
 * tcpserver.c
 *
 *  Created on: 2017年2月8日
 *      Author: dell
 */

#include "tcpserver.h"
#include "mem.h"
#include "smartconfig.h"
#include "init.h"
#include "soft_timer.h"
#include "Rgbled.h"
#include "uart.h"
#include "tcpclient.h"
#include "udpclient.h"
TcpServerPacket Tcp_Packet;
struct espconn esp_conn;   //建立一个espconn结构体
struct espconn esp_conn_App;   //建立一个espconn结构体
esp_tcp esptcp;            //建立一个esp_tcp结构体
esp_tcp esptcp_app;            //建立一个esp_tcp结构体
uint32 NtpappMode = 0;
uint8 DataError;
uint32_t TimeOutInterva = 0;
uint8 FirstLinkOK = 0;
DstPacket DstStart,DstEnd,DstStart_test,DstEnd_test;
DstPacket DstStart_Send,DstEnd_Send;
char TcpClientToServer[4] = {0,0,0,0};
char DnsBuffer[32];
char tempSaveData[FLASH_READ_SIZE];
char HttpHead[200];
 LOCAL os_timer_t sntp_timer_Start;
 extern uint32 IsDst;
extern  uint8_t isLedClose;
extern uint32 SysTime_ms;
/*-------------------------------------------------------------*/
/*函数功能：TCP服务器初始化函数                                                                       */
/*参       数：port：本地端口号                                                                            */
/*返       回：无                                                                                                     */
/*-------------------------------------------------------------*/
void ICACHE_FLASH_ATTR AP_tcpserver_init(u16 port)
{
    esp_conn.type = ESPCONN_TCP;           //espconn的类型是TCP
    esp_conn.state = ESPCONN_NONE;         //状态，初始的时候是空
    esp_conn.proto.tcp = &esptcp;          //因为是TCP类型，所以指向esptcp结构体
    esp_conn.proto.tcp->local_port = port; //记录本地端口号
    espconn_regist_connectcb(&esp_conn, tcp_server_listen);  //注册TCP客户端连接成功后的回调函数
    sint8 ret = espconn_accept(&esp_conn);                   //创建TCP服务器，并侦听
    espconn_regist_time(&esp_conn, 180, 0); //设置超时断开时间 单位：秒，最大值：7200 秒
}
void ICACHE_FLASH_ATTR AP_tcpserverAPP_init(u16 port)
{
	uint8_t i = 0;
	esp_conn_App.type = ESPCONN_TCP;           //espconn的类型是TCP
	esp_conn_App.state = ESPCONN_NONE;         //状态，初始的时候是空
	esp_conn_App.proto.tcp = &esptcp_app;          //因为是TCP类型，所以指向esptcp结构体
	esp_conn_App.proto.tcp->local_port = port; //记录本地端口号
	Tcp_Packet.TcpClientCount = 0;
    espconn_regist_connectcb(&esp_conn_App, tcp_serverApp_listen);  //注册TCP客户端连接成功后的回调函数
    sint8 ret = espconn_accept(&esp_conn_App);                   //创建TCP服务器，并侦听
    espconn_regist_time(&esp_conn_App, 1800, 0); //设置超时断开时间 单位：秒，最大值：7200 秒


}
uint8_t ApInfTemp[1024];
uint8_t ApInfTemp1[1024];
void ICACHE_FLASH_ATTR
scan_done(void *arg, STATUS status)
{
  static uint8 FirstIn;
  os_bzero(ApInfTemp,1024);
  ApInfTemp[0] = '\0';
  if (status == OK)
  {
    struct bss_info *bss_link = (struct bss_info *)arg;

    while (bss_link != NULL)
    {
      if (os_strlen(bss_link->ssid) <= 200)
      {
    	  os_sprintf(ApInfTemp + os_strlen(ApInfTemp),"<option>%s</option>",bss_link->ssid);
      }
      else
      {
      }
      if( os_strlen(ApInfTemp) >= 1000)
      {
    	  break;
      }
      //os_printf("(\"%s\",%d)\r\n",
    		 // bss_link->ssid, bss_link->rssi);
      bss_link = bss_link->next.stqe_next;
    }
    os_memcpy(ApInfTemp1,ApInfTemp, os_strlen(ApInfTemp)); /* 拷贝 AP信息 */
    //os_printf("ApInfTemp Len = %d\n",os_strlen(ApInfTemp));
    ApInfTemp1[os_strlen(ApInfTemp)] = '\0';
    //os_printf("Scan Succeed\n");
  }
  else
  {
	  ApInfTemp[0] = '\0';
     //os_printf("scan fail !!!\r\n");
  }
  if(FirstIn == 0)  /* 开机第一次进入该函数，则联网 */
  {
    Get_APid_Link_AP();   //联网
  }
  FirstIn = 1;
}

void ICACHE_FLASH_ATTR
user_scan(void)
{
   wifi_station_scan(NULL,scan_done);
}
uint32 len = 0;
void ICACHE_FLASH_ATTR SetSendWeb(AllSetData *AlreadySetData1)
{
	uint32 Ap_Erase = 0,i,APname[APNAMEMAXLEN];
	int32 TcpPort = 0;
	uint32 NTPIPLen = 0,TimePutInterva_32 = 0;
	int32 Timezone_32 = 0;
	uint32 NTPIP_32[64];
	uint8 NTPIP[64],TimePutInterva_8[6],Timezone_8[10];
	uint32 TCPServer[32], TcpServerLen = 0;
	uint8 TCPServer1[32];
	uint32 TcpServerPort[6];
	uint32 TcpServerPort1;
	uint32 Language_state = 0;
	uint32 APpasw[64],Pswdlen = 0;
    struct softap_config config;
    struct ip_info dynamic_ipconfig;
	uint32 APNAME_len = 0,APPSWD_len = 0;
	uint8 APname1[APNAMEMAXLEN] ,APpasw1[64];
	uint8 TestBuf[10],TestBuf1[10] ;
	uint8_t DynamicIP[32],DynamicGW[32],DynamicSub[32];
	uint32 DHCPstate = 0,Auto_Web_State = 0;
	uint8_t DHCP_switch1[12],DHCP_switch2[12];
	uint8_t Auto_Web_State1[12],Auto_Web_State2[12];
    uint8_t Tcp_Port[7];
    uint8_t TcpTypeState1[20],TcpTypeState2[20];
    uint8_t TcpClientISOpen[8];
    uint8_t Language_State1[20],Language_State2[20];
    uint8 ganxinNTPshizhongshezhi[30] = {0xe8,0xb5,0xa3,0xe9,0x91,0xab,'N','T','P',0xe6,0x97,0xb6,0xe9,0x92,0x9f,0xe8,0xae,0xbe,0xe7,0xbd,0xae,0};/*赣鑫NTP时钟设置*/
    uint8 wangluomingcheng[20] = {0xe7,0xbd,0x91,0xe7,0xbb,0x9c,0xe5,0x90,0x8d,0xe7,0xa7,0xb0,'\0'};/* 网络名称 */
    uint8 wangluomima[20] = {0xe7,0xbd,0x91,0xe7,0xbb,0x9c,0xe5,0xaf,0x86,0xe7,0xa0,0x81,'\0'};  /*网络密码*/
    uint8 dongtaiIP[20] = {0xe5,0x8a,0xa8,0xe6,0x80,0x81,0};   /*动态 */
    uint8 jingtaiIP[20] = {0xe9,0x9d,0x99,0xe6,0x80,0x81,0};   /*静态 */
    uint8 IPdizhi[20] = {0xe5,0x9c,0xb0,0xe5,0x9d,0x80,0};  /*IP地址 */
    uint8 ziwangyanma[20] = {0xe5,0xad,0x90,0xe7,0xbd,0x91,0xe6,0x8e,0xa9,0xe7,0xa0,0x81};/* 子网掩码 */
    uint8 wangguann[20] = {0xe7,0xbd,0x91,0xe5,0x85,0xb3};/*网关 */
    uint8 luyoushezhi[20] = {0xe8,0xb7,0xaf,0xe7,0x94,0xb1,0xe8,0xae,0xbe,0xe7,0xbd,0xae,'\0'};/*路由设置*/
    uint8 bendiTCPduankou[40] = {0xe6,0x9c,0xac,0xe5,0x9c,0xb0,'T','C','P',0xe6,0x9c,0x8d,0xe5,0x8a,0xa1,0xe5,0x99,0xa8,0xe7,0xab,0xaf,0xe5,0x8f,0xa3,0};/*本地TCP服务器端口 */
    uint8 TCPfuwuqiIP[24] = {'T','C','P',0xe6,0x9c,0x8d,0xe5,0x8a,0xa1,0xe5,0x99,0xa8,'I','P',0};/*TCP服务器IP*/
    uint8 TCPfuwuqiduankou[40] = {'T','C','P',0xe6,0x9c,0x8d,0xe5,0x8a,0xa1,0xe5,0x99,0xa8,0xe7,0xab,0xaf,0xe5,0x8f,0xa3,0};/*TCP服务器端口*/
    uint8 NTPwangluoshezhi[24] ={'N','T','P',0xe7,0xbd,0x91,0xe7,0xbb,0x9c,0xe8,0xae,0xbe,0xe7,0xbd,0xae,0};/*NTP网络设置 */
    uint8 NTPfuwuqi[20] = {'N','T','P',0xe6,0x9c,0x8d,0xe5,0x8a,0xa1,0xe5,0x99,0xa8,0}; /*NTP服务器*/
    uint8 chushushijianjiange[30] = {0xe8,0xbe,0x93,0xe5,0x87,0xba,0xe6,0x97,0xb6,0xe9,0x97,0xb4,0xe9,0x97,0xb4,0xe9,0x9a,0x94,0};/*输出时间间隔*/
    uint8 shiqu[20] = {0xe6,0x97,0xb6,0xe5,0x8c,0xba,0}; /*时区*/
    uint8 xialingshishezhi[30] = {0xe5,0xa4,0x8f,0xe4,0xbb,0xa4,0xe6,0x97,0xb6,0xe8,0xae,0xbe,0xe7,0xbd,0xae,0};/*夏令时设置*/
    uint8 xialingshikaishixiaoshi[40] = {0xe5,0xa4,0x8f,0xe4,0xbb,0xa4,0xe6,0x97,0xb6,0xe5,0xbc,0x80,0xe5,0xa7,0x8b,0xe5,0xb0,0x8f,0xe6,0x97,0xb6,0};/*夏令时开始小时*/
    uint8 xialingkaishiriqi[40] = {0xe5,0xa4,0x8f,0xe4,0xbb,0xa4,0xe6,0x97,0xb6,0xe5,0xbc,0x80,0xe5,0xa7,0x8b,0xe6,0x97,0xa5,0xe6,0x9c,0x9f,0};/*夏令时开始日期*/
    uint8 xialingshijieshuxiaoshi[40] = {0xe5,0xa4,0x8f,0xe4,0xbb,0xa4,0xe6,0x97,0xb6,0xe7,0xbb,0x93,0xe6,0x9d,0x9f,0xe5,0xb0,0x8f,0xe6,0x97,0xb6,0};/*夏令时结束小时*/
    uint8 xialingshijieshuriqi[40] = {0xe5,0xa4,0x8f,0xe4,0xbb,0xa4,0xe6,0x97,0xb6,0xe7,0xbb,0x93,0xe6,0x9d,0x9f,0xe6,0x97,0xa5,0xe6,0x9c,0x9f,0};/*夏令时结束日期*/
    uint8 shijianWIFIshezhi[30] = {0xe6,0x97,0xb6,0xe9,0x92,0x9f,'w','i','f','i',0xe8,0xae,0xbe,0xe7,0xbd,0xae,0};/*时钟WIFI设置*/

    uint8 WIFImingcheng[20] = {'W','I','F','I',0xe5,0x90,0x8d,0xe7,0xa7,0xb0,0};/*WIFI名称*/
    uint8 WIFImima[20] = {'W','I','F','I',0xe5,0xaf,0x86,0xe7,0xa0,0x81,0};/*WIFI密码*/
    uint8 zidongfangwenwangye[30] = {0xe8,0x87,0xaa,0xe5,0x8a,0xa8,0xe8,0xae,0xbf,0xe9,0x97,0xae,0xe7,0xbd,0x91,0xe9,0xa1,0xb5,0};/*自动访问网页*/
    uint8 shi[10] = {0xe6,0x98,0xaf};/*是*/
    uint8 fou[10] = {0xe5,0x90,0xa6};/*否*/
    uint8 yuyanxuanze[24] = {0xe8,0xaf,0xad,0xe8,0xa8,0x80,0xe9,0x80,0x89,0xe6,0x8b,0xa9,0}; /* 语言选择 */
    uint8 zhongwen[10] = {0xe4,0xb8,0xad,0xe6,0x96,0x87,0}; /*中文 */
	 os_bzero(Timezone_8,10);
     os_bzero(TimePutInterva_8,6);
	 os_bzero(APname1,APNAMEMAXLEN);
	 os_bzero(APpasw1,64);
	 os_bzero(DynamicIP,32);
	 os_bzero(DynamicGW,32);
	 os_bzero(DynamicSub,32);
	 os_bzero(DHCP_switch1,12);
	 os_bzero(DHCP_switch2,12);
	 os_bzero(Auto_Web_State1,12);
	 os_bzero(Auto_Web_State2,12);
	 os_bzero(Tcp_Port,7);
	 os_bzero(TCPServer,32);
	 os_bzero(TCPServer1,32);
	 os_bzero(TcpTypeState1,20);
	 os_bzero(TcpTypeState2,20);
	 os_bzero(TcpClientISOpen,8);
	 Spi_FlashRead(TCP_SERVERIP_Erase,TCP_SERVERIPLen_ERASE_OFFSET,&TcpServerLen,1);
	 if(TcpServerLen < 4)
	 {
		 os_sprintf(TcpClientISOpen,"");
		 os_sprintf(TcpTypeState1,"selected= \"selected\"");
		 os_sprintf(TcpTypeState2,"");
		 os_sprintf(TCPServer1,"");
		 os_sprintf(TcpServerPort,"");
	 }
	 else
	 {
		 os_sprintf(TcpClientISOpen,"checked");
		 if(TcpServerLen == 4)/* 代表是IP地址 */
		 {
			 Spi_FlashRead(TCP_SERVERIP_Erase,TCP_SERVERIP_ERASE_OFFSET,TCPServer,TcpServerLen);/*读取TCPIP数据*/
			 os_sprintf(TCPServer1,"%d.%d.%d.%d",TCPServer[0],TCPServer[1],TCPServer[2],TCPServer[3]);
		 }
		 else if((TcpServerLen > 4) && (TcpServerLen < 32))
		 {
			 uint8 i = 0;
			 Spi_FlashRead(TCP_SERVERIP_Erase,TCP_SERVERIP_ERASE_OFFSET,TCPServer,TcpServerLen);/*读取TCPIP数据*/
			 for(i = 0; i < TcpServerLen; i ++)
			 {
				 TCPServer1[i] = (char)TCPServer[i];
			 }
		 }
		 Spi_FlashRead(TCP_SERVERIP_Erase,TCP_SERVER_PORT_ERASE_OFFSET, &TcpServerPort1,1);/* 读取TCP 端口 */
		 os_sprintf(TcpServerPort,"%d",TcpServerPort1);
		 Spi_FlashRead(TCP_SERVERIP_Erase,TCP_SERVER_TPYE_ERASE_OFFSET, &TcpServerPort1,1);/* 读取TCP 端口 */
		 if(TcpServerPort1 != 'L')
		 {
			 os_sprintf(TcpTypeState1,"selected= \"selected\"");
			 os_sprintf(TcpTypeState2,"");
		 }
		 else
		 {
			 os_sprintf(TcpTypeState2,"selected= \"selected\"");
			 os_sprintf(TcpTypeState1,"");
		 }

	 }
	 Spi_FlashRead(TCP_PORT_Erase,TCP_PORT_ERASE_OFFSET,&TcpPort,1);
	 if(TcpPort < 0)
	 {
		 TcpPort = 50000;
		 spi_flash_erase_sector (TCP_PORT_Erase);  // /* 默认存储50000端口号*/
		 spi_flash_write (TCP_PORT_Erase*4*1024 + TCP_PORT_ERASE_OFFSET, &TcpPort, 1 * 4);
	 	 os_sprintf(Tcp_Port,"50000");
	 }
	 else
	 {
		 os_sprintf(Tcp_Port,"%d",TcpPort);
	 }
     Spi_FlashRead(AP_NUM_Erase,AP_NUM_ERASE_OFFSET,&AlreadySetData1-> AP_num,1);/*获取已记录的AP个数 */
	 Spi_FlashRead(IS_AUTO_WEB_Erase,IS_AUTO_WEB_ERASE_OFFSET,&Auto_Web_State,4); /*获取是否自动弹出网页设置 */
	 if(Auto_Web_State != NO_AUTO_ACCESS_WEB)
	 {
	      os_sprintf(Auto_Web_State1,"checked");
	      os_sprintf(Auto_Web_State2,"");
	 }
	 else
	 {
		  os_sprintf(Auto_Web_State1,"");
		  os_sprintf(Auto_Web_State2,"checked");
	 }
     Spi_FlashRead(NO_DHCP_Erase,DHCP_OPEN_ERASE_OFFSET,&DHCPstate,1);
	 if(DHCPstate != DHCP_CLOSE)
	 {
	      os_sprintf(DHCP_switch1,"checked");
	      os_sprintf(DHCP_switch2,"");
	 }
	 else
	 {
		  os_sprintf(DHCP_switch1,"");
		  os_sprintf(DHCP_switch2,"checked");
	 }
    switch(AlreadySetData1 -> AP_num)
    {
	  case APID1: Ap_Erase = AP_NAME_PSWD_Erase;break;
	  case APID2: Ap_Erase = AP1_NAME_PSWD_Erase;break;
	  default:APname1[0] = '\0';
	          APpasw1[0] = '\0';
	           break;
    }
    //os_printf("AlreadySetData1-> AP_num = %d\n",AlreadySetData1-> AP_num);
    if((AlreadySetData1-> AP_num == APID1) || (AlreadySetData1-> AP_num == APID2))
    	{
    		 Spi_FlashRead(Ap_Erase,AP_NAME_LEN_ERASE_OFFSET,&APNAME_len,1);  //从FLASH中获取AP名称长度和密码长度
    		 Spi_FlashRead(Ap_Erase,AP_PSWD_LEN_ERASE_OFFSET,&APPSWD_len,1);
    		// os_printf("\n");
    		 if((APNAME_len <= APNAMEMAXLEN) &&(APNAME_len > 0) &&(APPSWD_len <= 64) && (APPSWD_len >= 0))
    		 {
    			//os_printf("APNAME_len = %d\nAPPSWD_len = %d\n",APNAME_len,APPSWD_len);
    			Spi_FlashRead(Ap_Erase,AP_NAME_ERASE_OFFSET,(uint32 *)APname,APNAME_len);  //从FLASH中获取AP名称和密码
    			Spi_FlashRead(Ap_Erase,AP_PSWD_ERASE_OFFSET,(uint32 *)APpasw,APPSWD_len);

    			 for(i = 0; i < APNAME_len; i ++)
    			  {
    				APname1[i] = (uint8 )APname[i];
    				//uart0_tx_buffer(APname1,APNAME_len);
    				//os_printf("%c",APname1[i]);
    			  }
    			 APname1[i] = '\0';
    			  //os_printf("\n");
    			 for(i = 0; i < APPSWD_len; i ++)
    			  {
    				APpasw1[i] = (uint8 )APpasw[i];
    				//os_printf("%c",APpasw1[i]);
    			  }
    			 APpasw1[i] = '\0';

    		 }
    		 else
    		 {
    			 APname1[0] = '\0';
    			 APpasw1[0] = '\0';
    		 }
    	}

	wifi_get_ip_info(STATION_IF, &dynamic_ipconfig);  //获取动态IP
	os_sprintf(DynamicIP,"%d.%d.%d.%d",(dynamic_ipconfig.ip.addr & 0xFF),(dynamic_ipconfig.ip.addr  >> 8 & 0xFF),
			(dynamic_ipconfig.ip.addr  >> 16 & 0xFF),((dynamic_ipconfig.ip.addr  >> 24) & 0xFF));
	os_sprintf(DynamicGW,"%d.%d.%d.%d",(dynamic_ipconfig.gw.addr & 0xFF),(dynamic_ipconfig.gw.addr  >> 8 & 0xFF),
			(dynamic_ipconfig.gw.addr  >> 16 & 0xFF),((dynamic_ipconfig.gw.addr  >> 24) & 0xFF));
	os_sprintf(DynamicSub,"%d.%d.%d.%d",(dynamic_ipconfig.netmask.addr & 0xFF),(dynamic_ipconfig.netmask.addr  >> 8 & 0xFF),
			(dynamic_ipconfig.netmask.addr  >> 16 & 0xFF),((dynamic_ipconfig.netmask.addr  >> 24) & 0xFF));

	 Spi_FlashRead(NTP_IP_Erase,NTP_IP_LEN_ERASE_OFFSET,&NTPIPLen,1); /*获取NTP IP的长度 */
	 if(NTPIPLen <= 0 || NTPIPLen > 64)
	 {
		 NTPIP[0] = '\0';
	 }
	 else
	 {    /* 若长度合适 则有NTPIP的数据 读取出来 */
		 Spi_FlashRead(NTP_IP_Erase,NTP_IP_ERASE_OFFSET,NTPIP_32,NTPIPLen);
		 for(i = 0 ; i < NTPIPLen; i ++)
		 {
			 NTPIP[i] = (char)NTPIP_32[i];
		 }
		 NTPIP[NTPIPLen] = '\0';
	 }
    wifi_softap_get_config(&config);       //获取参数
    //os_printf("password = %s\n",config.password);
	//os_printf("config.ssid = %s\n",(config.ssid));
	Spi_FlashRead(LOCALSSID_Erase,PSWD_LEN_ERASE_OFFSET,&Pswdlen,1); /* 获取密码长度 */
     if(Pswdlen <= 0)
     {
    	 os_bzero(config.password,64);
     }
	Spi_FlashRead(TIME_Interva_ERASE,TIME_Interva_ERASE_OFFSET,&TimePutInterva_32,1);
	if(TimePutInterva_32 <= 1000 || TimePutInterva_32 > 0)
	{
      os_sprintf(TimePutInterva_8,"%d",TimePutInterva_32);
	}
	else
	{
		TimePutInterva_8[0] = '\0';
	}

	Spi_FlashRead(TIMEZONE_ERASE,TIMEZONE_ERASE_OFFSET,&(AlreadySetData1->Timezone),1);
    if(AlreadySetData1 -> Timezone < 0)
    {
    	uint8 TimezoneLen = 0;
    	AlreadySetData1 -> Timezone = -AlreadySetData1 -> Timezone;
    	Timezone_8[0] = '-';
    	TimezoneLen = os_sprintf((Timezone_8 + 1),"%d",(AlreadySetData1 -> Timezone / 100));
    	AlreadySetData1 -> Timezone = AlreadySetData1 -> Timezone % 100;
    	AlreadySetData1 -> Timezone = AlreadySetData1 -> Timezone * 60 / 100;
    	os_sprintf((Timezone_8 + 1 + TimezoneLen),":%d%d",AlreadySetData1 -> Timezone / 10,AlreadySetData1 -> Timezone % 10);
    	//os_printf("Timezone = %s\n",Timezone_8);
    }
    else
    {
    	uint8 TimezoneLen = 0;
    	Timezone_8[0] = 0x2b;
    	TimezoneLen = os_sprintf((Timezone_8 + 1),"%d",(AlreadySetData1 -> Timezone / 100));
    	AlreadySetData1 -> Timezone = AlreadySetData1 -> Timezone % 100;
    	AlreadySetData1 -> Timezone = AlreadySetData1 -> Timezone * 60 / 100;
    	os_sprintf((Timezone_8  + 1 + TimezoneLen),":%d%d",AlreadySetData1 -> Timezone / 10,AlreadySetData1 -> Timezone % 10);
    	//os_printf("Timezone1 = %s\n",Timezone_8);
    }

    Spi_FlashRead(DST_Erase,ISSET_DST_ERASE_OFFSET,&(AlreadySetData1->IsSetDST),1);/* 读取夏令时数据 */

    Spi_FlashRead(LANGUAGE_Erase,LANGUAGE_ERASE_OFFSET,&Language_state,1);/* 语言版本 */
    if(Language_state != LANGUAGE_CH)
    {
    	os_sprintf(Language_State1,"selected= \"selected\"");
    	os_sprintf(Language_State2,"");
    }
    else
    {
    	os_sprintf(Language_State2,"selected= \"selected\"");
    	os_sprintf(Language_State1,"");
    }
    if(AlreadySetData1->IsSetDST == DST_NO_SET) /* 代表未设置过夏令时 */
    {

    	if(Language_state != LANGUAGE_CH)
    	{
   		 len = os_sprintf(tempSaveData,
   				      HTML,APname1,ApInfTemp1,APpasw1
					 ,DHCP_switch1,\
				      DHCP_switch2,DynamicIP,DynamicSub,DynamicGW
				     ,Tcp_Port,TCPServer1,\
				     TcpClientISOpen,TcpServerPort,
				     TcpTypeState1,TcpTypeState2
				     ,NTPIP,TimePutInterva_8,\
				      Timezone_8,"0","1st","Sun","Jan",\
				     "0","1st","Sun","Jan"
				     ,(config.ssid),(config.password),
			          Auto_Web_State1,Auto_Web_State2,Language_State1,Language_State2,zhongwen);
    	}
    	else
    	{
    		 len = os_sprintf(tempSaveData,
    				 HTML_CH,ganxinNTPshizhongshezhi,
					 luyoushezhi,wangluomingcheng,APname1,ApInfTemp1,wangluomima,APpasw1,
					 dongtaiIP,DHCP_switch1,jingtaiIP,\
				DHCP_switch2,IPdizhi,DynamicIP,ziwangyanma,DynamicSub,wangguann,DynamicGW,
				bendiTCPduankou,Tcp_Port,TCPfuwuqiIP,TCPServer1,\
				TcpClientISOpen,TCPfuwuqiduankou,TcpServerPort,
				TcpTypeState1,TcpTypeState2,
				NTPwangluoshezhi,NTPfuwuqi,NTPIP,chushushijianjiange,TimePutInterva_8,\
				shiqu,Timezone_8,xialingshishezhi,xialingshikaishixiaoshi,"0",xialingkaishiriqi,"1st","Sun","Jan",xialingshijieshuxiaoshi,\
				"0",xialingshijieshuriqi,"1st","Sun","Jan",
				shijianWIFIshezhi,WIFImingcheng,(config.ssid),WIFImima,(config.password),
				zidongfangwenwangye,shi,Auto_Web_State1,fou,Auto_Web_State2,yuyanxuanze,Language_State1,Language_State2,zhongwen);
    	}
    }
    else
    {
    	uint8 DstStartHour[5],DstStartWeekSel[5],DstStartWeek[5],DstStartMon[5];
    	uint8 DstEndHour[5],DstEndWeekSel[5],DstEndWeek[5],DstEndMon[5];
    	uint8 Dst_Hour_8 = 0;
    	/* 清空字符串 */
    	 os_bzero(DstStartHour,5); os_bzero(DstStartWeekSel,5); os_bzero(DstStartWeek,5);os_bzero(DstStartMon,5);
    	 os_bzero(DstEndHour,5); os_bzero(DstEndWeekSel,5); os_bzero(DstEndWeek,5);os_bzero(DstEndMon,5);

    	Spi_FlashRead(DST_Erase,DSTSTART_HOUR_ERASE_OFFSET,&DstStart_Send.Dst_Hour,1);
    	Spi_FlashRead(DST_Erase,DSTSTART_SELE_WEEK_ERASE_OFFSET,&DstStart_Send.Dst_WeekSel,1);
    	Spi_FlashRead(DST_Erase,DSTSTART_WEEK_ERASE_OFFSET,&DstStart_Send.Dst_Week,1);
    	Spi_FlashRead(DST_Erase,DSTSTART_MON_ERASE_OFFSET,&DstStart_Send.Dst_Mon,1);

    	Spi_FlashRead(DST_Erase,DSTEND_HOUR_ERASE_OFFSET,&DstEnd_Send.Dst_Hour,1);
    	Spi_FlashRead(DST_Erase,DSTEND_SELE_WEEK_ERASE_OFFSET,&DstEnd_Send.Dst_WeekSel,1);
    	Spi_FlashRead(DST_Erase,DSTEND_WEEK_ERASE_OFFSET,&DstEnd_Send.Dst_Week,1);
    	Spi_FlashRead(DST_Erase,DSTEND_MON_ERASE_OFFSET,&DstEnd_Send.Dst_Mon,1);

    	Dst_Hour_8 = (uint8)DstStart_Send.Dst_Hour;  /* 把夏令时数据以字符串的形式存放着 */
    	os_sprintf(DstStartHour,"%d",Dst_Hour_8);

    	Dst_Hour_8 = (uint8)DstEnd_Send.Dst_Hour;
    	os_sprintf(DstEndHour,"%d",Dst_Hour_8);
    	switch(DstStart_Send.Dst_WeekSel)
    	{
    	  case 1:os_sprintf(DstStartWeekSel,"1st");break;
    	  case 2:os_sprintf(DstStartWeekSel,"2nd");break;
    	  case 3:os_sprintf(DstStartWeekSel,"3rd");break;
    	  case 4:os_sprintf(DstStartWeekSel,"4th");break;
    	  case 5:os_sprintf(DstStartWeekSel,"5th");break;
    	  case LASTDST_WEEK:os_sprintf(DstStartWeekSel,"last");break;
    	  default:os_sprintf(DstStartWeekSel,"1st");break;
    	}
    	switch(DstEnd_Send.Dst_WeekSel)
    	{
    	  case 1:os_sprintf(DstEndWeekSel,"1st");break;
    	  case 2:os_sprintf(DstEndWeekSel,"2nd");break;
    	  case 3:os_sprintf(DstEndWeekSel,"3rd");break;
    	  case 4:os_sprintf(DstEndWeekSel,"4th");break;
    	  case 5:os_sprintf(DstEndWeekSel,"5th");break;
    	  case LASTDST_WEEK:os_sprintf(DstEndWeekSel,"last");break;
    	  default:os_sprintf(DstEndWeekSel,"1st");break;
    	}
    	switch(DstStart_Send.Dst_Week)
    	{
    	  case 0:os_sprintf(DstStartWeek,"Sun");break;
    	  case 1:os_sprintf(DstStartWeek,"Mon");break;
    	  case 2:os_sprintf(DstStartWeek,"Tue");break;
    	  case 3:os_sprintf(DstStartWeek,"Wed");break;
    	  case 4:os_sprintf(DstStartWeek,"Thu");break;
    	  case 5:os_sprintf(DstStartWeek,"Fri");break;
    	  case 6:os_sprintf(DstStartWeek,"Sat");break;
    	  default:os_sprintf(DstStartWeek,"Sun");break;
    	}
    	switch(DstEnd_Send.Dst_Week)
    	{
    	  case 0:os_sprintf(DstEndWeek,"Sun");break;
    	  case 1:os_sprintf(DstEndWeek,"Mon");break;
    	  case 2:os_sprintf(DstEndWeek,"Tue");break;
    	  case 3:os_sprintf(DstEndWeek,"Wed");break;
    	  case 4:os_sprintf(DstEndWeek,"Thu");break;
    	  case 5:os_sprintf(DstEndWeek,"Fri");break;
    	  case 6:os_sprintf(DstEndWeek,"Sat");break;
    	  default:os_sprintf(DstEndWeek,"Sun");break;
    	}
    	switch(DstStart_Send.Dst_Mon)
    	{
    	 case 1:os_sprintf(DstStartMon,"Jan");break;
    	 case 2:os_sprintf(DstStartMon,"Feb");break;
    	 case 3:os_sprintf(DstStartMon,"Mar");break;
    	 case 4:os_sprintf(DstStartMon,"Apr");break;
    	 case 5:os_sprintf(DstStartMon,"May");break;
    	 case 6:os_sprintf(DstStartMon,"Jun");break;
    	 case 7:os_sprintf(DstStartMon,"Jul");break;
    	 case 8:os_sprintf(DstStartMon,"Aug");break;
    	 case 9:os_sprintf(DstStartMon,"Sep");break;
    	 case 10:os_sprintf(DstStartMon,"Oct");break;
    	 case 11:os_sprintf(DstStartMon,"Nov");break;
    	 case 12:os_sprintf(DstStartMon,"Dec");break;
    	 default:os_sprintf(DstStartMon,"Jan");break;
    	}
    	switch(DstEnd_Send.Dst_Mon)
    	{
    	 case 1:os_sprintf(DstEndMon,"Jan");break;
    	 case 2:os_sprintf(DstEndMon,"Feb");break;
    	 case 3:os_sprintf(DstEndMon,"Mar");break;
    	 case 4:os_sprintf(DstEndMon,"Apr");break;
    	 case 5:os_sprintf(DstEndMon,"May");break;
    	 case 6:os_sprintf(DstEndMon,"Jun");break;
    	 case 7:os_sprintf(DstEndMon,"Jul");break;
    	 case 8:os_sprintf(DstEndMon,"Aug");break;
    	 case 9:os_sprintf(DstEndMon,"Sep");break;
    	 case 10:os_sprintf(DstEndMon,"Oct");break;
    	 case 11:os_sprintf(DstEndMon,"Nov");break;
    	 case 12:os_sprintf(DstEndMon,"Dec");break;
    	 default:os_sprintf(DstEndMon,"Jan");break;
    	}

    	if(Language_state != LANGUAGE_CH)
    	{

   		 len = os_sprintf(tempSaveData,
   				      HTML,
					  APname1,ApInfTemp1,APpasw1
					 ,DHCP_switch1,DHCP_switch2,DynamicIP,DynamicSub,DynamicGW,Tcp_Port,TCPServer1,\
				     TcpClientISOpen,TcpServerPort,
				     TcpTypeState1,TcpTypeState2
				     ,NTPIP,TimePutInterva_8,\
				      Timezone_8,
 						DstStartHour,DstStartWeekSel,DstStartWeek,DstStartMon,
 						DstEndHour,DstEndWeekSel,DstEndWeek,DstEndMon
				     ,(config.ssid),(config.password),Auto_Web_State1,Auto_Web_State2,Language_State1,Language_State2,zhongwen);
    	}
    	else
    	{
    		 len = os_sprintf(tempSaveData,
    				 HTML_CH,ganxinNTPshizhongshezhi,
					 luyoushezhi,wangluomingcheng,APname1,ApInfTemp1,wangluomima,APpasw1,
					 dongtaiIP,DHCP_switch1,jingtaiIP,\
				DHCP_switch2,IPdizhi,DynamicIP,ziwangyanma,DynamicSub,wangguann,DynamicGW,
				bendiTCPduankou,Tcp_Port,TCPfuwuqiIP,TCPServer1,\
				TcpClientISOpen,TCPfuwuqiduankou,TcpServerPort,
				TcpTypeState1,TcpTypeState2,
				NTPwangluoshezhi,NTPfuwuqi,NTPIP,chushushijianjiange,TimePutInterva_8,\
				shiqu,Timezone_8,xialingshishezhi,xialingshikaishixiaoshi,DstStartHour,xialingkaishiriqi,DstStartWeekSel,DstStartWeek,DstStartMon,xialingshijieshuxiaoshi,\
				DstEndHour,xialingshijieshuriqi,DstEndWeekSel,DstEndWeek,DstEndMon,
				shijianWIFIshezhi,WIFImingcheng,(config.ssid),WIFImima,(config.password),
				zidongfangwenwangye,shi,Auto_Web_State1,fou,Auto_Web_State2,yuyanxuanze,Language_State1,Language_State2,zhongwen);
    	}
    }

}

AllSetData AlreadySetData;
void ICACHE_FLASH_ATTR
webserver_sent_cb(void *arg)
{
	uint32 i = 0;
	struct espconn *pesp_conn = arg;
	if(pesp_conn->user_reverse == 1)
	{
	   pesp_conn->user_reverse = 0;
	   //os_printf("len = %d\n",len);
	   espconn_sent(pesp_conn, (char*)tempSaveData, len);

	}
	else
	{
	 espconn_disconnect(pesp_conn);
	}


}
/*-------------------------------------------------------------*/
/*函数功能：TCP客户端连接成功后的回调函数                                                     */
/*参       数：arg：指向espconn的指针                                                                 */
/*返       回：无                                                                                                     */
/*-------------------------------------------------------------*/
 void ICACHE_FLASH_ATTR tcp_server_listen(void *arg)
{
    struct espconn *pesp_conn = arg;           //得到espconn
    pesp_conn -> user_reverse = 0;
    //os_printf("There is client access\r\n");   //提示有客户端接入
    //espconn_sent(pesp_conn, "<meta HTTP-EQUIV='REFRESH' content='0; url=http://192.168.1.100/'>",strlen("<meta HTTP-EQUIV='REFRESH' content='0; url=http://192.168.1.100/'>"));
 	espconn_regist_recvcb(pesp_conn, tcp_server_recv_cb);      //注册接收到数据的回调函数
    espconn_regist_reconcb(pesp_conn, tcp_server_recon_cb);    //注册连接异常断开的回调函数
    espconn_regist_disconcb(pesp_conn, tcp_server_discon_cb);  //注册连接正常断开的回调函数
	espconn_regist_sentcb(pesp_conn,   webserver_sent_cb);     //注册数据发送成功后的回调函数

}
void ICACHE_FLASH_ATTR tcp_serverApp_listen(void *arg)
{
    struct espconn *pesp_conn = arg;           //得到espconn
    //os_printf("There is App client access\r\n");   //提示有客户端接入
    Tcp_Packet.TcpClientCount ++;
 	espconn_regist_recvcb(pesp_conn, tcp_server_recvApp_cb);      //注册接收到数据的回调函数
    espconn_regist_reconcb(pesp_conn, tcp_Appserver_recon_cb);    //注册连接异常断开的回调函数
    espconn_regist_disconcb(pesp_conn, tcp_Appserver_discon_cb);  //注册连接正常断开的回调函数

}
void ICACHE_FLASH_ATTR ChangeTcpPort(char* TcpPortPoint,uint32 Len)
{
	char TcpPortTemp[10];
	int32_t TcpPort = 0,TcpPort1 = 0;

	os_memcpy(TcpPortTemp,TcpPortPoint, Len); /* 拷贝信息 */

	TcpPortTemp[Len] = '\0';

	TcpPort = atoi(TcpPortTemp);

  if(Tcp_Packet.TcpClientCount <= 0)
  {
	  Spi_FlashRead(TCP_PORT_Erase,TCP_PORT_ERASE_OFFSET,&TcpPort1,1);
	  if((TcpPort == TcpPort1) || (TcpPort > 65535) || (TcpPort <= 0))
	  {
		  return;
	  }
	  //espconn_disconnect(&esp_conn_App);
	  if(espconn_delete(&esp_conn_App) == 0)
	  {
		  esp_conn_App.proto.tcp->local_port = TcpPort; //记录本地端口号
		  Tcp_Packet.TcpClientCount = 0;
		  espconn_regist_connectcb(&esp_conn_App, tcp_serverApp_listen);  //注册TCP客户端连接成功后的回调函数
		  if(espconn_accept(&esp_conn_App) == 0)               //创建TCP服务器，并侦听
		  {
		    espconn_regist_time(&esp_conn_App, 1800, 0); //设置超时断开时间 单位：秒，最大值：7200 秒
		    spi_flash_erase_sector (TCP_PORT_Erase);  // /* 默认存储50000端口号*/
		    spi_flash_write (TCP_PORT_Erase*4*1024 + TCP_PORT_ERASE_OFFSET, &TcpPort, 1 * 4);
		  }
	  }


  }
  else
  {
	  //os_printf("Tcp_Packet.TcpClientCount = %d\n",Tcp_Packet.TcpClientCount);
  }
}
/*-------------------------------------------------------------*/
/*函数功能：连接异常断开的回调函数                                                                   */
/*参       数：arg：指向espconn的指针                                                                 */
/*参       数：err：错误代码                                                                                  */
/*返       回：无                                                                                                     */
/*-------------------------------------------------------------*/
 void ICACHE_FLASH_ATTR tcp_server_recon_cb(void *arg, sint8 err)
{
    //os_printf("App Connection aborted, error code %d !!! \r\n",err);   //提示连接是异常断开的

}
 /*-------------------------------------------------------------*/
 /*函数功能：连接异常断开的回调函数                                                                   */
 /*参       数：arg：指向espconn的指针                                                                 */
 /*参       数：err：错误代码                                                                                  */
 /*返       回：无                                                                                                     */
 /*-------------------------------------------------------------*/
  void ICACHE_FLASH_ATTR tcp_Appserver_recon_cb(void *arg, sint8 err)
 {
     //os_printf("App Connection aborted, error code %d !!! \r\n",err);   //提示连接是异常断开的
     Tcp_Packet.TcpClientCount --;
     if(Tcp_Packet.TcpClientCount <= 0)
     {
    	 Tcp_Packet.TcpClientCount = 0;
     }
     //os_printf("Tcp_Packet.TcpClientCount = %d\n",Tcp_Packet.TcpClientCount);
 }
/*-------------------------------------------------------------*/
/*函数功能：连接正常断开的回调函数                                                                   */
/*参       数：arg：指向espconn的指针                                                                 */
/*返       回：无                                                                                                     */
/*-------------------------------------------------------------*/
 void ICACHE_FLASH_ATTR tcp_server_discon_cb(void *arg)
{
     //os_printf("App The connection is normally disconnected\r\n");       //提示连接正常断开
}
 /*-------------------------------------------------------------*/
 /*函数功能：连接正常断开的回调函数                                                                   */
 /*参       数：arg：指向espconn的指针                                                                 */
 /*返       回：无                                                                                                     */
 /*-------------------------------------------------------------*/
  void ICACHE_FLASH_ATTR tcp_Appserver_discon_cb(void *arg)
 {
      //os_printf("App The connection is normally disconnected\r\n");       //提示连接正常断开
      Tcp_Packet.TcpClientCount --;
      if(Tcp_Packet.TcpClientCount <= 0)
      {
     	 Tcp_Packet.TcpClientCount = 0;
      }
      //os_printf("Tcp_Packet.TcpClientCount = %d\n",Tcp_Packet.TcpClientCount);
 }
uint8 ICACHE_FLASH_ATTR IsIpaddr(char *ptr,uint32 len,uint8 Type)
{
	char i,s1[32];
	uint32 PointNum,a = 0,b = 0,c = 0,d = 0;
	if(len <= 0)
	{
		return 0;
	}
	for(i = 0; i < len; i ++)
	{
        s1[i] = ptr[i];
	}
     s1[len] = '\0';
    if(s1[2] == '.' || s1[2] == '0')
    {
    	//return 0;
    }
    PointNum = 0;//记录小数点的数量
	for(i = 0; i < len; i ++)
	{
		if(s1[i] == '.')
		{
			PointNum ++;
			if(s1[i + 1] == '.' || s1[i + 1] == '\0' )
			{
		    	return 0;
			}
		}
		else
		{
			if(s1[i] >= '0' && s1[i] <= '9')
			{
				switch(PointNum)
				{
				case 0:a = a * 10;
		               a = (uint32)((uint32)s1[i] - 0x30 + a);
				       break;
				case 1:b = b * 10;
	                   b = (uint32)((uint32)s1[i] - 0x30 + b);
			           break;
				case 2:c = c * 10;
	                   c = (uint32)((uint32)s1[i] - 0x30 + c);
			           break;
				case 3:d = d * 10;
	                   d = (uint32)((uint32)s1[i] - 0x30 + d);
			           break;
				default:return 0;
				}
			}
			else
			{
				return 0;
			}
		}
	}
	switch(Type)
	{
	case IPTYPE:if((a >= 0) && (a<= 255) && (b >= 0) && (b<= 255) && (c >= 0) && (c<= 255) && (d >= 0) && (d<= 255) && (PointNum == 3) )
					{
		                if((a == 0) && (b == 0) &&(c == 0)&&(d == 0))
		                {
		                	return 0;
		                }
		                else if((a == 255) && (b == 255) &&(c == 255)&&(d == 255))
		                {
		                	return 0;
		                }
						return 1;
					}
					else
					{
						return 0;
					}
	                 break;
	case SUBTYPE:if(PointNum == 3)
					{
		               if(((a == 0 || a == 255)) && ((b == 0 || b == 255))&& ((c == 0 || c == 255))&& ((d == 0 || d == 255)) )
		               {
		            	   if((a == 0) && (b == 0) && (c == 0) && (d == 0))//不能全为0
		            	   {
		            		   return 0;
		            	   }
		            	   else if((a == 255) && (b == 255) && (c == 255) && (d == 255))//不能全为255
		            	   {
		            		   return 0;
		            	   }

                          if((b>a) || (c>b) || (d>c) || (d>b) || (d>a)|| (c>a))
                          {
                        	  return 0;
                          }
                          return 1;
		               }
		               else
		               {
						return 0;
		               }
					}
					else
					{
						return 0;
					}
	                 break;
	case GATEWAYTYPE:if((a >= 0) && (a<= 255) && (b >= 0) && (b<= 255) && (c >= 0) && (c<= 255) && (d >= 0) && (d<= 255) && (PointNum == 3) )
					{
					   if((a == 0) && (b == 0) && (c == 0) && (d == 0))//不能全为0
					   {
						   return 0;
					   }
	            	   else if((a == 255) && (b == 255) && (c == 255) && (d == 255))//不能全为255
	            	   {
	            		   return 0;
	            	   }
						return 1;
					}
					else
					{
						return 0;
					}
	                 break;
	default:return 0;break;
	}
}
void ICACHE_FLASH_ATTR GetIPData(uint32 IP_addr[],char * IP_Point)
{
  uint8_t i = 0, j = 0;

  for(i = 0, j = 0; IP_Point[i] != '&';)
	{
	 while(1)
	 {
		IP_addr[j] = (uint32)(IP_addr[j] * 10);
		IP_addr[j] = (uint32)((uint32)IP_Point[i] - 0x30 + IP_addr[j]);
		i ++;
		if(IP_Point[i] == '.')
		{
		  i ++;
		  j ++;
		 break;
		}
		if(IP_Point[i] == '&')
		{
		 break;
		}
	  }
	 }

}
void ICACHE_FLASH_ATTR GetIPData2(uint32 IP_addr[],char * IP_Point)
{
  uint8_t i = 0, j = 0;

  for(i = 0, j = 0; IP_Point[i] != ';';)
	{
	 while(1)
	 {
		IP_addr[j] = (uint32)(IP_addr[j] * 10);
		IP_addr[j] = (uint32)((uint32)IP_Point[i] - 0x30 + IP_addr[j]);
		i ++;
		if(IP_Point[i] == '.')
		{
		  i ++;
		  j ++;
		 break;
		}
		if(IP_Point[i] == ';')
		{
		 break;
		}
	  }
	 }

}
#define LINK   1
#define NOLINK 0
uint32 StringToHex(char *str, unsigned char *out, unsigned int *outlen)
{
    char *p = str;
    char high = 0, low = 0;
    int tmplen = 0, cnt = 0;
    tmplen = 2;
    while(cnt < (tmplen / 2))
    {
        high = ((*p > '9') && ((*p <= 'F') || (*p <= 'f'))) ? *p - 48 - 7 : *p - 48;
        low = (*(++ p) > '9' && ((*p <= 'F') || (*p <= 'f'))) ? *(p) - 48 - 7 : *(p) - 48;
        out[cnt] = ((high & 0x0f) << 4 | (low & 0x0f));
        p ++;
        cnt ++;
    }
    if(tmplen % 2 != 0) out[cnt] = ((*p > '9') && ((*p <= 'F') || (*p <= 'f'))) ? *p - 48 - 7 : *p - 48;

    if(outlen != NULL) *outlen = tmplen / 2 + tmplen % 2;
    return tmplen / 2 + tmplen % 2;
}
uint8 ICACHE_FLASH_ATTR EscapecharDeal(char * string,uint32 *stringLen)
{
	char *Point_a = NULL;
	char *Point_b = NULL;
	unsigned int Temp = 0;
	uint32 i = 0;
	Point_b = string;
	 while(1)
	{
		Point_a = strstr( Point_b, "+");
		if(Point_a != NULL)
		{
			*Point_a = ' ';
			Point_b = Point_a + 1;
		}
		else
		{
			break;
		}

	 }

	 i = 0;
	 Point_b = string;
	 while(1)
	{
		Point_a = strstr( Point_b, "%");
		if(Point_a != NULL) //则表示存在网络转义字符
		{
		  if(Point_a[1] == '2' && Point_a[2] == 'B')
		  {
		    Point_a[0] = '+';
		  }
		  else if(Point_a[1] == '2' && Point_a[2] == 'F')
		  {
		    Point_a[0] = '/';
		  }
		  else if(Point_a[1] == '3' && Point_a[2] == 'F')
		  {
		    Point_a[0] = '?';
		  }
		  else if(Point_a[1] == '2' && Point_a[2] == '5')
		  {
		    Point_a[0] = '%';
		  }
		  else if(Point_a[1] == '2' && Point_a[2] == '3')
		  {
		    Point_a[0] = '#';
		  }
		  else if(Point_a[1] == '2' && Point_a[2] == '6')
		  {
		    Point_a[0] = '&';
		  }
		  else if(Point_a[1] == '3' && Point_a[2] == 'D')
		  {
		    Point_a[0] = '=';
		  }
		  else if(Point_a[1] == '2' && Point_a[2] == '2')
		  {
		    Point_a[0] = '"';
		  }
		  else if(Point_a[1] == '2' && Point_a[2] == '8')
		  {
		    Point_a[0] = '(';
		  }
		  else if(Point_a[1] == '2' && Point_a[2] == '9')
		  {
		    Point_a[0] = ')';
		  }
		  else if(Point_a[1] == '2' && Point_a[2] == 'C')
		  {
		    Point_a[0] = ',';
		  }
		  else if(Point_a[1] == '3' && Point_a[2] == 'A')
		  {
		    Point_a[0] = ':';
		  }
		  else if(Point_a[1] == '3' && Point_a[2] == 'B')
		  {
		    Point_a[0] = ';';
		  }
		  else if(Point_a[1] == '3' && Point_a[2] == 'C')
		  {
		    Point_a[0] = '<';
		  }
		  else if(Point_a[1] == '3' && Point_a[2] == 'D')
		  {
		    Point_a[0] = '=';
		  }
		  else if(Point_a[1] == '3' && Point_a[2] == 'E')
		  {
		    Point_a[0] = '>';
		  }
		  else if(Point_a[1] == '4' && Point_a[2] == '0')
		  {
		    Point_a[0] = '@';
		  }
		  else if(Point_a[1] == '7' && Point_a[2] == 'C')
		  {
		    Point_a[0] = '|';
		  }
		  else if(Point_a[1] == '5' && Point_a[2] == 'C')
		  {
		    Point_a[0] = '\\';
		  }
		  else
		  {
			  StringToHex(&Point_a[1],&Point_a[0],&Temp);
			  //return NOLINK;
		  }
		    Point_b = Point_a + 1;
		    i = 1;
		    while(1)
		    {
		      if(Point_a[i + 2] == '\0' || Point_a[i + 1] == '\0') //在末尾 直接跳出
		     {
		     	break;
		     }
		      Point_a[i + 1] = Point_a[i + 3];
		      Point_a[i] = Point_a[i + 2];
		      i = i + 2;
		     }
		    *stringLen = *stringLen - 2;
		     string[*stringLen] = '\0';
		 }
		 else
		 {
			 break;
		 }
	}
	 return LINK;
	  //os_printf("string = %s\n",string);
}
uint8 ICACHE_FLASH_ATTR GetAPname_pswd_to_LingAP(char *Ap_Name,char *Ap_Pswd,uint32 Ap_NameLen,uint32 Ap_PswdLen)
	{
		uint32 i,APName[100] ;
		uint32 APPswd[64] ;
		uint8 APName1[100] ;
		uint8 APPswd1[64];
        uint32 AP_ID = 0,Ap_Erase = 0;
        /* AP名字太长，不连接 */
        if(Ap_NameLen > 90)
        {
           return 1;
        }
        os_bzero(APName1,100);//清空缓存区
        os_bzero(APPswd1,64);

		 for(i = 0; i < Ap_NameLen; i ++)
		 {
		   APName1[i] = (char)Ap_Name[i];
		   //os_printf("%c",APName1[i]);
		 }
		 APName1[Ap_NameLen] = '\0';
		   //os_printf("APPswd1 = \n");
		 for(i = 0; i < Ap_PswdLen; i ++)
		 {
		  APPswd1[i] = (char)Ap_Pswd[i];
		 // os_printf("%c",APPswd1[i]);
		 }
		 APPswd1[Ap_PswdLen] = '\0';
		 if(EscapecharDeal(APName1,&Ap_NameLen) == NOLINK)  //检查是否存在网络的转义字符
		 {
			// os_printf("return wufajiexi 1\n");
			 return 1; /* 存在无法解析的字符 直接返回*/
		 }
		 if(EscapecharDeal(APPswd1,&Ap_PswdLen) == NOLINK)  //检查是否存在网络的转义字符
		 {
			// os_printf("return wufajiexi 2\n");
			 return 1; /* 存在无法解析的字符 直接返回*/
		 }
		 /*AP名字或密码字符太长  拒绝连接 */
		 if(Ap_NameLen > APNAMEMAXLEN || Ap_PswdLen > 64)
		 {
			 return 1;
		 }
		 for(i = 0; i < Ap_NameLen; i ++)
		 {
			 APName[i] =  (uint32)APName1[i];
		 }
		 for(i = 0; i < Ap_PswdLen; i ++)
		 {
			 APPswd[i] =  (uint32)APPswd1[i];
		 }
		 Spi_FlashRead(AP_NUM_Erase,AP_NUM_ERASE_OFFSET,&AP_ID,1);  //从FLASH中获取AP_ID
		  switch(AP_ID)
		  {
		   case NO_APID: Ap_Erase = AP_NAME_PSWD_Erase;
						 AP_ID = APID1;    //
						 spi_flash_erase_sector (AP_NUM_Erase);
						 spi_flash_write(AP_NUM_Erase*4*1024 + AP_NUM_ERASE_OFFSET,&AP_ID,1 * 4);   //记录客户配置的APid
						 //os_printf("APID1 = %d\n",AP_ID);
						 break;
		   case APID1: Ap_Erase = AP1_NAME_PSWD_Erase;
			           AP_ID = APID2;    //
		               spi_flash_erase_sector (AP_NUM_Erase);
		               spi_flash_write(AP_NUM_Erase*4*1024 + AP_NUM_ERASE_OFFSET,&AP_ID,1 * 4);   //记录客户配置的APid
		               //os_printf("APID2 = %d\n",AP_ID);
		               break;
		   case APID2: Ap_Erase = AP_NAME_PSWD_Erase;
					   AP_ID = APID1;    //
					   spi_flash_erase_sector (AP_NUM_Erase);
					   spi_flash_write(AP_NUM_Erase*4*1024 + AP_NUM_ERASE_OFFSET,&AP_ID,1 * 4);   //记录客户配置的APid
					  // os_printf("APID1 = %d\n",AP_ID);
					   break;
		   default :  AP_ID = NO_APID;    //
			          spi_flash_erase_sector (AP_NUM_Erase);
			          spi_flash_write(AP_NUM_Erase*4*1024 + AP_NUM_ERASE_OFFSET,&AP_ID,1 * 4);   //记录客户配置的APid
			          //os_printf("default = %d\n",AP_ID);
			          break;
		  }
		  if((Ap_Erase == AP_NAME_PSWD_Erase) || (Ap_Erase == AP1_NAME_PSWD_Erase) )
		  {
			  spi_flash_erase_sector (Ap_Erase);  //往FLASH里存入AP名称和密码以及长度
			  spi_flash_write (Ap_Erase*4*1024 + AP_NAME_LEN_ERASE_OFFSET, &Ap_NameLen, 1 * 4);
			  spi_flash_write (Ap_Erase*4*1024 + AP_PSWD_LEN_ERASE_OFFSET, &Ap_PswdLen, 1 * 4);
			  spi_flash_write (Ap_Erase*4*1024 + AP_NAME_ERASE_OFFSET, (uint32 *)APName, Ap_NameLen * 4);
			  spi_flash_write (Ap_Erase*4*1024 + AP_PSWD_ERASE_OFFSET, (uint32 *)APPswd, Ap_PswdLen * 4);
		  }
		  timerForLinkAP = 0; //AP未连接状态计时清零
		  user_set_station_config(APName1,APPswd1,Ap_NameLen,Ap_PswdLen);  //连接相应的AP
		  return 0;
	}

void ICACHE_FLASH_ATTR Get_StaticIP_Set(char *StaticIP,char *Sub_Mask,char* Gate_way,char *SSID,char *Pswd,uint32 SSIDlen,uint32 Pswdlen)
{
	struct ip_info info;
	uint32 i,IP_addr[4] = {0,0,0,0};      //用以存储解析好的IP地址
	uint32 Subnet_mask[4] = {0,0,0,0};  //用以存储解析好的子网掩码
	uint32 Gateway[4] = {0,0,0,0};      //用以存储解析好的网管
    uint32 SSID1[32],Pswd1[32];
    os_bzero(SSID1,32);
    os_bzero(Pswd1,32);

    for(i = 0; i < SSIDlen; i ++)
    {
    	SSID1[i] = (uint32)SSID[i];
    	//os_printf("%c",SSID1[i]);
    }
       //os_printf("\n");
    for(i = 0; i < Pswdlen; i ++)
    {
    	Pswd1[i] = (uint32)Pswd[i];
    	//os_printf("%c",Pswd1[i]);
    }
       // os_printf("\n");
        GetIPData(IP_addr,StaticIP);
		GetIPData(Subnet_mask,Sub_Mask);
		GetIPData(Gateway,Gate_way);

		spi_flash_erase_sector (LOCAL_Erase);  //往FLASH里存入本地IP 网关 子网掩码
		spi_flash_write (LOCAL_Erase*4*1024 + LOCAL_IP_ERASE_OFFSET, IP_addr, 4 * 4);
		spi_flash_write (LOCAL_Erase*4*1024 + LOCAL_Mask_ERASE_OFFSET, Subnet_mask, 4 * 4);
		spi_flash_write (LOCAL_Erase*4*1024 + LOCAL_GATAWAY_ERASE_OFFSET, Gateway, 4 * 4);

                                           //往Flash里存入本地的wifi名称和密码以及相应的长度
    spi_flash_write (LOCAL_Erase*4*1024 + SSID_ERASE_OFFSET, SSID1, SSIDlen * 4);
    spi_flash_write (LOCAL_Erase*4*1024 + PSWD_ERASE_OFFSET, Pswd1, Pswdlen * 4);
    spi_flash_write (LOCAL_Erase*4*1024 + SSID_Len_ERASE_OFFSET, &SSIDlen, 1 * 4);
    spi_flash_write (LOCAL_Erase*4*1024 + PSWD_LEN_ERASE_OFFSET, &Pswdlen, 1 * 4);

	IP4_ADDR(&info.ip, IP_addr[0], IP_addr[1], IP_addr[2], IP_addr[3]);       //设置IP
	IP4_ADDR(&info.gw, Gateway[0], Gateway[1], Gateway[2], Gateway[3]);	      //设置网关
	IP4_ADDR(&info.netmask, Subnet_mask[0], Subnet_mask[1], Subnet_mask[2], Subnet_mask[3]); //设置子网掩码

	ConfigEsp8266(SSID,Pswd,SSIDlen,Pswdlen,&info,STATIONAP_MODE);   //重设本地相关参数

}
void ICACHE_FLASH_ATTR SetSSID(char *SSID,uint32 SSIDlen,char *PSWD,uint32 PSWDlen)
{
	struct softap_config config1;
	uint32 i = 0;
	uint32 SSID1[32],PSWD1[32];
	uint32 SSIDlen1 = 0;
	SSIDlen1 = SSIDlen;
	os_bzero(SSID1,32);
	os_bzero(PSWD1,32);

	 if((SSIDlen >= 32)|| (SSIDlen <= 0) || (PSWDlen >= 30))
	 {
		 //os_printf("return\n");
		 return;
	 }

	wifi_softap_get_config(&config1);       //获取参数
	os_memset(config1.ssid, 0, 32);         //先清空ssid
	os_memcpy(config1.ssid, SSID, SSIDlen);      //设置ssid
	if(PSWDlen >= 8)   /* 需要加密 */
	{
		 os_memset(config1.password, 0, 64);     //先清空密码
		 os_memcpy(config1.password, PSWD, PSWDlen);    //设置密码
		 config1.authmode = AUTH_WPA_WPA2_PSK;       //加密方式
	}
	else
	{
		config1.authmode = AUTH_OPEN;
	}
	config1.ssid_len = 0;                       //ssid长度=0，根据实际情况芯片自己处理
	config1.max_connection = 4;                 //可连接的STA的数量
	wifi_softap_set_config(&config1);           //根据参数设置AP

	for(i = 0; i < SSIDlen; i ++)/* 数据类型转换,flash只能存放32位数据 */
	{
		SSID1[i] = (uint32)SSID[i];
	}
	for(i = 0; i < PSWDlen; i ++)
	{
		PSWD1[i] = (uint32)PSWD[i];
	}

	spi_flash_erase_sector (LOCALSSID_Erase);  //往FLASH里存入wifi SSID和密码
	spi_flash_write (LOCALSSID_Erase*4*1024 + SSID_ERASE_OFFSET, SSID1, SSIDlen1 * 4);
    spi_flash_write (LOCALSSID_Erase*4*1024 + SSID_Len_ERASE_OFFSET, &SSIDlen1, 1 * 4);
    spi_flash_write (LOCALSSID_Erase*4*1024 + PSWD_ERASE_OFFSET, PSWD1, PSWDlen * 4);
    spi_flash_write (LOCALSSID_Erase*4*1024 + PSWD_LEN_ERASE_OFFSET, &PSWDlen, 1 * 4);



}
void ICACHE_FLASH_ATTR Get_StaticIP_Set2(char *StaticIP,char *Sub_Mask,char* Gate_way)
{
	struct ip_info info;
    uint32 DHCPState;
	uint32 IP_addr[4] = {0,0,0,0};      //用以存储解析好的IP地址
	uint32 Subnet_mask[4] = {0,0,0,0};  //用以存储解析好的子网掩码
	uint32 Gateway[4] = {0,0,0,0};      //用以存储解析好的网管

	GetIPData2(IP_addr,StaticIP);
	GetIPData2(Subnet_mask,Sub_Mask);
	GetIPData2(Gateway,Gate_way);

	//os_printf("NODHCPIP = %d.%d.%d.%d\n",IP_addr[0],IP_addr[1],IP_addr[2],IP_addr[3]);
	//os_printf("Sub_Mask = %d.%d.%d.%d\n",Subnet_mask[0],Subnet_mask[1],Subnet_mask[2],Subnet_mask[3]);
	//os_printf("Gate_way = %d.%d.%d.%d\n",Gateway[0],Gateway[1],Gateway[2],Gateway[3]);
	DHCPState = DHCP_CLOSE;
	spi_flash_erase_sector (NO_DHCP_Erase);  //往FLASH里存入本地IP 网关 子网掩码
	spi_flash_write (NO_DHCP_Erase*4*1024 + NO_DHCP_IP_ERASE_OFFSET, IP_addr, 4 * 4);
    spi_flash_write (NO_DHCP_Erase*4*1024 + NO_DHCP_Mask_ERASE_OFFSET, Subnet_mask, 4 * 4);
    spi_flash_write (NO_DHCP_Erase*4*1024 + NO_DHCP_GATAWAY_ERASE_OFFSET, Gateway, 4 * 4);
    spi_flash_write (NO_DHCP_Erase*4*1024 + DHCP_OPEN_ERASE_OFFSET, &DHCPState, 1* 4);

	IP4_ADDR(&info.ip, IP_addr[0], IP_addr[1], IP_addr[2], IP_addr[3]);       //设置IP
	IP4_ADDR(&info.gw, Gateway[0], Gateway[1], Gateway[2], Gateway[3]);	      //设置网关
	IP4_ADDR(&info.netmask, Subnet_mask[0], Subnet_mask[1], Subnet_mask[2], Subnet_mask[3]); //设置子网掩码

	wifi_station_dhcpc_stop();
	wifi_set_ip_info(STATION_IF,&info);

}

void ICACHE_FLASH_ATTR Get_StaticIP_Set3(char *StaticIP,char *Sub_Mask,char* Gate_way)
{
	struct ip_info info;
    uint32 DHCPState;
	uint32 IP_addr[4] = {0,0,0,0};      //用以存储解析好的IP地址
	uint32 Subnet_mask[4] = {0,0,0,0};  //用以存储解析好的子网掩码
	uint32 Gateway[4] = {0,0,0,0};      //用以存储解析好的网管

	GetIPData(IP_addr,StaticIP);
	GetIPData(Subnet_mask,Sub_Mask);
	GetIPData(Gateway,Gate_way);

//	os_printf("NODHCPIP = %d.%d.%d.%d\n",IP_addr[0],IP_addr[1],IP_addr[2],IP_addr[3]);
//	os_printf("Sub_Mask = %d.%d.%d.%d\n",Subnet_mask[0],Subnet_mask[1],Subnet_mask[2],Subnet_mask[3]);
//	os_printf("Gate_way = %d.%d.%d.%d\n",Gateway[0],Gateway[1],Gateway[2],Gateway[3]);

	IP4_ADDR(&info.ip, IP_addr[0], IP_addr[1], IP_addr[2], IP_addr[3]);       //设置IP
	IP4_ADDR(&info.gw, Gateway[0], Gateway[1], Gateway[2], Gateway[3]);	      //设置网关
	IP4_ADDR(&info.netmask, Subnet_mask[0], Subnet_mask[1], Subnet_mask[2], Subnet_mask[3]); //设置子网掩码
	wifi_station_dhcpc_stop();
	if(wifi_set_ip_info(STATION_IF,&info) == true)
	{
		DHCPState = DHCP_CLOSE;
		spi_flash_erase_sector (NO_DHCP_Erase);  //往FLASH里存入本地IP 网关 子网掩码
		spi_flash_write (NO_DHCP_Erase*4*1024 + NO_DHCP_IP_ERASE_OFFSET, IP_addr, 4 * 4);
		spi_flash_write (NO_DHCP_Erase*4*1024 + NO_DHCP_Mask_ERASE_OFFSET, Subnet_mask, 4 * 4);
		spi_flash_write (NO_DHCP_Erase*4*1024 + NO_DHCP_GATAWAY_ERASE_OFFSET, Gateway, 4 * 4);
		spi_flash_write (NO_DHCP_Erase*4*1024 + DHCP_OPEN_ERASE_OFFSET, &DHCPState, 1* 4);

	}
	else
	{
		wifi_station_dhcpc_start();
		DHCPState = DHCP_OPEN;
		spi_flash_erase_sector (NO_DHCP_Erase);  //往FLASH里存入本地IP 网关 子网掩码
		spi_flash_write (NO_DHCP_Erase*4*1024 + DHCP_OPEN_ERASE_OFFSET, &DHCPState, 1* 4);
	}

}
extern int32 Ntp_Timezone;
void ICACHE_FLASH_ATTR SetTimezone(char *TimezonePoint,uint32 Timezonelen)
{
	uint32 i,j,k;
	int32 Timezone = 0;
	int32 Timezone1 = 0;
	j = 0;
	if(TimezonePoint[0] == '-')
	{
		j = 1;
	}
	else
	{
		j = 3; /*代表为 正时区 */
	}
	for(i = j; i < j + 3; i ++)
	{
		if(TimezonePoint[i] == '%' || TimezonePoint[i] == '&' || TimezonePoint[i] == ':')
		{
			break;
		}

		Timezone = Timezone * 10;
		Timezone = (int32)TimezonePoint[i] - 0x30 + Timezone;
	}
	Timezone = Timezone * 100;
	for(k = i;k < i + 2; k ++ )
	{
		Timezone1 = Timezone1 * 10;
		Timezone1 = (int32)TimezonePoint[k + 3] - 0x30 + Timezone1;
	}
	Timezone1 = Timezone1 * 100 / 60 ;
	Timezone = Timezone1 + Timezone;
    if(j == 1)
    {
    	Timezone = -Timezone;
    }

    if((Timezone >= (-1200)) && (Timezone <= 1400))   //时区限制判断
    {
    	   //os_printf("genggai Timezone = %d\n",Timezone);
    	 Ntp_Timezone = Timezone;
		 spi_flash_erase_sector (TIMEZONE_ERASE);  //往FLASH里存入时区
		 spi_flash_write (TIMEZONE_ERASE*4*1024 + TIMEZONE_ERASE_OFFSET, &Timezone, 1 * 4);
    }

   //os_printf("genggai Timezone1 = %d\n",Timezone);
}

void ICACHE_FLASH_ATTR SetTimezone1(char *TimezonePoint,uint32 Timezonelen)
{
	uint32 i,j,k;
	int32 Timezone = 0;
	int32 Timezone1 = 0;
	j = 0;
	if(TimezonePoint[0] == '-')
	{
		j = 1;
	}
	//os_printf("\n");
	for(i = j; i < 2; i ++)
	{
		if(TimezonePoint[i] == ':')
		{
			break;
		}

		Timezone = Timezone * 10;
		Timezone = (int32)TimezonePoint[i] - 0x30 + Timezone;
	}
	Timezone = Timezone * 100;
	for(k = i;k < i + 2; k ++ )
	{
		Timezone1 = Timezone1 * 10;
		Timezone1 = (int32)TimezonePoint[k + 1] - 0x30 + Timezone1;
	}
	Timezone1 = Timezone1 * 100 / 60 ;
	Timezone = Timezone1 + Timezone;
    if(j == 1)
    {
    	Timezone = -Timezone;
    }

    if((Timezone >= (-1200)) && (Timezone <= 1400))   //时区限制判断
    {
    	 Ntp_Timezone = Timezone;
		 spi_flash_erase_sector (TIMEZONE_ERASE);  //往FLASH里存入时区
		 spi_flash_write (TIMEZONE_ERASE*4*1024 + TIMEZONE_ERASE_OFFSET, &Timezone, 1 * 4);
    }

   //os_printf("Timezone = %d\n",Timezone);
}
void ICACHE_FLASH_ATTR DstSet(char * Dsthourstart,char*Dsthourend,char * DstWeekSelStart,char *DstWeekSelEnd, char*DstWeekStart,char *DstWeekEnd,char * DstMonStart,char*DstMonEnd)
{
	uint32 IsChange = 0,i,hourstart1 = 0,hourend1 = 0;
	uint32 Dststartdata1[10],Dstenddata1[10];
	for(i = 0; i < 3; i ++)
	{
		if(DstWeekSelStart[i] != DstWeekSelEnd[i])
		{
			IsChange = 1;  //代表需要设置夏令时
			break;
		}
		if(DstWeekStart[i] != DstWeekEnd[i])
		{
			IsChange = 1;  //代表需要设置夏令时
			break;
		}
		if(DstMonStart[i] != DstMonEnd[i])
		{
			IsChange = 1;  //代表需要设置夏令时
			break;
		}
	}
	for(i = 0; i < 2; i ++)
	{
		if(Dsthourstart[i] == '&')
		{
			break;
		}
		  hourstart1 = hourstart1 * 10;
		  hourstart1 = (uint32)((uint32)Dsthourstart[i] - 0x30 + hourstart1);

	}
	for(i = 0; i < 2; i ++)
	{
		if(Dsthourend[i] == '&')
		{
			break;
		}
		 hourend1 = hourend1 * 10;
		 hourend1 = (uint32)((uint32)Dsthourend[i] - 0x30 + hourend1);

	}
	if(hourstart1 != hourend1)
	{
		IsChange = 1;
	}
	if(IsChange == 1)
	{
		DstStart.Dst_Hour = hourstart1;
		DstEnd.Dst_Hour = hourend1;
		switch(DstWeekSelStart[0])
		{
			case '1':DstStart.Dst_WeekSel = 1;break;
			case '2':DstStart.Dst_WeekSel = 2;break;
			case '3':DstStart.Dst_WeekSel = 3;break;
			case '4':DstStart.Dst_WeekSel = 4;break;
			case '5':DstStart.Dst_WeekSel = 5;break;
			case 'l':DstStart.Dst_WeekSel = LASTDST_WEEK;break;
			default:break;
		}
		switch(DstWeekSelEnd[0])
		{
			case '1':DstEnd.Dst_WeekSel = 1;break;
			case '2':DstEnd.Dst_WeekSel = 2;break;
			case '3':DstEnd.Dst_WeekSel = 3;break;
			case '4':DstEnd.Dst_WeekSel = 4;break;
			case '5':DstEnd.Dst_WeekSel = 5;break;
			case 'l':DstEnd.Dst_WeekSel = LASTDST_WEEK;break;
			default:break;
		}
		if(DstWeekStart[0] == 'S' && DstWeekStart[1] == 'u')
		{
			DstStart.Dst_Week = 0;
		}
		else if(DstWeekStart[0] == 'M' && DstWeekStart[1] == 'o')
		{
			DstStart.Dst_Week = 1;
		}
		else if(DstWeekStart[0] == 'T' && DstWeekStart[1] == 'u')
		{
			DstStart.Dst_Week = 2;
		}
		else if(DstWeekStart[0] == 'W' && DstWeekStart[1] == 'e')
		{
			DstStart.Dst_Week = 3;
		}
		else if(DstWeekStart[0] == 'T' && DstWeekStart[1] == 'h')
		{
			DstStart.Dst_Week = 4;
		}
		else if(DstWeekStart[0] == 'F' && DstWeekStart[1] == 'r')
		{
			DstStart.Dst_Week = 5;
		}
		else if(DstWeekStart[0] == 'S' && DstWeekStart[1] == 'a')
		{
			DstStart.Dst_Week = 6;
		}
		else
		{

		}

		if(DstWeekEnd[0] == 'S' && DstWeekEnd[1] == 'u')
		{
			DstEnd.Dst_Week = 0;
		}
		else if(DstWeekEnd[0] == 'M' && DstWeekEnd[1] == 'o')
		{
			DstEnd.Dst_Week = 1;
		}
		else if(DstWeekEnd[0] == 'T' && DstWeekEnd[1] == 'u')
		{
			DstEnd.Dst_Week = 2;
		}
		else if(DstWeekEnd[0] == 'W' && DstWeekEnd[1] == 'e')
		{
			DstEnd.Dst_Week = 3;
		}
		else if(DstWeekEnd[0] == 'T' && DstWeekEnd[1] == 'h')
		{
			DstEnd.Dst_Week = 4;
		}
		else if(DstWeekEnd[0] == 'F' && DstWeekEnd[1] == 'r')
		{
			DstEnd.Dst_Week = 5;
		}
		else if(DstWeekEnd[0] == 'S' && DstWeekEnd[1] == 'a')
		{
			DstEnd.Dst_Week = 6;
		}
		else
		{

		}
		if(DstMonStart[0] == 'J' && DstMonStart[1] == 'a' && DstMonStart[2] == 'n')
		{
			DstStart.Dst_Mon = 1;
		}
		else if(DstMonStart[0] == 'F' && DstMonStart[1] == 'e' && DstMonStart[2] == 'b')
		{
			DstStart.Dst_Mon = 2;
		}
		else if(DstMonStart[0] == 'M' && DstMonStart[1] == 'a' && DstMonStart[2] == 'r')
		{
			DstStart.Dst_Mon = 3;
		}
		else if(DstMonStart[0] == 'A' && DstMonStart[1] == 'p' && DstMonStart[2] == 'r')
		{
			DstStart.Dst_Mon = 4;
		}
		else if(DstMonStart[0] == 'M' && DstMonStart[1] == 'a' && DstMonStart[2] == 'y')
		{
			DstStart.Dst_Mon = 5;
		}
		else if(DstMonStart[0] == 'J' && DstMonStart[1] == 'u' && DstMonStart[2] == 'n')
		{
			DstStart.Dst_Mon = 6;
		}
		else if(DstMonStart[0] == 'J' && DstMonStart[1] == 'u' && DstMonStart[2] == 'l')
		{
			DstStart.Dst_Mon = 7;
		}
		else if(DstMonStart[0] == 'A' && DstMonStart[1] == 'u' && DstMonStart[2] == 'g')
		{
			DstStart.Dst_Mon = 8;
		}
		else if(DstMonStart[0] == 'S' && DstMonStart[1] == 'e' && DstMonStart[2] == 'p')
		{
			DstStart.Dst_Mon = 9;
		}
		else if(DstMonStart[0] == 'O' && DstMonStart[1] == 'c' && DstMonStart[2] == 't')
		{
			DstStart.Dst_Mon = 10;
		}
		else if(DstMonStart[0] == 'N' && DstMonStart[1] == 'o' && DstMonStart[2] == 'v')
		{
			DstStart.Dst_Mon = 11;
		}
		else if(DstMonStart[0] == 'D' && DstMonStart[1] == 'e' && DstMonStart[2] == 'c')
		{
			DstStart.Dst_Mon = 12;
		}

		if(DstMonEnd[0] == 'J' && DstMonEnd[1] == 'a' && DstMonEnd[2] == 'n')
		{
			DstEnd.Dst_Mon = 1;
		}
		else if(DstMonEnd[0] == 'F' && DstMonEnd[1] == 'e' && DstMonEnd[2] == 'b')
		{
			DstEnd.Dst_Mon = 2;
		}
		else if(DstMonEnd[0] == 'M' && DstMonEnd[1] == 'a' && DstMonEnd[2] == 'r')
		{
			DstEnd.Dst_Mon = 3;
		}
		else if(DstMonEnd[0] == 'A' && DstMonEnd[1] == 'p' && DstMonEnd[2] == 'r')
		{
			DstEnd.Dst_Mon = 4;
		}
		else if(DstMonEnd[0] == 'M' && DstMonEnd[1] == 'a' && DstMonEnd[2] == 'y')
		{
			DstEnd.Dst_Mon = 5;
		}
		else if(DstMonEnd[0] == 'J' && DstMonEnd[1] == 'u' && DstMonEnd[2] == 'n')
		{
			DstEnd.Dst_Mon = 6;
		}
		else if(DstMonEnd[0] == 'J' && DstMonEnd[1] == 'u' && DstMonEnd[2] == 'l')
		{
			DstEnd.Dst_Mon = 7;
		}
		else if(DstMonEnd[0] == 'A' && DstMonEnd[1] == 'u' && DstMonEnd[2] == 'g')
		{
			DstEnd.Dst_Mon = 8;
		}
		else if(DstMonEnd[0] == 'S' && DstMonEnd[1] == 'e' && DstMonEnd[2] == 'p')
		{
			DstEnd.Dst_Mon = 9;
		}
		else if(DstMonEnd[0] == 'O' && DstMonEnd[1] == 'c' && DstMonEnd[2] == 't')
		{
			DstEnd.Dst_Mon = 10;
		}
		else if(DstMonEnd[0] == 'N' && DstMonEnd[1] == 'o' && DstMonEnd[2] == 'v')
		{
			DstEnd.Dst_Mon = 11;
		}
		else if(DstMonEnd[0] == 'D' && DstMonEnd[1] == 'e' && DstMonEnd[2] == 'c')
		{
			DstEnd.Dst_Mon = 12;
		}

		IsChange = DST_SET;
		spi_flash_erase_sector (DST_Erase);  //往FLASH里存入夏令时 信息
		spi_flash_write (DST_Erase*4*1024 + ISSET_DST_ERASE_OFFSET, &IsChange, 1 * 4);
		 /* 存入夏令时相关数据 */
		spi_flash_write (DST_Erase*4*1024 + DSTSTART_HOUR_ERASE_OFFSET, &DstStart.Dst_Hour, 1 * 4);
		spi_flash_write (DST_Erase*4*1024 + DSTSTART_SELE_WEEK_ERASE_OFFSET, &DstStart.Dst_WeekSel, 1 * 4);
		spi_flash_write (DST_Erase*4*1024 + DSTSTART_WEEK_ERASE_OFFSET, &DstStart.Dst_Week, 1 * 4);
		spi_flash_write (DST_Erase*4*1024 + DSTSTART_MON_ERASE_OFFSET, &DstStart.Dst_Mon, 1 * 4);

		spi_flash_write (DST_Erase*4*1024 + DSTEND_HOUR_ERASE_OFFSET, &DstEnd.Dst_Hour, 1 * 4);
		spi_flash_write (DST_Erase*4*1024 + DSTEND_SELE_WEEK_ERASE_OFFSET, &DstEnd.Dst_WeekSel, 1 * 4);
		spi_flash_write (DST_Erase*4*1024 + DSTEND_WEEK_ERASE_OFFSET, &DstEnd.Dst_Week, 1 * 4);
		spi_flash_write (DST_Erase*4*1024 + DSTEND_MON_ERASE_OFFSET, &DstEnd.Dst_Mon, 1 * 4);

		//os_printf("DstStart %d,%d,%d,%d\n",DstStart.Dst_Hour,DstStart.Dst_WeekSel,DstStart.Dst_Week,DstStart.Dst_Mon);
		//os_printf("DstEnd %d,%d,%d,%d\n",DstEnd.Dst_Hour,DstEnd.Dst_WeekSel,DstEnd.Dst_Week,DstEnd.Dst_Mon);

	}
	else
	{
		//os_printf("NO_DST_SET\n");
		IsChange = DST_NO_SET;
		spi_flash_erase_sector (DST_Erase);
		spi_flash_write (DST_Erase*4*1024 + ISSET_DST_ERASE_OFFSET, &IsChange, 1 * 4);
	}


}
void ICACHE_FLASH_ATTR DstSet2(char * Dsthourstart,char*Dsthourend,char * DstWeekSelStart,char *DstWeekSelEnd, char*DstWeekStart,char *DstWeekEnd,char * DstMonStart,char*DstMonEnd)
{
	uint32 IsChange = 0,i;
	DstStart_test.Dst_Hour = 0;
	DstStart_test.Dst_WeekSel = 0;
	DstStart_test.Dst_Week = 0;
	DstStart_test.Dst_Mon = 0;

	DstEnd_test.Dst_Hour = 0;
	DstEnd_test.Dst_WeekSel = 0;
	DstEnd_test.Dst_Week = 0;
	DstEnd_test.Dst_Mon = 0;

	DstStart_test.Dst_WeekSel = DstWeekSelStart[0] - 0x30;
	DstEnd_test.Dst_WeekSel = DstWeekSelEnd[0] - 0x30;

	DstStart_test.Dst_Week = DstWeekStart[0] - 0x30;
	DstEnd_test.Dst_Week = DstWeekEnd[0] - 0x30;

	for(i = 0; i < 2; i ++)
	{
		if(DstMonStart[i] == ';')
		{
			break;
		}
		DstStart_test.Dst_Mon = DstStart_test.Dst_Mon * 10;
		DstStart_test.Dst_Mon = (uint32)((uint32)DstMonStart[i] - 0x30 + DstStart_test.Dst_Mon);
	}

	for(i = 0; i < 2; i ++)
	{
		if(DstMonEnd[i] == ';')
		{
			break;
		}
		DstEnd_test.Dst_Mon = DstEnd_test.Dst_Mon * 10;
		DstEnd_test.Dst_Mon = (uint32)((uint32)DstMonEnd[i] - 0x30 + DstEnd_test.Dst_Mon);
	}

	for(i = 0; i < 2; i ++)
	{
		if(Dsthourstart[i] == '-')
		{
			break;
		}
		DstStart_test.Dst_Hour = DstStart_test.Dst_Hour * 10;
		DstStart_test.Dst_Hour = (uint32)((uint32)Dsthourstart[i] - 0x30 + DstStart_test.Dst_Hour);

	}
	for(i = 0; i < 2; i ++)
	{
		if(Dsthourend[i] == '-')
		{
			break;
		}
		DstEnd_test.Dst_Hour = DstEnd_test.Dst_Hour * 10;
		DstEnd_test.Dst_Hour = (uint32)((uint32)Dsthourend[i] - 0x30 + DstEnd_test.Dst_Hour);
	}
	if((DstStart_test.Dst_Hour != DstEnd_test.Dst_Hour) || (DstStart_test.Dst_WeekSel != DstEnd_test.Dst_WeekSel) ||\
			(DstStart_test.Dst_Week != DstEnd_test.Dst_Week) || (DstStart_test.Dst_Mon != DstEnd_test.Dst_Mon)	)
	{
		if(DstStart_test.Dst_Hour >= 0 && DstStart_test.Dst_Hour <= 23 \
				&& DstEnd_test.Dst_Hour >= 0 && DstEnd_test.Dst_Hour <= 23 && \
				DstStart_test.Dst_WeekSel >= 0 && DstStart_test.Dst_WeekSel <= 8 &&\
				DstEnd_test.Dst_WeekSel >= 0 && DstEnd_test.Dst_WeekSel <= 8 &&\
				DstStart_test.Dst_Week >= 0 && DstStart_test.Dst_Week <= 6 &&\
				DstEnd_test.Dst_Week >= 0 && DstEnd_test.Dst_Week <= 6 &&\
				DstStart_test.Dst_Mon >= 1 && DstStart_test.Dst_Mon <= 12 &&\
				DstEnd_test.Dst_Mon >= 1 && DstEnd_test.Dst_Mon <= 12 )
		{

			IsChange = 1;
			IsDst = DST_SET;
		}

	}
	if(IsChange == 1)
	{
		DstStart.Dst_Hour = DstStart_test.Dst_Hour;
		DstStart.Dst_WeekSel = DstStart_test.Dst_WeekSel;
		DstStart.Dst_Week = DstStart_test.Dst_Week;
		DstStart.Dst_Mon = DstStart_test.Dst_Mon ;

		DstEnd.Dst_Hour = DstEnd_test.Dst_Hour;
		DstEnd.Dst_WeekSel = DstEnd_test.Dst_WeekSel;
		DstEnd.Dst_Week = DstEnd_test.Dst_Week;
		DstEnd.Dst_Mon = DstEnd_test.Dst_Mon ;

		//os_printf("DstStart %d,%d,%d,%d\n",DstStart.Dst_Hour,DstStart.Dst_WeekSel,DstStart.Dst_Week,DstStart.Dst_Mon);
		//os_printf("DstEnd %d,%d,%d,%d\n",DstEnd.Dst_Hour,DstEnd.Dst_WeekSel,DstEnd.Dst_Week,DstEnd.Dst_Mon);
		IsChange = DST_SET;
		spi_flash_erase_sector (DST_Erase);  //往FLASH里存入夏令时 信息
		spi_flash_write (DST_Erase*4*1024 + ISSET_DST_ERASE_OFFSET, &IsChange, 1 * 4);
		 /* 存入夏令时相关数据 */
		spi_flash_write (DST_Erase*4*1024 + DSTSTART_HOUR_ERASE_OFFSET, &DstStart.Dst_Hour, 1 * 4);
		spi_flash_write (DST_Erase*4*1024 + DSTSTART_SELE_WEEK_ERASE_OFFSET, &DstStart.Dst_WeekSel, 1 * 4);
		spi_flash_write (DST_Erase*4*1024 + DSTSTART_WEEK_ERASE_OFFSET, &DstStart.Dst_Week, 1 * 4);
		spi_flash_write (DST_Erase*4*1024 + DSTSTART_MON_ERASE_OFFSET, &DstStart.Dst_Mon, 1 * 4);

		spi_flash_write (DST_Erase*4*1024 + DSTEND_HOUR_ERASE_OFFSET, &DstEnd.Dst_Hour, 1 * 4);
		spi_flash_write (DST_Erase*4*1024 + DSTEND_SELE_WEEK_ERASE_OFFSET, &DstEnd.Dst_WeekSel, 1 * 4);
		spi_flash_write (DST_Erase*4*1024 + DSTEND_WEEK_ERASE_OFFSET, &DstEnd.Dst_Week, 1 * 4);
		spi_flash_write (DST_Erase*4*1024 + DSTEND_MON_ERASE_OFFSET, &DstEnd.Dst_Mon, 1 * 4);

		//os_printf("DstStart %d,%d,%d,%d\n",DstStart.Dst_Hour,DstStart.Dst_WeekSel,DstStart.Dst_Week,DstStart.Dst_Mon);
		//os_printf("DstEnd %d,%d,%d,%d\n",DstEnd.Dst_Hour,DstEnd.Dst_WeekSel,DstEnd.Dst_Week,DstEnd.Dst_Mon);

	}
	else
	{
		//os_printf("NO_DST_SET\n");
		IsChange = DST_NO_SET;
		spi_flash_erase_sector (DST_Erase);
		spi_flash_write (DST_Erase*4*1024 + ISSET_DST_ERASE_OFFSET, &IsChange, 1 * 4);
	}


}
void ICACHE_FLASH_ATTR SetNtpServer(char *Ntpserver,uint32 NtpseverLen)
{
	uint32 i;
    int32 timezone1 = 0;
    sint8 timezone =0;
    uint32 Ntpserver2[32];
	char Ntpserver1[32];

	 os_bzero(Ntpserver2,32);
	 os_bzero(Ntpserver1,32);

	//os_printf("Ntpserver1 = ");
	for(i = 0; i < NtpseverLen; i ++)
	{
		Ntpserver1[i] = Ntpserver[i];
		Ntpserver2[i] = (uint32)Ntpserver[i];
		//os_printf("%c",Ntpserver1[i]);
	}
	Ntpserver1[NtpseverLen] = '\0';

	spi_flash_erase_sector (NTP_IP_Erase);  //往FLASH里存入NTP IP 信息
	spi_flash_write (NTP_IP_Erase*4*1024 + NTP_IP_ERASE_OFFSET, Ntpserver2, NtpseverLen * 4);
	spi_flash_write (NTP_IP_Erase*4*1024 + NTP_IP_LEN_ERASE_OFFSET, &NtpseverLen, 1 * 4);
	sntp_stop();
	Sntp_Config(Ntpserver1,NtpseverLen);
	//os_printf("\n");
}
void ICACHE_FLASH_ATTR ChangeTimeOutInterva(char *TimeIntervaPoing,uint32 TimeIntervaLen1)
{
	uint8_t i = 0;
	TimeOutInterva = 0;
	for(i = 0; i < TimeIntervaLen1; i ++)
	{
	  if((TimeIntervaPoing[i] > '9') || (TimeIntervaPoing[i] < '0'))
	  {
		  if(TimeIntervaPoing[i] == '&')
		  {
			  break;
		  }
		  else
		  {
			  return;
		  }
	  }
	  if(TimeIntervaPoing[i] == '&')
	  {
		  break;
	  }
	  TimeOutInterva = TimeOutInterva * 10;
	  TimeOutInterva = (uint32_t)((uint32_t)(TimeIntervaPoing[i]) - 0x30 + TimeOutInterva);
	}
	spi_flash_erase_sector (TIME_Interva_ERASE);  //
	spi_flash_write (TIME_Interva_ERASE*4*1024 + TIME_Interva_ERASE_OFFSET, &TimeOutInterva, 1 * 4);
	if(TimeOutInterva >= 1 && TimeOutInterva <= 9999)
	{
	 os_timer_disarm(&sntpPutTime);
	 os_timer_setfn(&sntpPutTime, (os_timer_func_t *)PutSntpTime, NULL);
	 os_timer_arm(&sntpPutTime, TimeOutInterva * 1000, 1);
	}
	if(TimeOutInterva == 0)
	{
		 os_timer_disarm(&sntpPutTime);
	}
}
void ICACHE_FLASH_ATTR SetAutoWeb(char *Point)
{
	uint32 AutoWebState = 0;
    if(Point[0] != NO_AUTO_ACCESS_WEB) /* 代表连接上热点后，自动弹出网页*/
    {
    	AutoWebState = AUTO_ACCESS_WEB;
    	spi_flash_erase_sector (IS_AUTO_WEB_Erase);  //
    	spi_flash_write (IS_AUTO_WEB_Erase*4*1024 + IS_AUTO_WEB_ERASE_OFFSET, &AutoWebState, 1 * 4); /* 设置自动弹出网页 */
    }
    else
    {
    	AutoWebState = NO_AUTO_ACCESS_WEB;
    	spi_flash_erase_sector (IS_AUTO_WEB_Erase);  //
    	spi_flash_write (IS_AUTO_WEB_Erase*4*1024 + IS_AUTO_WEB_ERASE_OFFSET, &AutoWebState, 1 * 4); /* 设置不会自动弹出网页 */
    }
}


void ICACHE_FLASH_ATTR PrintfBuffer(char *Point,uint32 Len)
{
	uint32 i = 0;
	for(i = 0; i < Len; i ++)
	{
		os_printf("%c",Point[i]);
	}
	os_printf("\n");
}
void ICACHE_FLASH_ATTR tcp_server_recvApp_cb(void *arg, char *pusrdata, unsigned short length)
{
	uint32_t i = 0;
	AppSetPacket AppSetInf;
	char * Test_Point = NULL;
	char * Test_Point1 = NULL;
	struct espconn *pespconn = arg;
	uart0_tx_buffer(pusrdata, length);

	LedBlue(0);
	LedGreen(0);
	LedRed(0);
	SysTime_ms = 300;
//	AppSetInf.DHCP_State = strstr( pusrdata, "DHCP:OPEN;");
//	AppSetInf.Ssidpoint = strstr( pusrdata, "SSID:");
//	AppSetInf.Pskpoint = strstr( pusrdata, "PSK:");
//	AppSetInf.NtpServerPoint = strstr( pusrdata, "NTPServer:");
//	AppSetInf.TimeZonePoint = strstr( pusrdata, "ZONE:");
//	AppSetInf.DstStartPoint = strstr( pusrdata, "DSTS:");
//	AppSetInf.DstEndPoint =  strstr( pusrdata, "DSTE:");
//	AppSetInf.Ip_Point = strstr( pusrdata, "IP:");
//	AppSetInf.Mask_Point = strstr( pusrdata, "mask:");
//	AppSetInf.Gateway_Point = strstr( pusrdata, "Gateway:");
//
//
//	 /*得到了更改配置的命令 */
//	if(AppSetInf.DHCP_State != NULL)
//	{
//		uint32 DHCPState;
//		DHCPState = DHCP_OPEN;
//		spi_flash_erase_sector (NO_DHCP_Erase);
//	    spi_flash_write (NO_DHCP_Erase*4*1024 + DHCP_OPEN_ERASE_OFFSET, &DHCPState, 1* 4);
//		espconn_sent(pespconn, "OK", 2);
//		wifi_station_dhcpc_start();
//	}
//	if((AppSetInf.Ssidpoint != NULL) && (AppSetInf.Pskpoint != NULL))
//	{
//		Test_Point1 = AppSetInf.Pskpoint - 1;
//	    if(Test_Point1[0] != ';')
//	    {
//	    	espconn_sent(pespconn, "ERROR", 5);
//	    	return ;
//	    }
//		AppSetInf.Ssidpoint = AppSetInf.Ssidpoint + strlen("SSID:"); /* 获取热点名称以及长度 */
//		Test_Point = strstr( AppSetInf.Ssidpoint, ";");
//		if(Test_Point != NULL)
//		{
//		  AppSetInf.SsidLen = strstr( AppSetInf.Ssidpoint, ";") - AppSetInf.Ssidpoint;
//		  if(AppSetInf.SsidLen <= 0)
//		  {
//				espconn_sent(pespconn, "ERROR", 5);
//				return;
//		  }
//		  //PrintfBuffer(AppSetInf.Ssidpoint,AppSetInf.SsidLen);
//		}
//		else
//		{
//			espconn_sent(pespconn, "ERROR", 5);
//			return;
//		}
//
//		AppSetInf.Pskpoint = AppSetInf.Pskpoint + strlen("PSK:"); /* 获取热点密码名称以及长度 */
//		Test_Point = strstr( AppSetInf.Pskpoint, ";");
//		if(Test_Point != NULL)
//		{
//		  AppSetInf.PskLen = Test_Point - AppSetInf.Pskpoint;
//		  //PrintfBuffer(AppSetInf.Pskpoint,AppSetInf.PskLen);
//		}
//		else
//		{
//			espconn_sent(pespconn, "ERROR", 5);
//			return;
//		}
//
//		espconn_sent(pespconn, "OK", 2);
//		GetAPname_pswd_to_LingAP(AppSetInf.Ssidpoint,AppSetInf.Pskpoint,AppSetInf.SsidLen,AppSetInf.PskLen);
//		Init_Setup = INIT_NO_SET;     //下次启动时，代表数据已经更改，不需要恢复到出厂设置
//		Spi_FlashWrite(Init_Erase,0, &Init_Setup, 1);
//	}
//
//	if(AppSetInf.TimeZonePoint != NULL)
//	{
//		AppSetInf.TimeZonePoint = AppSetInf.TimeZonePoint + strlen("ZONE:");/* 获取时区 以及长度 */
//		Test_Point = strstr( AppSetInf.TimeZonePoint, ":") ;
//		if(Test_Point == NULL)
//		{
//			espconn_sent(pespconn, "ERROR", 5);
//			return;
//		}
//		Test_Point =  Test_Point + 3;
//		if(Test_Point[0] == ';')
//		{
//		 AppSetInf.TimeZoneLen = Test_Point - AppSetInf.TimeZonePoint;
//		 //PrintfBuffer(AppSetInf.TimeZonePoint,AppSetInf.TimeZoneLen);
//		 if(AppSetInf.TimeZoneLen < 4)
//		 {
//			 espconn_sent(pespconn, "ERROR", 5);
//			 return;
//		 }
//		}
//		else
//		{
//			espconn_sent(pespconn, "ERROR", 5);
//			return;
//		}
//
//		espconn_sent(pespconn, "OK", 2);
//		SetTimezone1(AppSetInf.TimeZonePoint,AppSetInf.TimeZoneLen);  //设置时区
//
//		Init_Setup = INIT_NO_SET;     //下次启动时，代表数据已经更改，不需要恢复到出厂设置
//		Spi_FlashWrite(Init_Erase,0, &Init_Setup, 1);
//	}
//
//	if(AppSetInf.NtpServerPoint != NULL)
//	{
//		AppSetInf.NtpServerPoint = AppSetInf.NtpServerPoint + strlen("NTPServer:");/* 获取NTPip以及长度 */
//		Test_Point = strstr( AppSetInf.NtpServerPoint, ";");
//		if(Test_Point != NULL)
//		{
//		 AppSetInf.NtpServerLen = Test_Point - AppSetInf.NtpServerPoint;
//		 if (AppSetInf.NtpServerLen <= 0)
//		 {
//				espconn_sent(pespconn, "ERROR", 5);
//				return;
//		 }
//		 //PrintfBuffer(AppSetInf.NtpServerPoint,AppSetInf.NtpServerLen);
//		}
//		else
//		{
//			espconn_sent(pespconn, "ERROR", 5);
//			return;
//		}
//
//		espconn_sent(pespconn, "OK", 2);
//		SetNtpServer(AppSetInf.NtpServerPoint,AppSetInf.NtpServerLen );
//		Init_Setup = INIT_NO_SET;     //下次启动时，代表数据已经更改，不需要恢复到出厂设置
//		Spi_FlashWrite(Init_Erase,0, &Init_Setup, 1);
//	}
//
//	if( (AppSetInf.DstStartPoint != NULL) && (AppSetInf.DstEndPoint != NULL))
//	{
//		Test_Point1 = AppSetInf.DstEndPoint - 1;
//	    if(Test_Point1[0] != ';')
//	    {
//	    	espconn_sent(pespconn, "ERROR", 5);
//	    	return ;
//	    }
//		AppSetInf.DstStartPoint = AppSetInf.DstStartPoint + strlen("DSTS:");
//		AppSetInf.DstEndPoint = AppSetInf.DstEndPoint + strlen("DSTE:");
//               /* 从数据段分离夏令时相关字符串 */
//		AppSetInf.Dst_point.DstHourS_Point = AppSetInf.DstStartPoint;
//
//		Test_Point = strstr( AppSetInf.Dst_point.DstHourS_Point, "-");
//		if(Test_Point != NULL)
//		{
//		  AppSetInf.Dst_point.DstWeekSeleS_Point = Test_Point + 1;
//		}
//		else
//		{
//			espconn_sent(pespconn, "ERROR", 5);
//			return;
//		}
//		Test_Point = strstr( AppSetInf.Dst_point.DstWeekSeleS_Point, "-");
//		if(Test_Point != NULL)
//		{
//		  AppSetInf.Dst_point.DstWeekS_Point = Test_Point + 1;
//		}
//		else
//		{
//			espconn_sent(pespconn, "ERROR", 5);
//			return;
//		}
//
//		Test_Point = strstr( AppSetInf.Dst_point.DstWeekS_Point, "-");
//		if(Test_Point != NULL)
//		{
//		  AppSetInf.Dst_point.DstMonS_Point = Test_Point + 1;
//		}
//		else
//		{
//			espconn_sent(pespconn, "ERROR", 5);
//			return;
//		}
//
//		AppSetInf.Dst_point.DstHourE_Point = AppSetInf.DstEndPoint;
//		Test_Point = strstr( AppSetInf.Dst_point.DstHourE_Point, "-") ;
//		if(Test_Point != NULL)
//		{
//		  AppSetInf.Dst_point.DstWeekSeleE_Point = Test_Point + 1;
//		}
//		Test_Point = strstr( AppSetInf.Dst_point.DstWeekSeleE_Point, "-");
//		if(Test_Point != NULL)
//		{
//		  AppSetInf.Dst_point.DstWeekE_Point = Test_Point + 1;
//		}
//		else
//		{
//			espconn_sent(pespconn, "ERROR", 5);
//			return;
//		}
//		Test_Point =  strstr( AppSetInf.Dst_point.DstWeekE_Point, "-");
//		if(Test_Point != NULL)
//		{
//		  AppSetInf.Dst_point.DstMonE_Point = Test_Point + 1;
//		}
//		else
//		{
//			espconn_sent(pespconn, "ERROR", 5);
//			return;
//		}
//		espconn_sent(pespconn, "OK", 2);
//		DstSet2(AppSetInf.Dst_point.DstHourS_Point,AppSetInf.Dst_point.DstHourE_Point,\
//		        AppSetInf.Dst_point.DstWeekSeleS_Point,AppSetInf.Dst_point.DstWeekSeleE_Point,\
//				AppSetInf.Dst_point.DstWeekS_Point,\
//				AppSetInf.Dst_point.DstWeekE_Point,AppSetInf.Dst_point.DstMonS_Point,\
//				AppSetInf.Dst_point.DstMonE_Point); // 夏令时设置
//
//		Init_Setup = INIT_NO_SET;     //下次启动时，代表数据已经更改，不需要恢复到出厂设置
//		Spi_FlashWrite(Init_Erase,0, &Init_Setup, 1);
//	}
//
//	if( (AppSetInf.Ip_Point != NULL) && (AppSetInf.Mask_Point != NULL) && (AppSetInf.Gateway_Point != NULL))
//	{
//		Test_Point1 = AppSetInf.Mask_Point - 1;
//	    if(Test_Point1[0] != ';')
//	    {
//	    	espconn_sent(pespconn, "ERROR", 5);
//	    	return ;
//	    }
//		Test_Point1 = AppSetInf.Gateway_Point - 1;
//	    if(Test_Point1[0] != ';')
//	    {
//	    	espconn_sent(pespconn, "ERROR", 5);
//	    	return ;
//	    }
//		AppSetInf.Ip_Point = AppSetInf.Ip_Point + strlen("IP:");/* 获取静态IP，子网掩码，网关 以及长度 */
//		Test_Point = strstr( AppSetInf.Ip_Point, ";");
//		if(Test_Point != NULL)
//		{
//		  AppSetInf.Ip_Len = Test_Point - AppSetInf.Ip_Point;
//		  //PrintfBuffer(AppSetInf.Ip_Point,AppSetInf.Ip_Len);
//		}
//		else
//		{
//			espconn_sent(pespconn, "ERROR", 5);
//			return;
//		}
//
//
//
//		AppSetInf.Mask_Point = AppSetInf.Mask_Point + strlen("mask:");
//		Test_Point =  strstr( AppSetInf.Mask_Point, ";");
//		if(Test_Point != NULL)
//		{
//		  AppSetInf.Mask_Len = Test_Point - AppSetInf.Mask_Point;
//		  //PrintfBuffer(AppSetInf.Mask_Point,AppSetInf.Mask_Len);
//		}
//		else
//		{
//			espconn_sent(pespconn, "ERROR", 5);
//			return;
//		}
//
//
//
//		AppSetInf.Gateway_Point = AppSetInf.Gateway_Point + strlen("Gateway:");
//		Test_Point = strstr( AppSetInf.Gateway_Point, ";");
//		if(Test_Point != NULL)
//		{
//		  AppSetInf.Gateway_Len = Test_Point - AppSetInf.Gateway_Point;
//		  //PrintfBuffer(AppSetInf.Gateway_Point,AppSetInf.Gateway_Len);
//		}
//		else
//		{
//			espconn_sent(pespconn, "ERROR", 5);
//			return;
//		}
//
//
//		if((IsIpaddr(AppSetInf.Ip_Point,AppSetInf.Ip_Len,IPTYPE) == 1) && (IsIpaddr(AppSetInf.Mask_Point,AppSetInf.Mask_Len,SUBTYPE) == 1)\
//			&& (IsIpaddr(AppSetInf.Gateway_Point,AppSetInf.Gateway_Len,GATEWAYTYPE) == 1))  //判断IP地址是否合法
//		 {
//			espconn_sent(pespconn, "OK", 2);
//			Get_StaticIP_Set2(AppSetInf.Ip_Point,AppSetInf.Mask_Point ,AppSetInf.Gateway_Point);
//			Init_Setup = INIT_NO_SET;     //下次启动时，代表数据已经更改，不需要恢复到出厂设置
//		    Spi_FlashWrite(Init_Erase,0, &Init_Setup, 1);
//		 }
//		else
//		{
//		   espconn_sent(pespconn, "ERROR", 5);
//		}
//
//	}

}

void ICACHE_FLASH_ATTR SetDynamic_Static_IP(char * Point,char * IP,char IpLen,char * Sub,char SubLen, char * Gw,char GwLen)
{
	 if(Point[0] == '2')  /* 要修改为静态IP地址  */
	  {
		 if((IsIpaddr(IP,IpLen,IPTYPE) == 1) && (IsIpaddr(Sub,SubLen,SUBTYPE) == 1)\
			&& (IsIpaddr(Gw,GwLen,GATEWAYTYPE) == 1))  //判断IP地址是否合法
		  {
			  Get_StaticIP_Set3(IP,Sub ,Gw);
		  }
	     }
	 else if(Point[0] == '1')/* 动态获取IP地址 */
	  {
	      uint32 DHCPState;
	 	  Spi_FlashRead(NO_DHCP_Erase,DHCP_OPEN_ERASE_OFFSET,&DHCPState,1);
	      if(DHCPState != DHCP_OPEN)
	      {
	        DHCPState = DHCP_OPEN;
			spi_flash_erase_sector (NO_DHCP_Erase);
		    spi_flash_write (NO_DHCP_Erase*4*1024 + DHCP_OPEN_ERASE_OFFSET, &DHCPState, 1* 4);
			wifi_station_dhcpc_start();
	      }
	  }
}

void ICACHE_FLASH_ATTR ChangeTcpClient(char*Point1,char*Point2)
{
	   if(Point1 != NULL)
	   {
		   char *TcpServerIP_Point = NULL,*TcpServerPort = NULL,*TCP_Type = NULL;
		   int32 TcpServerIP_Len = 0,TcpServerPort_Len = 0;
		   TcpServerIP_Point =  strstr( Point2, "&TCP_Server_IP=");
		   TcpServerPort =  strstr( Point2, "&TCP_Server_Port1=");
		   TCP_Type =  strstr( Point2, "&Tcp_Type=");
		   if((TcpServerIP_Point != NULL) && (TcpServerPort != NULL) && (TCP_Type != NULL))
		   {
			   TcpServerIP_Point = TcpServerIP_Point + strlen("&TCP_Server_IP=");
			   TcpServerIP_Len = Point1 - TcpServerIP_Point;  /*获取TCP服务器IP长度 */
			   //os_printf("TcpServerIP_Len = %d\n",TcpServerIP_Len);

			   TcpServerPort = TcpServerPort + strlen("&TCP_Server_Port1=");
			   TcpServerPort_Len = TCP_Type - TcpServerPort;/*获取TCP服务器端口长度 */
			   //os_printf("TcpServerPort_Len = %d\n",TcpServerPort_Len);
			   if(TcpServerPort_Len <= 0)
			   {
				   return;
			   }
			   if((TcpServerIP_Len >= 7) && (TcpServerIP_Len < 32))
			   {

					char TcpPortTemp[10];
					int32_t TcpPort = 0,TcpType = 0;
				    uint8_t i = 0,isDnsTcpIP = 0;
					for(i = 0; i < TcpServerIP_Len;i ++)
					{
						if(((TcpServerIP_Point[i] - 0x30) >= 0) && ((TcpServerIP_Point[i] - 0x30) <= 9 ) )
						{

						}
						else if(TcpServerIP_Point[i] == '.')
						{

						}
						else
						{
							isDnsTcpIP = 1;
							break;
						}
					}

                    os_memcpy(TcpPortTemp,TcpServerPort, TcpServerPort_Len); /* 拷贝信息 */
					TcpPortTemp[TcpServerPort_Len] = '\0';
					TcpPort = atoi(TcpPortTemp);
					if(TcpPort > 65535 || TcpPort <= 0)
					{
							return;
					}
				    //os_printf("TcpPort = %d\n",TcpPort);
					TCP_Type = TCP_Type + strlen("&Tcp_Type=");
					TcpType = (char)TCP_Type[0];

					if(isDnsTcpIP == 1) /* 代表TCP IP是域名，需要先获取IP地址*/
					{
						uint8 i;
						uint32 DnsBuffer1[32];
						os_bzero(DnsBuffer,32);
						os_memcpy(DnsBuffer,TcpServerIP_Point, TcpServerIP_Len); /* 拷贝DNS信息 */
                        for(i = 0; i < TcpServerIP_Len; i ++)
                        {
                        	DnsBuffer1[i] = (uint32)TcpServerIP_Point[i];
                        }
                    	spi_flash_erase_sector (TCP_SERVERIP_Erase);/* 擦出数据 */
                    	spi_flash_write (TCP_SERVERIP_Erase*4*1024 + TCP_SERVERIP_ERASE_OFFSET, DnsBuffer1, TcpServerIP_Len * 4);/*存入TCPIP域名*/
                    	spi_flash_write (TCP_SERVERIP_Erase*4*1024 + TCP_SERVERIPLen_ERASE_OFFSET, &TcpServerIP_Len, 1 * 4);/*存入TCPIP域名长度*/
                    	spi_flash_write (TCP_SERVERIP_Erase*4*1024 + TCP_SERVER_PORT_ERASE_OFFSET, &TcpPort, 1 * 4);/*存入TCPIP域名端口*/
                    	spi_flash_write (TCP_SERVERIP_Erase*4*1024 + TCP_SERVER_TPYE_ERASE_OFFSET, &TcpType, 1 * 4);/*存入TCPIP域名长连接还是短连接*/

						os_timer_disarm(&TcpClientLink); /* 开启定时器检测是否获取到动态IP地址 */
						os_timer_setfn(&TcpClientLink, (os_timer_func_t *)IsTcpClientLink, NULL);
						os_timer_arm(&TcpClientLink, 100, 0);

					}
					else
					{

						 if(IsIpaddr(TcpServerIP_Point,TcpServerIP_Len,IPTYPE) == 1)/*代表是合法的TCP IP地址 */
		                 {
								uint32 TcpIpBuf[4] = {0,0,0,0};      //用以存储解析好的IP
								GetIPData(TcpIpBuf,TcpServerIP_Point);/*把TCPIp数据转换为数字，存到数组中 */
								//os_printf("%d.%d.%d.%d\n",TcpIpBuf[0],TcpIpBuf[1],TcpIpBuf[2],TcpIpBuf[3]);
                                for(i = 0; i < 4; i ++)
                                {
                                	TcpClientToServer[i] = (char)TcpIpBuf[i];/* 存入IP缓存区 */
                                }
                                if(TcpType != 'L')
                                {  /*若是短连接，则在 串口中断中连接 */

                                }
                                else
                                {
									os_timer_disarm(&TcpClientLink); /* 开启定时器检测是否获取到动态IP地址 */
									os_timer_setfn(&TcpClientLink, (os_timer_func_t *)IsTcpClientLink, NULL);
									os_timer_arm(&TcpClientLink, 100, 0);
                                }

                                TcpServerIP_Len = 4;
                            	spi_flash_erase_sector (TCP_SERVERIP_Erase);/* 擦出数据 */
                            	spi_flash_write (TCP_SERVERIP_Erase*4*1024 + TCP_SERVERIP_ERASE_OFFSET, TcpIpBuf, 4 * 4);/*存入TCPIP域名*/
                            	spi_flash_write (TCP_SERVERIP_Erase*4*1024 + TCP_SERVERIPLen_ERASE_OFFSET, &TcpServerIP_Len, 1 * 4);/*存入TCPIP域名长度*/
                            	spi_flash_write (TCP_SERVERIP_Erase*4*1024 + TCP_SERVER_PORT_ERASE_OFFSET, &TcpPort, 1 * 4);/*存入TCPIP域名端口*/
                            	spi_flash_write (TCP_SERVERIP_Erase*4*1024 + TCP_SERVER_TPYE_ERASE_OFFSET, &TcpType, 1 * 4);/*存入TCPIP域名长连接还是短连接*/

		                 }

					}
			   }
		   }
		   else
		   {

		   }

	   }
	   else
	   {
		   uint32 TcpServerIP_Len = 0;
		   espconn_disconnect(&tcpcilent_esp_conn);
		   spi_flash_erase_sector (TCP_SERVERIP_Erase);/* 擦出数据 */
		   spi_flash_write (TCP_SERVERIP_Erase*4*1024 + TCP_SERVERIPLen_ERASE_OFFSET, &TcpServerIP_Len, 1 * 4);
		   return;
	   }
}
void ICACHE_FLASH_ATTR ChangelanguageState(char * languagePoint)
{
	uint32 language_state = 0;
	if(languagePoint[0] == 'E')
	{
		language_state = LANGUAGE_EN;
		spi_flash_erase_sector (LANGUAGE_Erase);/* 擦出数据,初始化网页为英文版本  */
		spi_flash_write (LANGUAGE_Erase*4*1024 + LANGUAGE_ERASE_OFFSET, &language_state, 1 * 4);
	}
	else
	{
		language_state = LANGUAGE_CH;
		spi_flash_erase_sector (LANGUAGE_Erase);/* 擦出数据,初始化网页为英文版本  */
		spi_flash_write (LANGUAGE_Erase*4*1024 + LANGUAGE_ERASE_OFFSET, &language_state, 1 * 4);
	}
}
void ICACHE_FLASH_ATTR ReturnAckWeb(void *arg, uint8 web_state)
{
	uint32 httphead_len = 0,web_len = 0,Language_state = 0;
    uint8 ganxinNTPshizhongshezhi_Ch[30] = {0xe8,0xb5,0xa3,0xe9,0x91,0xab,\
    		'N','T','P',0xe6,0x97,0xb6,0xe9,0x92,0x9f,0xe8,0xae,0xbe,0xe7,0xbd,0xae,0};/*赣鑫NTP时钟设置*/
    uint8 shujucuowuqingchongxinshuru_ch[50] = {0xe6,0x95,0xb0,0xe6,0x8d,0xae,0xe9,0x94,0x99,0xe8,0xaf,0xaf,
    		0xef,0xbc,0x8c,0xe8,0xaf,0xb7,0xe9,0x87,0x8d,0xe6,0x96,0xb0,0xe8,
			0xbe,0x93,0xe5,0x85,0xa5,0xef,0xbc,0x81,0};/*数据错误，请重新输入*/
    uint8 shezhichenggong_Ch[24] = {0xe8,0xae,0xbe,0xe7,0xbd,0xae,0xe6,0x88,0x90,0xe5,0x8a,0x9f,0}; /*设置成功*/
    uint8 shezhichenggong_En[] = "Successful Setup!";
    uint8 shujucuowuqingchongxinshuru_en[] = "Data Error, Please Input Again!";
    uint8 ganxinNTPshizhongshezhi_En[] = "GX-NTP Clock Setup";
    uint8 *Gx_Title = NULL;
    uint8 *shujucuowu_point = NULL;
    uint8 *shezhichenggong_point = NULL;
	struct espconn *pespconn = arg;
	char send_buff[40];
	send_buff[0] = '\0';

	 Spi_FlashRead(LANGUAGE_Erase,LANGUAGE_ERASE_OFFSET,&Language_state,1);/* 语言版本 */
	 if(Language_state != LANGUAGE_CH)
	 {
		 Gx_Title = ganxinNTPshizhongshezhi_En;
		 shujucuowu_point = shujucuowuqingchongxinshuru_en;
		 shezhichenggong_point = shezhichenggong_En;
	 }
	 else
	 {
		 Gx_Title = ganxinNTPshizhongshezhi_Ch;
		 shujucuowu_point = shujucuowuqingchongxinshuru_ch;
		 shezhichenggong_point = shezhichenggong_Ch;
	 }
	switch(web_state)
	{
	case WEB_SET_SUCCESS:
		   os_sprintf(send_buff,shezhichenggong_point);//发送数据成功更改的提示网页
         break;
	case WEB_SET_FAIL: //发送数据更改失败的提示网页
		   os_sprintf(send_buff,shujucuowu_point);//发送数据成功更改的提示网页
         break;
	default:break;
	}

	 tempSaveData[0] = '\0';
	 //os_bzero(tempSaveData,FLASH_READ_SIZE);
	 web_len = os_sprintf(tempSaveData,HTML_RETURN_OK,Gx_Title,send_buff);
	 HttpHead[0] = '\0';
	 httphead_len = os_sprintf(HttpHead,
	 "HTTP/1.0 200 OK\r\n"
	 "Server: lwIP/1.4.0\r\n"
	 "Content-Type:text/html\r\n"
	 "Connection: keep-alive\r\n"
	 "Content-Length:%d\r\n\r\n",
	  web_len);
	  tempSaveData[0] = '\0';  /* 清零 */
	  web_len = os_sprintf(tempSaveData,HttpHead,httphead_len);
	  web_len += os_sprintf(tempSaveData + web_len,HTML_RETURN_OK,Gx_Title,send_buff);	 /*计算数据长度 = 网页长度 + 头长度*/
	  espconn_sent(pespconn, tempSaveData, web_len);
}

/*-------------------------------------------------------------*/
/*函数功能：接收到数据的回调函数                                                                      */
/*参       数：arg：指向espconn的指针                                                                */
/*参       数：pusrdata：收到的数据                                                                    */
/*参       数：length：数据量                                                                               */
/*返       回：无                                                                                                     */
/*-------------------------------------------------------------*/
extern uint8 FirstSetInf;
extern uint8 LastChannel;
 void ICACHE_FLASH_ATTR tcp_server_recv_cb(void *arg, char *pusrdata, unsigned short length)
{
	static struct ip_info info;
	static uint8_t state;
	char *DHCP_point = NULL,*IP_point = NULL,*Gw_point = NULL,*Sub_point = NULL,PswdLed = 0,IP_Len = 0,Sub_Len = 0,Gw_Len = 0;
	char *LengTh_Point = NULL,*DstHourS_Point = NULL ,*DstHourE_Point = NULL,*DstWeekSeleE_Point = NULL;
	char *Bright_Point = NULL,*DstMonS_Point = NULL,*DstWeekE_Point,*DstMonE_Point = NULL,*Language_Point = NULL;
	char *Save_Point = NULL,*DstWeekSeleS_Point = NULL,*DstWeekS_Point = NULL,*Auto_Web = NULL;
	char *Temp_Point = NULL,*Temp_Point1 = NULL,*APName_Point = NULL,*APPswd_Point = NULL,*Timezone_Point = NULL;
	char *Ntpsever_Point = NULL,*Setup_Point = NULL,*Post_Point = NULL,*TcpPort = NULL,TcpPortLen = 0;
	char *TimeOutInterva = NULL,* SSID_Point = NULL,*Pswd_Point = NULL,*kjLevel_Point = NULL,*TcpServerIP_Point = NULL;
	uint32 i = 0,IPaddrLen = 0,SubMaskLen = 0,AllDataLen = 0,GatewayLen = 0,LengTh = 0,SSIDlen = 0,PSWDlen = 0;
	uint32	APName_len = 0,TimeIntervaLen = 0,APPswd_len = 0,hourStartLen = 0,hourEndLen = 0,Timezone_Len = 0,NtpServerLen = 0;
    struct espconn *pespconn = arg;
    //char *tempSaveData = NULL;

  // os_printf("Received data: %s \r\n", pusrdata);    //打印收到的数据



   Post_Point = strstr( pusrdata, "POST / HTTP/");

   Auto_Web =  strstr( pusrdata, "Auto_Web=");

   APName_Point = strstr( pusrdata, "AP_Name=");
   APPswd_Point = strstr( pusrdata, "&AP_Password=");

   DHCP_point = strstr( pusrdata, "&DHCP_State=");

   IP_point = strstr( pusrdata, "&IP_addr=");
   Sub_point = strstr( pusrdata, "&Subnet_mask=");
   Gw_point = strstr( pusrdata, "&Gateway=");

   TcpPort = strstr( pusrdata, "&TCP_Server_Port=");

   Timezone_Point = strstr( pusrdata, "&Timezone=UTC");

   //Bright_Point = strstr( pusrdata, "&Brightness=");

   DstHourS_Point = strstr( pusrdata, "&hourStart=");
   DstHourE_Point = strstr( pusrdata, "&hourEnd=");

   DstWeekSeleS_Point = strstr( pusrdata, "&WeekendSelectStart=");
   DstWeekSeleE_Point = strstr( pusrdata, "&WeekendSelectEnd=");

   DstWeekS_Point = strstr( pusrdata, "&DST_Week_Start=");
   DstWeekE_Point = strstr( pusrdata, "&DST_Week_End=");

   DstMonS_Point = strstr( pusrdata, "&DST_Mon_Start=");
   DstMonE_Point = strstr( pusrdata, "&DST_Mon_End=");

   SSID_Point = strstr( pusrdata, "&SSID=");
   Pswd_Point = strstr( pusrdata, "&Password=");

   Save_Point = strstr( pusrdata, "&Save=Save");
   Ntpsever_Point =  strstr( pusrdata, "&Ntp_Serve=");

   TimeOutInterva = strstr( pusrdata, "&Time_Output_Interva=");
   Setup_Point = strstr( pusrdata, "Successful_setup=Return");
   Temp_Point = Timezone_Point; //用于获取NTP服务器IP长度做变量存储
   kjLevel_Point = strstr( pusrdata, "&kjLevel=on&");
   TcpServerIP_Point= strstr( pusrdata, "&TCP_Server_IP=");
   Language_Point = strstr( pusrdata, "&language=");
//
   if((strstr( pusrdata, "GET /") != NULL) || (Setup_Point != NULL) || (strstr( pusrdata, "&Refresh=Refresh") != NULL))   //代表有网页请求
   {
	   int len1 = 0;
	   Init_Setup_timer = 3000;
	   //user_scan();
	   SetSendWeb(&AlreadySetData); /* 把参数配置到网页中 */
	   HttpHead[0] = '\0';
	   os_bzero(HttpHead,200);
		len1 += os_sprintf(HttpHead,
				"HTTP/1.0 200 OK\r\n"
				"Server: lwIP/1.4.0\r\n"
				"Content-Type:text/html\r\n"
				"Connection: keep-alive\r\n"
				"Content-Length:%d\r\n\r\n",
				len);
		FirstLinkOK = 1;
		pespconn -> user_reverse = 1;
		espconn_sent(pespconn, HttpHead, len1);

   }
   else  if    ((DstHourS_Point != NULL) && (DstHourE_Point != NULL) \
			   && (DstWeekSeleS_Point != NULL) && (DstWeekSeleE_Point != NULL)\
			   && (DstWeekS_Point != NULL) && (DstWeekE_Point != NULL)\
			   && (DstMonS_Point != NULL) && (DstMonE_Point != NULL)\
			   && (Save_Point != NULL) && (TimeOutInterva != NULL) \
			   && (Ntpsever_Point != NULL) &&(Timezone_Point != NULL) \
			   && (IP_point != NULL) &&(Sub_point != NULL) &&(Pswd_Point != NULL) \
			   && (Gw_point != NULL) &&(DHCP_point != NULL) &&(Language_Point != NULL) \
			   && (SSID_Point != NULL)&&(Auto_Web != NULL) &&(TcpServerIP_Point != NULL) \
			    &&(APName_Point != NULL ) &&(TcpPort != NULL) \
			   && (APPswd_Point != NULL))
	   {
	   //os_printf("Received data: %s \r\n", pusrdata);
        //return;
		   APName_Point = APName_Point + 8;
		   APName_len = APPswd_Point - APName_Point;  //获取AP名字的长度

		   APPswd_Point = APPswd_Point + 13;
		   APPswd_len = DHCP_point - APPswd_Point;

		   DHCP_point = DHCP_point + strlen("&DHCP_State=");

		   IP_point = IP_point + strlen("&IP_addr=");
		   IP_Len = Sub_point - IP_point;

		   Sub_point = Sub_point + strlen("&Subnet_mask=");
		   Sub_Len = Gw_point - Sub_point;

		   Gw_point = Gw_point + strlen("&Gateway=");
		   Gw_Len = TcpPort - Gw_point;

		   TcpPort = TcpPort + strlen("&TCP_Server_Port=");
		   TcpPortLen = TcpServerIP_Point - TcpPort;

		   Ntpsever_Point = Ntpsever_Point + 11;
		   NtpServerLen = TimeOutInterva - Ntpsever_Point;

		   TimeOutInterva = TimeOutInterva + strlen("&Time_Output_Interva=");
		   TimeIntervaLen = Timezone_Point - TimeOutInterva;

		   Timezone_Point = Timezone_Point + strlen("&Timezone=UTC");
		   Timezone_Len = DstHourS_Point - Timezone_Point;

		   DstHourS_Point = DstHourS_Point + strlen("&hourStart=");
		   DstHourE_Point = DstHourE_Point + strlen("&hourEnd=");

		   DstWeekSeleS_Point = DstWeekSeleS_Point + strlen("&WeekendSelectStart=");
		   DstWeekSeleE_Point = DstWeekSeleE_Point + strlen("&WeekendSelectEnd=");

		   DstWeekS_Point = DstWeekS_Point + strlen("&DST_Week_Start=");
		   DstWeekE_Point = DstWeekE_Point + strlen("&DST_Week_End=");

		   DstMonS_Point = DstMonS_Point + strlen("&DST_Mon_Start=");
		   DstMonE_Point = DstMonE_Point + strlen("&DST_Mon_End=");

		   SSID_Point = SSID_Point + strlen("&SSID=");     //指向SSID的数据首字母
		   SSIDlen = Pswd_Point - SSID_Point;  //获取SSID字符长度

		   Pswd_Point = Pswd_Point + strlen("&Password=");     //指向PASW的数据首字母
		   PswdLed = Auto_Web - Pswd_Point - 1;  //获取PSWD字符长度

		   Auto_Web =  Auto_Web + strlen("Auto_Web=");  /* 指向是否自动弹出网页的状态值 */

		   Language_Point = Language_Point + strlen("&language=");  /* 指向语言版本信息的值  */
		   //把本地相关信息存入flash
		 if((APName_len > 0) && (APPswd_len >= 0) && (NtpServerLen >= 7) && (SSIDlen > 0) && (SSIDlen <= 32) &&
				 (TimeIntervaLen > 0 ) && (TimeIntervaLen <= 4 ) &&(TcpPortLen > 0)  &&(TcpPortLen <= 5))
		 {
			//os_printf("Successfur SetUp\n");
           uint32 len1 = 0;
           struct softap_config config1;

          SetDynamic_Static_IP(DHCP_point,IP_point,IP_Len,Sub_point,Sub_Len, Gw_point,Gw_Len); /*修改动态或静态IP信息*/
		  if( GetAPname_pswd_to_LingAP(APName_Point,APPswd_Point,APName_len,APPswd_len) == 1)  /*  连接网络 */
		  {
				 spi_flash_read(SEVER5_FLASH_Erase * 4096, (uint32 *) &tempSaveData, FLASH_READ_SIZE4);
				 espconn_sent(pespconn, tempSaveData, FLASH_READ_SIZE4);
				 return;
		  }
		   ChangeTcpPort(TcpPort,TcpPortLen);  /* 修改TCP服务器端口 */
		   ChangeTcpClient(kjLevel_Point,pusrdata);/* 修改TCP客户端数据 */
		   wifi_softap_get_config(&config1);       //获取参数
           LastChannel = config1.channel;
           SetAutoWeb(Auto_Web); /*  设置是否自动弹出网页 */
		   SetTimezone(Timezone_Point,Timezone_Len);  //设置时区
		   DstSet(DstHourS_Point,DstHourE_Point,DstWeekSeleS_Point,DstWeekSeleE_Point,DstWeekS_Point,DstWeekE_Point,DstMonS_Point,DstMonE_Point); // 夏令时设置
		   ChangeTimeOutInterva(TimeOutInterva,TimeIntervaLen);
		   SetNtpServer(Ntpsever_Point,NtpServerLen);
		   ChangelanguageState(Language_Point); /*更改网页语言版本*/

		   SetSSID(SSID_Point,SSIDlen,Pswd_Point,PswdLed); /* 更改设备SSID和PSWD */

		   Spi_FlashRead(FIRST_START_Erase,FIRST_START_ERASE_OFFSET,&SetDevideCount,1);
		   if(SetDevideCount < 0)
			{
			  SetDevideCount = 0;
			}
		   SetDevideCount ++; /*更改设备的次数存入 */
		   if(SetDevideCount >= 100000) /* 防溢出*/
		   {
			   SetDevideCount = 2;
		   }
		   spi_flash_erase_sector (FIRST_START_Erase);
		   spi_flash_write (FIRST_START_Erase*4*1024 + FIRST_START_ERASE_OFFSET, &SetDevideCount, 1 * 4);

		   Init_Setup = INIT_NO_SET;     //下次启动时，代表数据已经更改，不需要恢复到出厂设置
		   FirstSetInf = 1;
		   Spi_FlashWrite(Init_Erase,0, &Init_Setup, 1);
		   ReturnAckWeb(arg,WEB_SET_SUCCESS);  /*返回提示网页*/
		 }
		 else
		 {
			 //os_printf("data error\n");
			 //发送出现数据错误的提示网页
			 ReturnAckWeb(arg,WEB_SET_FAIL);  /*返回提示网页*/

		 }
	   }
   else
   {
	   if(//(strstr( pusrdata, "POST /") != NULL) &&
			   ((strstr( pusrdata, "captive.apple") != NULL)||
			   (strstr( pusrdata, "connect.rom.miui") != NULL)||
			   (strstr( pusrdata, "Safari") != NULL)||
			   (strstr( pusrdata, "192.168.1.100") != NULL)||
			   (strstr( pusrdata, "clients3.google") != NULL)))
	   {

	   }
	   else
	   {
		 //os_printf("duankai\n");
	     espconn_disconnect(pespconn);
	   }
//	   	spi_flash_read(SEVER3_FLASH_Erase * 4096, (uint32 *) &tempSaveData, FLASH_READ_SIZE2); //发送出现未知错误的提示网页
//	   	espconn_sent(pespconn, tempSaveData, FLASH_READ_SIZE2);
//		 spi_flash_read(SEVER5_FLASH_Erase * 4096, (uint32 *) &tempSaveData, FLASH_READ_SIZE4);
//		 espconn_sent(pespconn, tempSaveData, FLASH_READ_SIZE4);

   }

}
