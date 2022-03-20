/*
 * mySmartlink.c
 *
 *  Created on: 2017年2月8日
 *      Author: dell
 */

#include "mysmartlink.h"
#include "led.h"
#include "sntp.h"
#include "mem.h"
#include "spi_flash.h"
#include "soft_timer.h"
#include "tcpserver.h"
#include "udpclient.h"
#include "init.h"
uint32 IsDst;//记录是否需要设置夏令时
uint8 AlreadStartPutTime = 0;/* 记录是否已经成功从NTP服务器获取时间 */
Dst_Packet Ntp_Data1;
uint8_t PackSend[18] = {0x1f,0x30,0x6d,0,0,0,0,0,0,0,0,0,0,0,0,0,0x09,0};
void Spi_FlashRead(uint32 Erase_Point,uint32 Erase_Offset,uint32 * data_point,uint32 data_len);
void SetDst(DstPacket * Dststart,DstPacket*Dstend,Dst_Packet*Ntp_data);
void DealwithNtpStr(char *NtpData,Dst_Packet *DstData);
LOCAL os_timer_t my_sntp_timer;//smartconfig_timer; //定义一个定时器结构，智能连接过程中，控制LED闪烁，起到指示的作用
extern uint32_t TimeOutInterva;
extern  uint8_t isLedClose;
void ICACHE_FLASH_ATTR DealwithNtpStr(char *NtpData,Dst_Packet *DstData)
{
	uint8 i;
	if (strstr( NtpData, "Jan") != NULL)
	{
		DstData->Dst_month = 1;
	}
	else if (strstr( NtpData, "Feb") != NULL)
	{
		DstData->Dst_month = 2;
	}
	else if (strstr( NtpData, "Mar") != NULL)
	{
		DstData->Dst_month = 3;
	}
	else if (strstr( NtpData, "Apr") != NULL)
	{
		DstData->Dst_month = 4;
	}
	else if (strstr( NtpData, "May") != NULL)
	{
		DstData->Dst_month = 5;
	}
	else if (strstr( NtpData, "Jun") != NULL)
	{
		DstData->Dst_month = 6;
	}
	else if (strstr( NtpData, "Jul") != NULL)
	{
		DstData->Dst_month = 7;
	}
	else if (strstr( NtpData, "Aug") != NULL)
	{
		DstData->Dst_month = 8;
	}
	else if (strstr( NtpData, "Sep") != NULL)
	{
		DstData->Dst_month = 9;
	}
	else if (strstr( NtpData, "Oct") != NULL)
	{
		DstData->Dst_month = 10;
	}
	else if (strstr( NtpData, "Nov") != NULL)
	{
		DstData->Dst_month = 11;
	}
	else if (strstr( NtpData, "Dec") != NULL)
	{
		DstData->Dst_month = 12;
	}

	DstData -> Dst_day = 0;
	DstData -> Dst_hour = 0;
	DstData -> Dst_min = 0;
	DstData -> Dst_sec = 0;
	DstData -> Dst_year = 0;
   for(i = 0 ; i < 2 ; i ++)
   {
	   DstData -> Dst_sec = DstData -> Dst_sec * 10;
	   DstData -> Dst_sec =  (uint32)((uint32)NtpData[i + 17] - 0x30 + DstData -> Dst_sec);
	   DstData -> Dst_min = DstData -> Dst_min * 10;
	   DstData -> Dst_min = (uint32)((uint32)NtpData[i + 14] - 0x30 + DstData -> Dst_min);
	   DstData -> Dst_hour = DstData -> Dst_hour * 10;
	   DstData -> Dst_hour = (uint32)((uint32)NtpData[i + 11] - 0x30 + DstData -> Dst_hour);
	   DstData -> Dst_day =  DstData->Dst_day * 10;
	   DstData -> Dst_day = (uint32)((uint32)NtpData[i + 8] - 0x30 + DstData->Dst_day);
   }
   for(i = 0; i < 4; i ++)
   {
	  DstData -> Dst_year = DstData -> Dst_year * 10;
	  DstData -> Dst_year =  (uint32)((uint32)NtpData[i + 20] - 0x30 + DstData -> Dst_year);
   }
   DstData -> Dst_week = Get_Week(DstData -> Dst_year,DstData -> Dst_month,DstData -> Dst_day); /* 蔡乐公式获取星期 */
   //DNS_SERVER_DEBUG("DstData = %d-%d-%d-%d-%d-%d-%d\n",DstData -> Dst_year,DstData ->Dst_month,DstData -> Dst_day, DstData -> Dst_hour, DstData -> Dst_min, DstData -> Dst_sec,DstData->Dst_week);
}
/*返回这个月的第几周*/
uint32 ICACHE_FLASH_ATTR ReturnMonSelWeek(Dst_Packet*Ntp_data2)
{
	uint32  week,year,century,month,day = 1;
	uint32 period = 1;

	/*判断该月第一天是星期几*/
	century = (*Ntp_data2).Dst_year/100;
	year = (*Ntp_data2).Dst_year%100;
	month = (*Ntp_data2).Dst_month;
	if(month < 3)
	{
		month += 12;
		year --;
	}
	week = year + year/4 + century/4 - 2*century + 26*(month+1)/10 ;
	while(week < 0)
	{
		week += 7;
	}
	week = week % 7;
	if((*Ntp_data2).Dst_week < week)
	{
		period -= 1;
	}
//	printf("%d\n",week);
	/*判断改日是本月第几周*/
	while(day < (*Ntp_data2).Dst_day)
	{
		day ++;
		week ++;
		if(week == 7)	//每到星期日周数加一
		{
			period++;
			week = 0;
		}
	}

	return period;
}
/*已知Ntp_data.Dst_year,Ntp_data.Dst_month,Dst_Inf.Dst_WeekSel,Dst_Inf.Dst_Week*/
/*年，月，第几个星期，星期已知*/
/*返回 相应日期 */
uint32 ReturnMonSelDate(Dst_Packet *Ntp_data,DstPacket *Dst_Inf)
{
	int  week,year,century,month,day = 1;
	unsigned int period = 1;


	/*判断该月第一天是星期几*/
	century = (*Ntp_data).Dst_year/100;
	year = (*Ntp_data).Dst_year%100;
	month = (*Ntp_data).Dst_month;
	if(month<3)
	{
		month+=12;
		year--;
	}
	week = year + year/4 + century/4 - 2*century + 26*(month+1)/10 ;
	while(week<0)
	{
		week += 7;
	}
	week = week%7;

	while((period != (*Dst_Inf).Dst_WeekSel) || (week != (*Dst_Inf).Dst_Week))
	{
		if(week == (*Dst_Inf).Dst_Week)
		{
			period++;

			if(((*Ntp_data).Dst_year%4 == 0 && (*Ntp_data).Dst_year%100 != 0) || (*Ntp_data).Dst_year%400 == 0)
			{
				if((*Ntp_data).Dst_month == 2)
				{
					if((day+7)>29)
					{
						return day;
					}
				}
				else if((*Ntp_data).Dst_month == 1 || (*Ntp_data).Dst_month == 3 || (*Ntp_data).Dst_month == 5 || (*Ntp_data).Dst_month == 7 || (*Ntp_data).Dst_month == 8 || (*Ntp_data).Dst_month == 10 || (*Ntp_data).Dst_month == 12)
				{
					if((day+7)>31)
					{
						return day;
					}
				}
				else
				{
					if((day+7)>30)
					{
						return day;
					}
				}
			}
			else
			{
				if((*Ntp_data).Dst_month == 2)
				{
					if((day+7)>28)
					{
						return day;
					}
				}
				else if((*Ntp_data).Dst_month == 1 || (*Ntp_data).Dst_month == 3 || (*Ntp_data).Dst_month == 5 || (*Ntp_data).Dst_month == 7 || (*Ntp_data).Dst_month == 8 || (*Ntp_data).Dst_month == 10 || (*Ntp_data).Dst_month == 12)
				{
					if((day+7)>31)
					{
						return day;
					}
				}
				else
				{
					if((day+7)>30)
					{
						return day;
					}
				}
			}
		}
		day++;
		week++;
		if(week == 7)
		{
			week = 0;
		}
	}
	return day;
}
int ICACHE_FLASH_ATTR GetMonthDays(int year,int month)
{
    switch(month)
    {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
            return 31;
            break;
        case 4:
        case 6:
        case 9:
        case 11:
            return 30;
            break;
        case 2:
            if((year%4==0 && year%100!=0 ) || (year%400==0))
            {
                return 29;
            }
            else
            {
                return 28;
            }
            break;
        default:
            DNS_SERVER_DEBUG("month error!");
            return 0;
            break;
    }
}
void ICACHE_FLASH_ATTR SetDst(DstPacket * Dststart,DstPacket*Dstend,Dst_Packet*Ntp_data)
{
	int32 DstIsstart = 0;
    uint32 DstStartWelSelData = 0,DstEndWelSelData = 0;


	//到了夏令时开始时间
	if((Ntp_data->Dst_month) > (Dststart -> Dst_Mon) &&  (Ntp_data->Dst_month) < (Dstend -> Dst_Mon))
	{
		DstIsstart = DST_STRAT;
		if(Ntp_data->Dst_month + 1 == Dstend -> Dst_Mon)/* 考虑极限情况 在一号的零点结束夏令时 */
	   {
			if(Dstend -> Dst_Hour == 0)
			{
				DstEndWelSelData = ReturnMonSelDate(Ntp_data,Dstend);
				if(DstEndWelSelData == 1)
				{
					if(Ntp_data ->Dst_day == GetMonthDays(Ntp_data ->Dst_year,Ntp_data ->Dst_month) )
					{
						if(Ntp_data -> Dst_hour + 1 == 24)
						 {
						  DstIsstart = DST_FINISH;
						 }
					}
				}
			  }
		}
	}
	else if(Ntp_data->Dst_month == Dststart -> Dst_Mon)
	{
	    DstStartWelSelData = ReturnMonSelDate(Ntp_data,Dststart);

		if(Ntp_data ->Dst_day > DstStartWelSelData)
		{
			DstIsstart = DST_STRAT;
		}
		else if(Ntp_data ->Dst_day == DstStartWelSelData)
		{
			if(Ntp_data -> Dst_hour >= Dststart -> Dst_Hour)
			{
				DstIsstart = DST_STRAT;
			}
			else
			{
				DstIsstart = DST_FINISH;
			}
		}
		else
		{
			DstIsstart = DST_FINISH;
		}

	}
	else if ((Ntp_data->Dst_month) == (Dstend -> Dst_Mon))
	{
	    DstEndWelSelData = ReturnMonSelDate(Ntp_data,Dstend);

		if(Ntp_data ->Dst_day < DstEndWelSelData)  /* 考虑极限情况 在非一号的零点结束夏令时 */
		{
			DstIsstart = DST_STRAT;
			if(Ntp_data ->Dst_day + 1 == DstEndWelSelData)
			{
			  if(Dstend -> Dst_Hour == 0)
			  {
				  if(Ntp_data -> Dst_hour + 1 == 24)
				  {
					  DstIsstart = DST_FINISH;
				  }
			  }

			}
		}
		else if(Ntp_data ->Dst_day == DstEndWelSelData)
		{
			if((Ntp_data -> Dst_hour + 1) < Dstend -> Dst_Hour)
			{
				DstIsstart = DST_STRAT;
			}
			else
			{
				DstIsstart = DST_FINISH;
			}
		}
		else
		{
			DstIsstart = DST_FINISH;
		}
	}
	else
	{
		DstIsstart = DST_FINISH;
	}

	if(DstIsstart == DST_STRAT)  /* 软件更新夏令时时间  */
	{
		Ntp_data ->Dst_hour ++;
		if(Ntp_data ->Dst_hour == 24)
		{
			Ntp_data ->Dst_hour = 0;
			Ntp_data ->Dst_day ++;
			if((Ntp_data->Dst_month == 1 )||(Ntp_data->Dst_month == 3)||(Ntp_data->Dst_month == 5)||(Ntp_data->Dst_month == 7)||(Ntp_data->Dst_month == 8)||(Ntp_data->Dst_month == 10)||(Ntp_data->Dst_month == 12))
			{
				if(Ntp_data ->Dst_day == 32)
				{
					Ntp_data ->Dst_day = 0;
					Ntp_data->Dst_month ++;
					if(Ntp_data->Dst_month == 13)
					{
						Ntp_data->Dst_month = 1;
						//Ntp_data->Dst_year ++;
					}
				}
			}
			else
			{
				if(Ntp_data ->Dst_month == 2)
				{     /* 闰年 */
					if((Ntp_data->Dst_year%400 == 0)||(Ntp_data->Dst_year%4 == 0 && Ntp_data->Dst_year%100 != 0))//闰年
					{
						if( Ntp_data ->Dst_day == 30)
						{
							Ntp_data ->Dst_day = 0;
							Ntp_data->Dst_month ++;
						 if(Ntp_data->Dst_month == 13)
						 {
							 Ntp_data->Dst_month = 1;
							// Ntp_data->Dst_year ++;
						 }
						}
					}
					else
					{
						if( Ntp_data ->Dst_day == 29)
						{
							Ntp_data ->Dst_day = 0;
							Ntp_data->Dst_month ++;
						 if(Ntp_data->Dst_month == 13)
						 {
							 Ntp_data->Dst_month = 1;
							// Ntp_data->Dst_year ++;
						 }
						}
					}
				}
				else
				{
					if( Ntp_data ->Dst_day == 31)
					{
						Ntp_data ->Dst_day = 0;
						Ntp_data->Dst_month ++;
					 if(Ntp_data->Dst_month == 13)
					 {
						 Ntp_data->Dst_month = 1;
						// Ntp_data->Dst_year ++;
					 }
					}
				}
			}
		}

	}

}
/****************************************************************************
* 名    称：u8 Get_Week(u16 Year,u16 Month,u16 Date)
* 功    能：用蔡勒（Zeller）公式计算星期几

	w=y+[y/4]+[c/4]-2c+[26（m+1）/10]+d-1

	然后w对7取余。
	公式中的符号含义如下，w：星期；c：世纪；y：年（两位数）；m：月（m大于等于3，小
	于等于14，即在蔡勒公式中，某年的1、2月要看作上一年的13、14月来计算，比如2003年1月1
	日要看作2002年的13月1日来计算）；d：日；[ ]代表取整，即只要整数部分。

* 入口参数：Year:年; Month:月; Date:日。
* 出口参数：星期几 1~6代表星期一~星期六，0代表星期天
* 说    明：
* 调用方法：无
****************************************************************************/
unsigned char ICACHE_FLASH_ATTR Get_Week(unsigned int Year,unsigned char Month,unsigned char Date)
{
	int  W,Y,C,M,D;
	unsigned char day;

	C = Year/100;
	Y = Year%100;
	M = Month;
	if(M<3){M+=12;Y--;}
	D = Date;
	W = Y + Y/4 + C/4 - 2*C + 26*(M+1)/10 + D - 1;
	while(W<0)	W += 7;
	day = W%7;
	return day;
}
extern int32 Ntp_Timezone;
extern uint32 SysTime_ms;
#define SEC_TIME_ZONE         (Ntp_Timezone*6*6)  //Beijing,GMT+8
void ICACHE_FLASH_ATTR user_check_sntp_stamp(void *arg)
{
	 uint32 i,current_stamp;
	 char * RealTime = NULL;
	 char NTP_time[32];
     char *string ;
	 uint32 hourstart = 0,hourend = 0;

	 current_stamp =  get_current_stamp();
	 RealTime = sntp_get_real_time(current_stamp + SEC_TIME_ZONE);

     for(i = 0; i < 32; i ++)
     {
    	 if(RealTime[i] == '\0')
    	 {
    		 NTP_time[i] = '\0';
    		 break;
    	 }
    	 NTP_time[i] = RealTime[i];
     }

    DealwithNtpStr(NTP_time,&Ntp_Data1); /* 对字符串时间数据做处理，存储到结构体中 */
	if(IsDst == DST_SET)
	{            //取出要设置的夏令时数据
		Spi_FlashRead(DST_Erase,DSTSTART_HOUR_ERASE_OFFSET,&DstStart.Dst_Hour,1);
		Spi_FlashRead(DST_Erase,DSTSTART_SELE_WEEK_ERASE_OFFSET,&DstStart.Dst_WeekSel,1);
		Spi_FlashRead(DST_Erase,DSTSTART_WEEK_ERASE_OFFSET,&DstStart.Dst_Week,1);
		Spi_FlashRead(DST_Erase,DSTSTART_MON_ERASE_OFFSET,&DstStart.Dst_Mon,1);

		Spi_FlashRead(DST_Erase,DSTEND_HOUR_ERASE_OFFSET,&DstEnd.Dst_Hour,1);
		Spi_FlashRead(DST_Erase,DSTEND_SELE_WEEK_ERASE_OFFSET,&DstEnd.Dst_WeekSel,1);
		Spi_FlashRead(DST_Erase,DSTEND_WEEK_ERASE_OFFSET,&DstEnd.Dst_Week,1);
		Spi_FlashRead(DST_Erase,DSTEND_MON_ERASE_OFFSET,&DstEnd.Dst_Mon,1);

		SetDst(&DstStart,&DstEnd,&Ntp_Data1);

	}

	PackSend[3] = (uint8_t)((Ntp_Data1.Dst_year / 10 % 10) + 0x30);
	PackSend[4] = (uint8_t)((Ntp_Data1.Dst_year / 1 % 10) + 0x30);
	PackSend[5] = (uint8_t)(Ntp_Data1.Dst_month / 10 + 0x30);
	PackSend[6] = (uint8_t)(Ntp_Data1.Dst_month % 10 + 0x30);
	PackSend[7] = (uint8_t)(Ntp_Data1.Dst_day / 10 + 0x30);
	PackSend[8] = (uint8_t)(Ntp_Data1.Dst_day % 10 + 0x30);
	PackSend[9] = (uint8_t)(Ntp_Data1.Dst_hour / 10 + 0x30);
	PackSend[10] = (uint8_t)(Ntp_Data1.Dst_hour % 10 + 0x30);
	PackSend[11] = (uint8_t)(Ntp_Data1.Dst_min / 10 + 0x30);
	PackSend[12] = (uint8_t)(Ntp_Data1.Dst_min % 10 + 0x30);
	PackSend[13] = (uint8_t)(Ntp_Data1.Dst_sec / 10 + 0x30);
	PackSend[14] = (uint8_t)(Ntp_Data1.Dst_sec % 10 + 0x30);
	PackSend[15] = (uint8_t)(Ntp_Data1. Dst_week + 0x30);

	AlreadStartPutTime = 1;

	DNS_SERVER_DEBUG("sntp: %d, %s \n",current_stamp,	NTP_time);


}
extern uint8_t LedChoose;
extern uint32 SysTime_ms;
void ICACHE_FLASH_ATTR PutSntpTime(void)
{
	uint8_t i = 0;
	PackSend[17] = PackSend[1] + PackSend[2] + PackSend[3] + PackSend[4] + PackSend[5] + \
			      PackSend[6] + PackSend[7] + PackSend[8] + PackSend[9] + PackSend[10] + PackSend[11] + PackSend[12] + PackSend[13] + PackSend[14] + PackSend[15];

	uart0_tx_buffer(PackSend,18);
	LedBlue(0);
	LedGreen(0);
	LedRed(0);
	SysTime_ms = 300;
}
void ICACHE_FLASH_ATTR
user_esp_platform_dns_found_ntp(const char *name, ip_addr_t *ipaddr, void *arg)
{
struct espconn *pespconn = (struct espconn *)arg;
if (ipaddr != NULL)
{
os_printf("user_esp_platform_dns_found %d.%d.%d.%d/n",
		 *((uint8 *)&ipaddr->addr),
		 *((uint8 *)&ipaddr->addr + 1),
		 *((uint8 *)&ipaddr->addr + 2),
		 *((uint8 *)&ipaddr->addr + 3));

		set_sntp_server_ip(ipaddr);
}
else
{
	 os_printf("Found DNS Failed!\n");
}

}
struct espconn Dns_esp_conn_ntp;     //建立一个espconn结构体
ip_addr_t esp_server_ip_ntp;
void ICACHE_FLASH_ATTR Sntp_Config(char *Ntpsever,uint32 NtpipLen)
{
	uint8_t i,isDns = 0;
	isDns = 0;
	ip_addr_t *addr = (ip_addr_t *)os_zalloc(sizeof(ip_addr_t));
	Ntpsever[NtpipLen] = '\0';
	for(i = 0; i < NtpipLen;i ++)
	{
		if(((Ntpsever[i] - 0x30) >= 0) && ((Ntpsever[i] - 0x30) <= 9 ) )
		{
		}
		else if(Ntpsever[i] == '.')
		{

		}
		else
		{
			isDns = 1;
			break;
		}
	}
	DNS_SERVER_DEBUG("ntpserver = %s", Ntpsever);

	if(isDns == 1)
	{
     DNS_SERVER_DEBUG("DNSSet\n");
	 espconn_gethostbyname(&Dns_esp_conn_ntp,Ntpsever,&esp_server_ip_ntp,user_esp_platform_dns_found_ntp);
	}
	else
	{		
		ipaddr_aton(Ntpsever, addr);
		set_sntp_server_ip(addr);
		DNS_SERVER_DEBUG("NTPIP != DSN\n");

		//sntp_setserver(0, addr); // set server 2 by IP address
	}

    os_free(addr);
    Spi_FlashRead(DST_Erase,ISSET_DST_ERASE_OFFSET,&IsDst,1);  //读取是否需要设置夏令时

}
///*-------------------------------------------------------------*/
///*函数功能：smartlink智能初始化                                                                  */
///*参       数：status ：smartlink状态                                                               */
///*参       数：pdata ：不同的状态代表的数据不同                                                */
///*返       回：无                                                                                                     */
///*-------------------------------------------------------------*/
//void ICACHE_FLASH_ATTR smartconfig_init(void)
//{
//	smartconfig_set_type(SC_TYPE_ESPTOUCH_AIRKISS);     //设置smartlink模式：ESPTOUCH
//	smartconfig_start(smartconfig_done,0);      //开始smartlink,并设置smartlink回调函数，1：表示串口0打印smartlink的连接过程信息
//}
///*-------------------------------------------------------------*/
///*函数功能：smartlink智能连接函数                                                                  */
///*参       数：status ：smartlink状态                                                               */
///*参       数：pdata ：不同的状态代表的数据不同                                                */
///*返       回：无                                                                                                     */
///*-------------------------------------------------------------*/
//void ICACHE_FLASH_ATTR smartconfig_done(sc_status status, void *pdata)
//{
//	uint8_t i;
//	switch(status)
//	{
//			case SC_STATUS_WAIT:				os_printf("SC_STATUS_WAIT\n");
//												break;
//
//			case SC_STATUS_FIND_CHANNEL:		os_printf("SC_STATUS_FIND_CHANNEL\n");
//												//100ms的一个定时器(自动模式)，智能连接过程在，控制led闪烁，起到指示作用
//												os_timer_disarm(&smartconfig_timer);
//												os_timer_setfn(&smartconfig_timer, (os_timer_func_t *)smartconfig_led, NULL);
//												os_timer_arm(&smartconfig_timer, 100, 1);
//												break;
//
//			case SC_STATUS_GETTING_SSID_PSWD:	os_printf("SC_STATUS_GETTING_SSID_PSWD\n");
//												sc_type *type = pdata;
//												if (*type == SC_TYPE_ESPTOUCH)
//												{
//													os_printf("SC_TYPE:SC_TYPE_ESPTOUCH\n");
//												}
//												else
//												{
//													os_printf("SC_TYPE:SC_TYPE_AIRKISS\n");
//												}
//												break;
//
//			case SC_STATUS_LINK:      		    os_printf("SC_STATUS_LINK\n");
//												struct station_config *sta_conf = pdata;
//												wifi_station_set_config(sta_conf);
//												wifi_station_disconnect();
//												wifi_station_connect();
//
//												break;
//
//			case SC_STATUS_LINK_OVER:   		os_printf("SC_STATUS_LINK_OVER\n");
//												if (pdata != NULL)
//												{
//													uint8 phone_ip[4] = {0};
//													memcpy(phone_ip, (uint8*)pdata, 4);
//													os_printf("Phone ip: %d.%d.%d.%d\n",phone_ip[0],phone_ip[1],phone_ip[2],phone_ip[3]);
//												}
//												os_printf("已记录信息\r\n");
//												smartconfig_stop();
//												os_timer_disarm(&smartconfig_timer);
//
//												break;
//	}
//}
void Spi_FlashWrite(uint32 Erase_Point,uint32 Erase_Offset,uint32 * data_point,uint32 data_len)
{
    spi_flash_erase_sector (Erase_Point);
    spi_flash_write (Erase_Point*4*1024 + Erase_Offset, (uint32 *)data_point, data_len * 4);
}
void Spi_FlashRead(uint32 Erase_Point,uint32 Erase_Offset,uint32 * data_point,uint32 data_len)
{
	spi_flash_read(Erase_Point * 4096 + Erase_Offset, (uint32 *) data_point, data_len * 4);
}
/*-------------------------------------------------------------*/
/*函数功能：控制led闪烁，起到指示作用                                                              */
/*参       数：无                                                                                                     */
/*返       回：无                                                                                                     */
/*-------------------------------------------------------------*/
//void ICACHE_FLASH_ATTR smartconfig_led(void)
//{
//	if(GPIO_REG_READ(GPIO_OUT_ADDRESS)&0x4000) //读取GPIO14的状态,如果if成立,表示GPIO14是高电平，LED3是熄灭的状态
//		rgbSensorTest(0);                               //点亮
//	else                                       //反之,表示GPIO14是低电平，LED3是点亮的状态
//		rgbSensorTest(1);                              //熄灭
//}
