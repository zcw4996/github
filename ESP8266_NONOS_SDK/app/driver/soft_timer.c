/*
 * soft_timer.c
 *
 *  Created on: 2019��7��28��
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
os_timer_t Init_timer,Check_SmartConfig,test_timer,sntpPutTime,LedCloseTime,TcpClientLink;  //����һ����ʱ���ṹ
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

	 Spi_FlashRead(NTP_IP_Erase,NTP_IP_LEN_ERASE_OFFSET,&NtpIpLen,1);  //��ȡNTP IP�ĳ���
	 if((NtpIpLen <= 0) && (NtpIpLen >= 32))
	 {
		//os_printf("NtpIpLen= %d\n",NtpIpLen);
	 	return ;
	 }
	 else
	 {
		 //os_printf("NtpIpLen= %d\n",NtpIpLen);
		 Spi_FlashRead(NTP_IP_Erase,NTP_IP_ERASE_OFFSET,Ntpsever,NtpIpLen);  //��ȡNTP IP
		 Spi_FlashRead(TIMEZONE_ERASE,TIMEZONE_ERASE_OFFSET,&Ntp_Timezone,1);   //��ȡʱ��
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
		wifi_softap_get_config(&config1);       //��ȡ����
		if(FirstSetInf == 1)
		{
		    Spi_FlashRead(FIRST_START_Erase,FIRST_START_ERASE_OFFSET,&SetDevideCount,1);
		    if(SetDevideCount > 1)
		    {
				if(config1.channel != LastChannel ) /* ����޸����ݺ� �������ӵ������ŵ�����һ�εĲ�һ�� Ϊ�˱���ƻ���豸���Ӳ��ϣ�������*/
				{
					system_restart();
				}
		    }
		}
	    LastChannel = config1.channel;
		wifi_get_ip_info(STATION_IF, &dynamic_ipconfig);  //��ȡ��̬IP
		ApLink = wifi_station_get_connect_status();
	    text = wifi_softap_get_station_num();
		SysTime_ms = 0;
		if( text >= 1)
		{
			if(AlreadStartPutTime == 0)
			{
				switch(LedState)   /*��û�л�ȡ��NTP�����������ݣ�����˸�� */
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
				switch(LedState)   /*��û�л�ȡ��NTP�����������ݣ�����˸�� */
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
	 case 0:if(ApLink != 5)   //û�з��䵽IP��ַ��˵��û��������WIFI�����¼��
			{
		       timerForLinkAP = timerForLinkAP + 500;
				if(timerForLinkAP >= 8000) //���8�뻹δ���ӵ�WIFI�����л�AP ID����������
				{
					timerForLinkAP = 0;
					Spi_FlashRead(AP_NUM_Erase,AP_NUM_ERASE_OFFSET,&APid,1);  //��FLASH�л�ȡAP_ID
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
////					  spi_flash_write(AP_NUM_Erase*4*1024 + AP_NUM_ERASE_OFFSET,&APid,1 * 4);   //��¼�ͻ����õ�APid
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
	 case 1:if(ApLink != 5)   //û�з��䵽IP��ַ��˵��û��������WIFI�����¼��
			{

	            LinkAPstate = 0;   //���Ͽ������� ����ص���һ��
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
/*�������ܣ���ѯIP��ȡ����Ļص�����                                                               */
/*��       ������                                                                                                     */
/*��       �أ���                                                                                                     */
/*-------------------------------------------------------------*/

void ICACHE_FLASH_ATTR user_check_ip(void)
{
    struct ip_info Static_ipconfig,dynamic_ipconfig;

    wifi_get_ip_info(SOFTAP_IF, &Static_ipconfig);    //��ȡAPģʽ�� IP��Ϣ�����浽ipconfig
	wifi_get_ip_info(STATION_IF, &dynamic_ipconfig);  //��ȡ��̬IP

    if (Static_ipconfig.ip.addr != 0)                 //���ò�ѯ ״̬������ipconfig�ṹ����ipֵ��0����if����
    {
    	//os_printf("IP = %d\n",Static_ipconfig.ip.addr);
        uint32 APP_TCPPort = 0;
    	ipv4_to_str(SOFTAP_IF,Static_ipconfig.ip.addr);  //��ӡ��̬IP
    	ipv4_to_str(STATION_IF,dynamic_ipconfig.ip.addr); //��ӡ��̬IP
        user_dns_server_init();
		//os_printf("Get IP successful\r\n");    //��ʾ��ȡIP�ɹ�
        espconn_tcp_set_max_con(8);
       // AP_tcpclient_init();                   /* �ͻ��˳�ʼ�� */
		AP_tcpserver_init(SERVER_PORT);          //��������ʼ��
		Spi_FlashRead(TCP_PORT_Erase,TCP_PORT_ERASE_OFFSET,&APP_TCPPort,1);
		if(APP_TCPPort <= 0 || APP_TCPPort > 65535)
		{
	       APP_TCPPort = 50000;
		}
		AP_tcpserverAPP_init(APP_TCPPort);
		//AP_udpclient_init();                   /* UDP��������ʼ�� */

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
	if(FirstIn == 0)  /* ��һ�ν��� ��ɨ��һ��AP������ʱɨ��*/
	{
	  FirstIn = 1;
	  user_scan();  /* ɨ��AP��Ϣ */
	}

	if(wifi_softap_get_station_num() >= 1) /*�����豸���ӣ���8S���һ��AP��Ϣ */
	{
		if(Init_Setup_timer >= 8000)
		{
		    //os_printf("system_get_free_heap_size() = %d\n",system_get_free_heap_size());
			Init_Setup_timer = 0;
			user_scan();
		}
	}
	else     /*�����豸���ӣ���20S���һ��AP��Ϣ */
	{
		if(Init_Setup_timer >= 20000)
		{
			Init_Setup_timer = 0;
			user_scan();
			if(system_get_free_heap_size() <= 500) /* �������ڴ�С��500�ֽ� ������ */
			{
				system_restart();      //����
			}
		}
	}

	if(GPIO_INPUT_GET(GPIO_ID_PIN(12)) == 0x00)  //�������³���0.5�룬��ָ���������
    {
		Restart_Timer = Restart_Timer + 100;
    	if(Restart_Timer >= 500)
    	{
			 Init_Setup = INIT_SET;
			 Spi_FlashWrite(Init_Erase, 0,&Init_Setup, 1);
			 os_printf("Restart\n");
			 system_restore();
			 system_restart();      //����
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

	wifi_get_ip_info(STATION_IF, &dynamic_ipconfig);  //��ȡ��̬IP

    if (dynamic_ipconfig.ip.addr != 0)                 //���ò�ѯ ״̬������ipconfig�ṹ����ipֵ��0����if����
    {
		int32_t TcpPort = 0,TcpServerLen = 0;
    	Spi_FlashRead(TCP_SERVERIP_Erase,TCP_SERVER_PORT_ERASE_OFFSET, &TcpPort,1);
    	Spi_FlashRead(TCP_SERVERIP_Erase,TCP_SERVERIPLen_ERASE_OFFSET, &TcpServerLen,1);
        if(TcpServerLen == 4)  /*˵����IP��ַ  ֱ�����Ӽ��� */
        {
           AP_tcpclient_init(TcpClientToServer,TcpPort);

        }
        else if(TcpServerLen >= 7) /*����DNS������Ҫ�⿪������IP ������TCP������ */
        {
    	   espconn_gethostbyname(&Dns_Client_conn,DnsBuffer,&esp_server_ip,user_esp_platform_dns_found);
        }
    	   os_timer_disarm(&TcpClientLink); /* ȡ����ʱ�� */
    	   //os_printf("quxiaodingshiqi\n");

    }
    else
    {
		os_timer_setfn(&TcpClientLink, (os_timer_func_t *)IsTcpClientLink, NULL);
		os_timer_arm(&TcpClientLink, 100, 0);
    }
}
