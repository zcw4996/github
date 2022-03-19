/*
 * soft_timer.c
 *
 *  Created on: 2019年7月28日
 *      Author: Administrator
 */
#include "udpclient.h"
#include "soft_timer.h"
#include "tcpserver.h"
#include "Rgbled.h"
#include "mySmartlink.h"
#include "tcpclient.h"
extern uint8_t LedChoose ;
extern int32 Ntp_Timezone;
os_timer_t Init_timer,Check_SmartConfig,test_timer,sntpPutTime,LedCloseTime,TcpClientLink;  //定义一个定时器结构
int32 Init_Setup = INIT_NO_SET;
extern uint32 If_Init_Set;
uint32_t timerForLinkAP;
uint8 LinkAPstate;
extern uint32 SysTime_ms;
extern  uint8_t isLedClose;

void ICACHE_FLASH_ATTR SetNtp()
{
    int32 timezone1 = 0,NtpIpLen = 0;
    sint8 timezone =0;
	uint32 i, Ntpsever[32];
	char Ntpsever1[32] ;


	 for(i = 0; i < 32; i++)
	  {
	  	Ntpsever[i] = 0;
	  	Ntpsever1[i] = 0;
	  }

	 Spi_FlashRead(NTP_IP_Erase,NTP_IP_LEN_ERASE_OFFSET,&NtpIpLen,1);  //读取NTP IP的长度
	 if((NtpIpLen <= 0) && (NtpIpLen >= 32))
	 {
		//os_printf("NtpIpLen= %d\n",NtpIpLen);
	 	return ;
	 }
	 else
	 {
		 //os_printf("NtpIpLen= %d\n",NtpIpLen);
		 Spi_FlashRead(NTP_IP_Erase,NTP_IP_ERASE_OFFSET,Ntpsever,NtpIpLen);  //读取NTP IP
		 Spi_FlashRead(TIMEZONE_ERASE,TIMEZONE_ERASE_OFFSET,&Ntp_Timezone,1);   //读取时区
		 for(i = 0; i < NtpIpLen; i ++)
		 {
		   Ntpsever1[i] = (char)Ntpsever[i];
		 }
		 Ntpsever1[NtpIpLen] = '\0';
		 sntp_stop();
		 Sntp_Config(Ntpsever1,NtpIpLen);

		 //os_printf("sntp_get_timezone = %d\n", sntp_get_timezone());
	 }

}
uint8 FirstSetInf = 0;
extern uint32 SysTime_ms;
extern uint8 LastChannel;
void ICACHE_FLASH_ATTR Check_Smart_Link(void)
{
    struct ip_info dynamic_ipconfig;
    uint8_t text,ApLink = 0;
    static uint32 Time_ms;
    static uint8 LedState;
    uint32 i,APid = 0;
    struct softap_config config1;
//    uint8 stationCount = wifi_softap_get_station_num();
//    os_printf("stationCount = %d\n", stationCount);
    SysTime_ms = SysTime_ms + 100;
	if(SysTime_ms >= 500)
	{
		wifi_softap_get_config(&config1);       //获取参数
		if(FirstSetInf == 1)
		{
		    Spi_FlashRead(FIRST_START_Erase,FIRST_START_ERASE_OFFSET,&SetDevideCount,1);
		    if(SetDevideCount > 1)
		    {
				if(config1.channel != LastChannel ) /* 如果修改数据后 发现连接的网络信道和上一次的不一样 为了避免苹果设备连接不上，则重启*/
				{
					system_restart();
				}
		    }
		}
	    LastChannel = config1.channel;
		wifi_get_ip_info(STATION_IF, &dynamic_ipconfig);  //获取动态IP
		ApLink = wifi_station_get_connect_status();
	    text = wifi_softap_get_station_num();
		SysTime_ms = 0;
		if( text >= 1)
		{
			if(AlreadStartPutTime == 0)
			{
				switch(LedState)   /*若没有获取到NTP服务器的数据，则闪烁灯 */
				{
					case 0:
							LedRed(0);
							LedGreen(0);
							LedChoose = BLUELED;
							LedOpenChoose(LedChoose);
							LedState = 1;break;
					case 1:
							LedCloseALL();
							LedState = 0;break;
					default:
						   LedState = 0;break;
				}
			}
			else
			{
				LedRed(0);
				LedGreen(0);
				LedChoose = BLUELED;
				LedOpenChoose(LedChoose);
			}

			//os_printf("blue\n");
		}
		else if((text == 0) && ApLink == 5)
		{
			if(AlreadStartPutTime == 0)
			{
				switch(LedState)   /*若没有获取到NTP服务器的数据，则闪烁灯 */
				{
					case 0:
							LedBlue(0);
							LedRed(0);
							LedChoose = GREENLED;
							LedOpenChoose(LedChoose);
							LedState = 1;break;
					case 1:
							LedCloseALL();
							LedState = 0;break;
					default:
						   LedState = 0;break;
				}
			}
			else
			{
				LedBlue(0);
				LedRed(0);
				LedChoose = GREENLED;
				LedOpenChoose(LedChoose);
			}
			//os_printf("GREENLED\n");
		}
		else if((text == 0) && ApLink != 5)
		{
			Time_ms = Time_ms + 500;
			//os_printf("REDLED\n");
			if(Time_ms >= 1000)
			{
				Time_ms = 0;
				switch(LedState)
				{
					case 0:LedState = 1;
						   LedBlue(0);
						   LedGreen(0);
						   LedChoose = REDLED;
						   LedOpenChoose(LedChoose);
						   break;
					case 1:LedState = 0;
						   LedBlue(0);
						   LedGreen(0);
						   LedRed(0);
							break;
					default:LedState = 0;break;
				}


			}
		}


	switch(LinkAPstate)
	{
	 case 0:if(ApLink != 5)   //没有分配到IP地址，说明没有连接上WIFI，重新检测
			{
		       timerForLinkAP = timerForLinkAP + 500;
				if(timerForLinkAP >= 8000) //如果8秒还未连接到WIFI，则切换AP ID，继续连接
				{
					timerForLinkAP = 0;
					Spi_FlashRead(AP_NUM_Erase,AP_NUM_ERASE_OFFSET,&APid,1);  //从FLASH中获取AP_ID
//					switch(APid)
//					{
//						case APID1:  //os_printf("APid = APID2;\n");
//						             APid = APID2;
//									 break;
//						case APID2:  //os_printf("APid = APID1;\n");
//							         APid = APID1;
//									 break;
//						default:     //os_printf("APid = default;\n");
//						             break;
//					}
//					if((APid == APID1) || (APid == APID2))
//					{
					      vLink_AP(APid);
////					  spi_flash_erase_sector (AP_NUM_Erase);
////					  spi_flash_write(AP_NUM_Erase*4*1024 + AP_NUM_ERASE_OFFSET,&APid,1 * 4);   //记录客户配置的APid
//					}
				}

			}
			   else
			{
				timerForLinkAP = 0;
				//os_printf("yilianjie\n");
				SetNtp();
				LinkAPstate = 1;
			}
	                       break;
	 case 1:if(ApLink != 5)   //没有分配到IP地址，说明没有连接上WIFI，重新检测
			{

	            LinkAPstate = 0;   //若断开了连接 ，则回到第一步
			}
			 else
			 {
              // os_printf("baochilianjie\n");
			 }
	                       break;
	 default:LinkAPstate = 1;break;
	}
	}


}
/*-------------------------------------------------------------*/
/*函数功能：查询IP获取结果的回调函数                                                               */
/*参       数：无                                                                                                     */
/*返       回：无                                                                                                     */
/*-------------------------------------------------------------*/

void ICACHE_FLASH_ATTR user_check_ip(void)
{
    struct ip_info Static_ipconfig,dynamic_ipconfig;

    wifi_get_ip_info(SOFTAP_IF, &Static_ipconfig);    //获取AP模式下 IP信息，保存到ipconfig
	wifi_get_ip_info(STATION_IF, &dynamic_ipconfig);  //获取动态IP

    if (Static_ipconfig.ip.addr != 0)                 //调用查询 状态函数，ipconfig结构体中ip值非0，则if成立
    {
    	//os_printf("IP = %d\n",Static_ipconfig.ip.addr);
        uint32 APP_TCPPort = 0;
    	ipv4_to_str(SOFTAP_IF,Static_ipconfig.ip.addr);  //打印静态IP
    	ipv4_to_str(STATION_IF,dynamic_ipconfig.ip.addr); //打印动态IP
        user_dns_server_init();
		//os_printf("Get IP successful\r\n");    //提示获取IP成功
        espconn_tcp_set_max_con(8);
       // AP_tcpclient_init();                   /* 客户端初始化 */
		AP_tcpserver_init(SERVER_PORT);          //服务器初始化
		Spi_FlashRead(TCP_PORT_Erase,TCP_PORT_ERASE_OFFSET,&APP_TCPPort,1);
		if(APP_TCPPort <= 0 || APP_TCPPort > 65535)
		{
	       APP_TCPPort = 50000;
		}
		AP_tcpserverAPP_init(APP_TCPPort);
		//AP_udpclient_init();                   /* UDP服务器初始化 */

    }
    else
    {
		os_timer_setfn(&test_timer, (os_timer_func_t *)user_check_ip, NULL);
		os_timer_arm(&test_timer, 100, 0);
    }
}
uint32_t Init_Setup_timer = 0;
void ICACHE_FLASH_ATTR Init_Set_up(void)
{
    static uint8 FirstIn;
    static uint32 Restart_Timer;;
    //static uint32_t timer1,Mode;
	Init_Setup_timer = Init_Setup_timer + 100;
	if(FirstIn == 0)  /* 第一次进来 则扫描一次AP，而后定时扫描*/
	{
	  FirstIn = 1;
	  user_scan();  /* 扫描AP信息 */
	}

	if(wifi_softap_get_station_num() >= 1) /*若有设备连接，则8S检测一次AP信息 */
	{
		if(Init_Setup_timer >= 8000)
		{
		    //os_printf("system_get_free_heap_size() = %d\n",system_get_free_heap_size());
			Init_Setup_timer = 0;
			user_scan();
		}
	}
	else     /*若无设备连接，则20S检测一次AP信息 */
	{
		if(Init_Setup_timer >= 20000)
		{
			Init_Setup_timer = 0;
			user_scan();
			if(system_get_free_heap_size() <= 500) /* 若可用内存小于500字节 则重启 */
			{
				system_restart();      //重启
			}
		}
	}

	if(GPIO_INPUT_GET(GPIO_ID_PIN(12)) == 0x00)  //按键按下超过0.5秒，则恢复出厂设置
    {
		Restart_Timer = Restart_Timer + 100;
    	if(Restart_Timer >= 500)
    	{
			 Init_Setup = INIT_SET;
			 Spi_FlashWrite(Init_Erase, 0,&Init_Setup, 1);
			 os_printf("Restart\n");
			 system_restore();
			 system_restart();      //重启
    	}

    }
	else
	{
		Restart_Timer = 0;
	}

}
void ICACHE_FLASH_ATTR IsTcpClientLink(void)
{
    struct ip_info dynamic_ipconfig;

	wifi_get_ip_info(STATION_IF, &dynamic_ipconfig);  //获取动态IP

    if (dynamic_ipconfig.ip.addr != 0)                 //调用查询 状态函数，ipconfig结构体中ip值非0，则if成立
    {
		int32_t TcpPort = 0,TcpServerLen = 0;
    	Spi_FlashRead(TCP_SERVERIP_Erase,TCP_SERVER_PORT_ERASE_OFFSET, &TcpPort,1);
    	Spi_FlashRead(TCP_SERVERIP_Erase,TCP_SERVERIPLen_ERASE_OFFSET, &TcpServerLen,1);
        if(TcpServerLen == 4)  /*说明是IP地址  直接连接即可 */
        {
           AP_tcpclient_init(TcpClientToServer,TcpPort);

        }
        else if(TcpServerLen >= 7) /*若是DNS，则需要解开域名的IP 再连接TCP服务器 */
        {
    	   espconn_gethostbyname(&Dns_Client_conn,DnsBuffer,&esp_server_ip,user_esp_platform_dns_found);
        }
    	   os_timer_disarm(&TcpClientLink); /* 取消定时器 */
    	   //os_printf("quxiaodingshiqi\n");

    }
    else
    {
		os_timer_setfn(&TcpClientLink, (os_timer_func_t *)IsTcpClientLink, NULL);
		os_timer_arm(&TcpClientLink, 100, 0);
    }
}
