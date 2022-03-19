/*
 * init.c

 *
 *  Created on: 2019年7月28日
 *      Author: Administrator
 */
#include "init.h"
#include "soft_timer.h"
#include "user_config.h"
#include "tcpserver.h"
void ICACHE_FLASH_ATTR user_set_station_config(char *ssid,char *password,uint32 ssidLen,uint32 PswdLen)
{
	struct station_config stationConf;

	 stationConf.bssid_set = 0;  //need not check MAC address of AP
	 os_memcpy(&stationConf.ssid, ssid, APNAMEMAXLEN);
	 os_memcpy(&stationConf.password, password, 64);
	 DNS_SERVER_DEBUG("LinkAgain\n");
	 //if(wifi_station_get_connect_status() == 5)
	 {
	   DNS_SERVER_DEBUG("duankailianjie\n");
	   wifi_station_disconnect();
	 }
	 wifi_station_set_config(&stationConf);
	 wifi_station_connect();
	 timerForLinkAP = 0; /* 网络检测重新计时 */

}
void  ICACHE_FLASH_ATTR ipv4_to_str(uint8 if_index, unsigned int ipv4_addr)
{
	if(if_index == SOFTAP_IF)
	{
     DNS_SERVER_DEBUG("Static_IP = %d.%d.%d.%d\n",ipv4_addr & 0xFF,(ipv4_addr >> 8) & 0xFF,(ipv4_addr >> 16) & 0xFF,(ipv4_addr >> 24) & 0xFF);
	}
	else
	{
     DNS_SERVER_DEBUG("Dynamic_IP = %d.%d.%d.%d\n",ipv4_addr & 0xFF,(ipv4_addr >> 8) & 0xFF,(ipv4_addr >> 16) & 0xFF,(ipv4_addr >> 24) & 0xFF);
	}
}
extern uint8 LastChannel;
void ICACHE_FLASH_ATTR ConfigEsp8266(char *SSID,char *PSWD,uint16 SSIDlen,uint16 PSWDlen,struct ip_info *info1,uint8 opmode)
{
	struct softap_config config1;
	//wifi_set_opmode(opmode);          //设置wifi模式：AP模式

    wifi_softap_get_config(&config1);       //获取参数
    LastChannel = config1.channel; /* 获取信道参数 */
    DNS_SERVER_DEBUG("config1.channel = %d\n",config1.channel);
	os_memset(config1.ssid, 0, 32);         //先清空ssid

	os_memcpy(config1.ssid, SSID, SSIDlen);      //设置ssid
	if(PSWDlen > 0)   /* 需要加密 */
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

	wifi_softap_dhcps_stop();
	if(wifi_set_ip_info(SOFTAP_IF, info1) == 1)        //设置AP模式下IP参数
	{
		DNS_SERVER_DEBUG("SetIP success\n");
	}
	else
	{
		DNS_SERVER_DEBUG("SetIP fail\n");
	}
	wifi_softap_dhcps_start();                 //开启AP模式下DHCP
	os_timer_disarm(&test_timer);
	os_timer_setfn(&test_timer, (os_timer_func_t *)user_check_ip, NULL);
	os_timer_arm(&test_timer, 100, 0);

}

void ICACHE_FLASH_ATTR ConfigEsp82662(struct ip_info *info1,uint8 opmode)
{
	//wifi_set_opmode(STATIONAP_MODE);

	wifi_softap_dhcps_stop();
	if(wifi_set_ip_info(SOFTAP_IF, info1) == 1)        //设置AP模式下IP参数
	{
		DNS_SERVER_DEBUG("SetIP success\n");
	}
	else
	{
		DNS_SERVER_DEBUG("SetIP fail\n");
	}
	wifi_softap_dhcps_start();                 //开启AP模式下DHCP

	os_timer_disarm(&test_timer);
	os_timer_setfn(&test_timer, (os_timer_func_t *)user_check_ip, NULL);
	os_timer_arm(&test_timer, 100, 0);

}
void ICACHE_FLASH_ATTR ConfigEsp82663(struct ip_info *info1)
{
  uint32 DHCPstate;

  Spi_FlashRead(NO_DHCP_Erase,DHCP_OPEN_ERASE_OFFSET,&DHCPstate,1);
	if(DHCPstate != DHCP_CLOSE)
	{
		wifi_station_dhcpc_start();
	}
	else
	{
		struct ip_info info;
		uint32 IP_addr[4] = {0,0,0,0};      //用以存储解析好的IP地址
		uint32 Subnet_mask[4] = {0,0,0,0};  //用以存储解析好的子网掩码
		uint32 Gateway[4] = {0,0,0,0};      //用以存储解析好的网管

		Spi_FlashRead(NO_DHCP_Erase,NO_DHCP_IP_ERASE_OFFSET,IP_addr,4);  //从flash中读取本地IP 网关 子网掩码
		Spi_FlashRead(NO_DHCP_Erase,NO_DHCP_Mask_ERASE_OFFSET,Subnet_mask,4);
		Spi_FlashRead(NO_DHCP_Erase,NO_DHCP_GATAWAY_ERASE_OFFSET,Gateway,4);

		IP4_ADDR(&info.ip, IP_addr[0], IP_addr[1], IP_addr[2], IP_addr[3]);       //设置IP
		IP4_ADDR(&info.gw, Gateway[0], Gateway[1], Gateway[2], Gateway[3]);	      //设置网关
		IP4_ADDR(&info.netmask, Subnet_mask[0], Subnet_mask[1], Subnet_mask[2], Subnet_mask[3]); //设置子网掩码
		wifi_station_dhcpc_stop();
		wifi_set_ip_info(STATION_IF,&info);
	}

}

void ICACHE_FLASH_ATTR Get_APid_Link_AP()
{
	uint32 APID = 0;
	Spi_FlashRead(AP_NUM_Erase,AP_NUM_ERASE_OFFSET,&APID,1);  //从FLASH中获取AP_ID
	vLink_AP(APID);
}
void ICACHE_FLASH_ATTR vLink_AP(uint32 AP_ID)
{
	uint32 APNAME_len = 0,APPSWD_len = 0;
	uint32 Ap_Erase = 0,i,APname[APNAMEMAXLEN];
	uint32 APpasw[64];
	uint8 APname1[APNAMEMAXLEN] ,APpasw1[64];

	switch(AP_ID)
	{
	  case APID1:Ap_Erase = AP_NAME_PSWD_Erase;break;
	  case APID2: Ap_Erase = AP1_NAME_PSWD_Erase;break;
	  default:break;
	}
	if((AP_ID == APID1) || (AP_ID == APID2))
	{
		 Spi_FlashRead(Ap_Erase,AP_NAME_LEN_ERASE_OFFSET,&APNAME_len,1);  //从FLASH中获取AP名称长度和密码长度
		 Spi_FlashRead(Ap_Erase,AP_PSWD_LEN_ERASE_OFFSET,&APPSWD_len,1);
		 DNS_SERVER_DEBUG("\n");
		 if((APNAME_len <= APNAMEMAXLEN) &&(APNAME_len > 0) &&(APPSWD_len <= 64) && (APPSWD_len >= 0))
		 {
			 os_bzero(APname1,APNAMEMAXLEN);
			 os_bzero(APpasw1,64);
			DNS_SERVER_DEBUG("APNAME_len = %d\nAPPSWD_len = %d\n",APNAME_len,APPSWD_len);
			Spi_FlashRead(Ap_Erase,AP_NAME_ERASE_OFFSET,(uint32 *)APname,APNAME_len);  //从FLASH中获取AP名称和密码
			Spi_FlashRead(Ap_Erase,AP_PSWD_ERASE_OFFSET,(uint32 *)APpasw,APPSWD_len);

			 for(i = 0; i < APNAME_len; i ++)
			  {
				APname1[i] = (uint8 )APname[i];
				DNS_SERVER_DEBUG("%c",APname1[i]);
			  }
			   DNS_SERVER_DEBUG("\nAppasw = ");
			 for(i = 0; i < APPSWD_len; i ++)
			  {
				APpasw1[i] = (uint8 )APpasw[i];
				DNS_SERVER_DEBUG("%c",APpasw1[i]);
			  }
			 DNS_SERVER_DEBUG("\n");
			 user_set_station_config(APname1,APpasw1,8,12);  //连接相应的AP
			 //user_set_station_config("OnePlus5","181701408444",8,12);
			 spi_flash_erase_sector (AP_NUM_Erase);
		     spi_flash_write(AP_NUM_Erase*4*1024 + AP_NUM_ERASE_OFFSET,&AP_ID,1 * 4);   //记录客户配置的APid
		 }
		else
		 {

			DNS_SERVER_DEBUG("noAPdata\n");
		    DNS_SERVER_DEBUG("APNAME_len = %d\nAPPSWD_len = %d\n",APNAME_len,APPSWD_len);

		 }
	}

}
void ICACHE_FLASH_ATTR vGetStaticIP(uint8 mode)
{
	struct ip_info info;
	uint32 IP_addr[4] = {0,0,0,0};      //用以存储解析好的IP地址
	uint32 Subnet_mask[4] = {0,0,0,0};  //用以存储解析好的子网掩码
	uint32 Gateway[4] = {0,0,0,0};      //用以存储解析好的网管
    uint32 i,SSIDlen = 0,Pswdlen = 0,SSID[32],Pswd[32];
    char SSID1[32],Pswd1[32];

	Spi_FlashRead(LOCAL_Erase,LOCAL_IP_ERASE_OFFSET,IP_addr,4);  //从flash中读取本地IP 网关 子网掩码
	Spi_FlashRead(LOCAL_Erase,LOCAL_Mask_ERASE_OFFSET,Subnet_mask,4);
	Spi_FlashRead(LOCAL_Erase,LOCAL_GATAWAY_ERASE_OFFSET,Gateway,4);

	Spi_FlashRead(LOCALSSID_Erase,SSID_Len_ERASE_OFFSET,&SSIDlen,1);	//读取本地wifi名字的长度和密码的长度
	Spi_FlashRead(LOCALSSID_Erase,PSWD_LEN_ERASE_OFFSET,&Pswdlen,1);

	 if((SSIDlen <= 32) &&(SSIDlen > 0) &&(Pswdlen <= 32) && (Pswdlen >= 0))
	 {
		 os_bzero(SSID1,32);
		 os_bzero(Pswd1,32);

		DNS_SERVER_DEBUG("SSIDlen = %d\nPswdlen = %d\n",SSIDlen,Pswdlen);
		Spi_FlashRead(LOCALSSID_Erase,SSID_ERASE_OFFSET,SSID,SSIDlen);  //读取本地wifi名称和密码
		if(Pswdlen > 0)
		{
		 Spi_FlashRead(LOCALSSID_Erase,PSWD_ERASE_OFFSET,Pswd,Pswdlen);
		 for(i = 0 ;i < Pswdlen;i ++)
		 {
			 Pswd1[i] = (char )Pswd[i];
			 DNS_SERVER_DEBUG("%c", Pswd1[i]);
		 }
		}
		 for(i = 0 ;i < SSIDlen;i ++)
		 {
			 SSID1[i] = (char )SSID[i];
			 DNS_SERVER_DEBUG("%c", SSID1[i]);
		 }
		 DNS_SERVER_DEBUG("\n");

		 DNS_SERVER_DEBUG("\n");
	 }



	DNS_SERVER_DEBUG("StaticIP = %d.%d.%d.%d\n",IP_addr[0],IP_addr[1],IP_addr[2],IP_addr[3]);
	DNS_SERVER_DEBUG("Sub_Mask = %d.%d.%d.%d\n",Subnet_mask[0],Subnet_mask[1],Subnet_mask[2],Subnet_mask[3]);
	DNS_SERVER_DEBUG("Gate_way = %d.%d.%d.%d\n",Gateway[0],Gateway[1],Gateway[2],Gateway[3]);

	IP4_ADDR(&info.ip, IP_addr[0], IP_addr[1], IP_addr[2], IP_addr[3]);       //设置IP
	IP4_ADDR(&info.gw, Gateway[0], Gateway[1], Gateway[2], Gateway[3]);	      //设置网关
	IP4_ADDR(&info.netmask, Subnet_mask[0], Subnet_mask[1], Subnet_mask[2], Subnet_mask[3]); //设置子网掩码

	ConfigEsp8266(SSID1,Pswd1,SSIDlen,Pswdlen,&info,mode);

}

void ICACHE_FLASH_ATTR RecoveryData()
{
	static struct ip_info info;
	int32 i,Timezone1;
	char yladdr[6];
	AllSetData DefaultData;
	//cn.ntp.org.cn
	//118.24.4.66
	uint32 NtpIP[13] = {'1','1','8','.','2','4','.','4','.','6','6','\0'};
	//uint32 NtpIP[16] = {'c','n','.','n','t','p','.','o','r','g','.','c','n','\0'};
	//uint32 Ssid[15] = {'G','x','L','E','D','S','_','S','N','T','P',0,0,0,'\0'};
	uint32 Ssid[14] = {'G','x','L','E','D','S','_','N','P',0,0,0,0,'\0'};
	uint32 SsidPswd[9] = {'g','x','1','2','3','4','5','6','\0'};

	SetDevideCount = 0; /*更改设备的次数存入 */
	spi_flash_erase_sector (FIRST_START_Erase);  //往FLASH里存入时区
	spi_flash_write (FIRST_START_Erase*4*1024 + FIRST_START_ERASE_OFFSET, &SetDevideCount, 1 * 4);

	wifi_get_macaddr(STATION_IF,yladdr);//查询MAC地址

	Ssid[9] = yladdr[4] / 100 + 0x30;
	Ssid[10] = yladdr[4] / 10 % 10 + 0x30;
	Ssid[11] = yladdr[4] % 10 + 0x30;
	Ssid[12] = yladdr[5] % 10 + 0x30;
	DefaultData.TCP_Server_Port = 50000;  /* 端口号默认50000*/
	DefaultData.Timezone = 800;
	DefaultData.Auto_Web_State = AUTO_ACCESS_WEB;  /* 默认自动弹出网页 */
	DefaultData.IsSetDST = DST_NO_SET;
	DefaultData.TcpServerIP_Len = 0;
	DefaultData.Language_state = LANGUAGE_EN;
	DefaultData.IP[0] = 192;  DefaultData.IP[1] =168;DefaultData.IP[2] =1;DefaultData.IP[3] =100;

	DefaultData.gw[0] = 192; DefaultData.gw[1] =168;DefaultData.gw[2] =1;DefaultData.gw[3] =100;

	DefaultData.netmask[0] = 255;DefaultData.netmask[1] = 255;DefaultData.netmask[2] = 255;DefaultData.netmask[3] = 0;

	DefaultData.NtpAppMode = NTP_MODE;

	DefaultData.SsidLen = 13;
	DefaultData.SsidPswdLen = 0;

	DefaultData.NtpseverIPlen = 11;
	DefaultData.AP_num = NO_APID;
	DefaultData.TimePutInterva = 10;
	DefaultData.DHCP_State = DHCP_OPEN;
	spi_flash_erase_sector (NTP_IP_Erase);  //往FLASH里存入NTP IP 信息
	spi_flash_write (NTP_IP_Erase*4*1024 + NTP_IP_ERASE_OFFSET, NtpIP, DefaultData.NtpseverIPlen * 4);
	spi_flash_write (NTP_IP_Erase*4*1024 + NTP_IP_LEN_ERASE_OFFSET, &DefaultData.NtpseverIPlen, 1 * 4);

	spi_flash_erase_sector (DST_Erase);  //往FLASH里存入夏令时 信息
	spi_flash_write (DST_Erase*4*1024 + ISSET_DST_ERASE_OFFSET, &DefaultData.IsSetDST, 1 * 4);

	spi_flash_erase_sector (TIMEZONE_ERASE);  //往FLASH里存入时区
	spi_flash_write (TIMEZONE_ERASE*4*1024 + TIMEZONE_ERASE_OFFSET, &DefaultData.Timezone, 1 * 4);

	spi_flash_erase_sector (LOCAL_Erase);  //往FLASH里存入本地IP 网关 子网掩码
	spi_flash_write (LOCAL_Erase*4*1024 + LOCAL_IP_ERASE_OFFSET, DefaultData.IP, 4 * 4);
    spi_flash_write (LOCAL_Erase*4*1024 + LOCAL_Mask_ERASE_OFFSET, DefaultData.netmask, 4 * 4);
    spi_flash_write (LOCAL_Erase*4*1024 + LOCAL_GATAWAY_ERASE_OFFSET, DefaultData.gw, 4 * 4);
                                           //往Flash里存入本地的wifi名称和密码以及相应的长度
    spi_flash_erase_sector (LOCALSSID_Erase);
    spi_flash_write (LOCALSSID_Erase*4*1024 + SSID_ERASE_OFFSET, Ssid, DefaultData.SsidLen * 4);
    //spi_flash_write (LOCALSSID_Erase*4*1024 + PSWD_ERASE_OFFSET, SsidPswd, DefaultData.SsidPswdLen * 4);
    spi_flash_write (LOCALSSID_Erase*4*1024 + SSID_Len_ERASE_OFFSET, &DefaultData.SsidLen, 1 * 4);
    spi_flash_write (LOCALSSID_Erase*4*1024 + PSWD_LEN_ERASE_OFFSET, &DefaultData.SsidPswdLen, 1 * 4);


	spi_flash_erase_sector (NO_DHCP_Erase);  //往FLASH里存入是否开启DHCP 默认开启DHCP
	spi_flash_write (NO_DHCP_Erase*4*1024 + DHCP_OPEN_ERASE_OFFSET, &DefaultData.DHCP_State, 1 * 4);


    spi_flash_erase_sector (AP_NAME_PSWD_Erase);    //清除AP信息
    spi_flash_erase_sector (AP1_NAME_PSWD_Erase);

    spi_flash_erase_sector (AP_NUM_Erase);
    spi_flash_write(AP_NUM_Erase*4*1024 + AP_NUM_ERASE_OFFSET,&DefaultData.AP_num,1 * 4);   //记录客户配置的AP个数

	spi_flash_erase_sector (TIME_Interva_ERASE);  //
	spi_flash_write (TIME_Interva_ERASE*4*1024 + TIME_Interva_ERASE_OFFSET, &DefaultData.TimePutInterva, 1 * 4);

	spi_flash_erase_sector (IS_AUTO_WEB_Erase);  //
	spi_flash_write (IS_AUTO_WEB_Erase*4*1024 + IS_AUTO_WEB_ERASE_OFFSET, &DefaultData.Auto_Web_State, 1 * 4); /* 设置自动弹出网页 */

	spi_flash_erase_sector (TCP_PORT_Erase);  // /* 默认存储50000端口号*/
	spi_flash_write (TCP_PORT_Erase*4*1024 + TCP_PORT_ERASE_OFFSET, &DefaultData.TCP_Server_Port, 1 * 4);


	spi_flash_erase_sector (TCP_SERVERIP_Erase);/* 擦出数据,初始化无TCPClient连接 */
	spi_flash_write (TCP_SERVERIP_Erase*4*1024 + TCP_SERVERIPLen_ERASE_OFFSET, &(DefaultData.TcpServerIP_Len), 1 * 4);

	spi_flash_erase_sector (LANGUAGE_Erase);/* 擦出数据,初始化网页为英文版本  */
	spi_flash_write (LANGUAGE_Erase*4*1024 + LANGUAGE_ERASE_OFFSET, &(DefaultData.Language_state), 1 * 4);

}

void ICACHE_FLASH_ATTR GetTcpClientInif(void)
{
	int32 TcpServerIP_Len = 0;
	Spi_FlashRead(TCP_SERVERIP_Erase,TCP_SERVERIPLen_ERASE_OFFSET, &TcpServerIP_Len,1);
	if(TcpServerIP_Len < 4)
	{
		return;/* 未配置过TCP Client */
	}
	else if((TcpServerIP_Len >= 4) && (TcpServerIP_Len < 32))
	{
		int32 TcpPort = 0,TcpType = 0;
		uint32 TcpIpBuf[32];
		uint8 i = 0;
		Spi_FlashRead(TCP_SERVERIP_Erase,TCP_SERVER_PORT_ERASE_OFFSET, &TcpPort,1);/* 获取端口号 */
		Spi_FlashRead(TCP_SERVERIP_Erase,TCP_SERVER_TPYE_ERASE_OFFSET, &TcpType,1);/* 获取长连接还是短连接 */
		Spi_FlashRead(TCP_SERVERIP_Erase,TCP_SERVERIP_ERASE_OFFSET, TcpIpBuf,TcpServerIP_Len);   /* 获取TCP服务器IP或域名  */

		if(TcpServerIP_Len == 4)
		{
			for(i = 0; i < TcpServerIP_Len; i ++)
			{
				TcpClientToServer[i] = (char)TcpIpBuf[i];
			}
			if(TcpType != 'L') /*短连接 则不做处理，在串口中断中处理 */
	     	{

		    }
		    else     /*  长连接则连接TCP服务器 */
		    {
			  os_timer_disarm(&TcpClientLink); /* 开启定时器检测是否获取到动态IP地址 */
			  os_timer_setfn(&TcpClientLink, (os_timer_func_t *)IsTcpClientLink, NULL);
			  os_timer_arm(&TcpClientLink, 100, 0);
		     }
		}
		else
		{
			for(i = 0; i < TcpServerIP_Len; i ++)
			{
				DnsBuffer[i] = (char)TcpIpBuf[i];
			}
			 os_timer_disarm(&TcpClientLink); /* 开启定时器检测是否获取到动态IP地址 */
			 os_timer_setfn(&TcpClientLink, (os_timer_func_t *)IsTcpClientLink, NULL);
			 os_timer_arm(&TcpClientLink, 100, 0);
		}

	}

}
