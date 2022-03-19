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

#include "init.h"
#include "mySmartlink.h"
#include "sntp.h"
#include "tcpserver.h"
#include "spi_flash.h"
#include "string.h"
#include "uart0.h"
#include "uart.h"
#include "soft_timer.h"
#include "Rgbled.h"
uint32 If_Init_Set;
uint32 SetDevideCount; /*��¼�����豸�Ĵ���  */
uint8_t LedChoose = 0;
uint8_t isLedClose = 0;
uint32 SysTime_ms;
uint8 LastChannel;
UartControlblock Uart0CB=     //����0�Ŀ��Ƶ�Ԫ,��ʼ����
{
    BIT_RATE_9600,   //115200������
	EIGHT_BITS,        //8����λ
	STICK_PARITY_DIS,  //��������ʹ��
	NONE_BITS,         //��У��
	ONE_STOP_BIT,      //1��ֹͣλ
};


void user_check_ip(void);     //������������ʱ���ص�����
void Check_Smart_Link(void);
void Init_Set_up(void);
void ICACHE_FLASH_ATTR vLink_AP(uint32 AP_ID);
int32 Ntp_Timezone;
/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}

void ICACHE_FLASH_ATTR
user_rf_pre_init(void)
{
}

extern UartDevice    UartDev;
void to_scan(void)
{
	RgbInit();
    //wifi_softap_dhcps_stop();                  //��ֹͣAPģʽ��DHCP
	UartDev.baut_rate = 9600;
	uart_config(UART1);
	uart0_init(&Uart0CB);  //����0��ʼ����ע�⴮��0���շ�FIFO��128�ֽڣ��������ݵ�ʱ�򣬲�Ҫ���
	UART_SetPrintPort( UART1 );  /* ESP8266�ڲ���Ϣ��ӡ������Ϊ����1*/
    Spi_FlashRead(Init_Erase, 0,&If_Init_Set, 1);  //�Ƿ���Ҫ�ָ�����������
    if(If_Init_Set != INIT_NO_SET)  //�ָ�����������
    {
      RecoveryData();
      //os_printf("huifuchuchangshezhi\n");
     }

      ConfigEsp82663(NULL);
	  vGetStaticIP(STATIONAP_MODE);
	  GetTcpClientInif();    /*   ����TcpClient��ز���  */
	  os_timer_disarm(&Init_timer);
	  os_timer_setfn(&Init_timer, (os_timer_func_t *)Init_Set_up, NULL);/* �˶�ʱ��������ʱɨ����Χ��AP��Ϣ */
	  os_timer_arm(&Init_timer, 100, 1);
	//wifi_station_dhcpc_start();
	  os_timer_disarm(&Check_SmartConfig);
	  os_timer_setfn(&Check_SmartConfig, (os_timer_func_t *)Check_Smart_Link, NULL);/* �˶�ʱ��������ʱ�������AP���������RGB*/
	  os_timer_arm(&Check_SmartConfig, 100, 1);
}


/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR user_init(void)
{
	wifi_set_opmode(STATIONAP_MODE);          //����wifiģʽ��APģʽ
    system_init_done_cb(to_scan);

}


