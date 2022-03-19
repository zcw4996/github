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

#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__
#include "mem.h"

#define Init_Erase  124  //���������ݼ�¼�Ƿ�Ҫ��λ����������

#define AP_NAME_PSWD_Erase   125  //���������ݼ�¼�ͻ����õ�AP���ƺ�����
#define AP_NAME_ERASE_OFFSET   0 //������ƫ�Ƶ�ַ�����洢AP����
#define AP_NAME_LEN_ERASE_OFFSET   240  //������ƫ�Ƶ�ַ�����洢AP���Ƴ���
#define AP_PSWD_ERASE_OFFSET   148  //������ƫ�Ƶ�ַ�����洢AP����
#define AP_PSWD_LEN_ERASE_OFFSET   280  //������ƫ�Ƶ�ַ�����洢AP���볤��

#define AP1_NAME_PSWD_Erase   131  //���������ݼ�¼�ͻ����õĵڶ���AP���ƺ�����

#define NO_APID 0   //��ʾ��δ����AP����
#define APID1   1
#define APID2   2

#define AP_NUM_Erase   132  //���������ݼ�¼�ͻ����õ�AP����
#define AP_NUM_ERASE_OFFSET   0 //������ƫ�Ƶ�ַ�����洢AP����

#define AP_NUM_MAX       2     //����¼����AP����

#define TIMEZONE_ERASE  123
#define TIMEZONE_ERASE_OFFSET  0 //������ƫ�Ƶ�ַ�����洢ʱ��

#define TIME_Interva_ERASE  133  //���NTPʱ�����ݵ�ʱ����
#define TIME_Interva_ERASE_OFFSET  0

#define LOCAL_Erase   126  //���������ݼ�¼�ͻ����õı���IP�����أ�����
#define LOCAL_IP_ERASE_OFFSET   0 //������ƫ�Ƶ�ַ�����洢����IP��ַ
#define LOCAL_Mask_ERASE_OFFSET   96     //������ƫ�Ƶ�ַ�����洢������������
#define LOCAL_GATAWAY_ERASE_OFFSET   220     //������ƫ�Ƶ�ַ�����洢��������

#define FIRST_START_Erase   136  ///* ��������¼�Ƿ��һ�������豸 */
#define FIRST_START_ERASE_OFFSET   0 //

#define IS_AUTO_WEB_Erase   137  ///* ��������¼���Ƿ��Զ�������ҳ */
#define IS_AUTO_WEB_ERASE_OFFSET   0 //

#define TCP_PORT_Erase   138  ///* ��������¼TCP�������˿ں�  */
#define TCP_PORT_ERASE_OFFSET   0 //

#define TCP_SERVERIP_Erase   139            ///* ��������¼TCP�ͻ��˲���  */
#define TCP_SERVERIP_ERASE_OFFSET   0       ///* ��������¼TCP�ͻ���IP������  */
#define TCP_SERVERIPLen_ERASE_OFFSET   96    ///* ��������¼TCP�ͻ���IP����������  */
#define TCP_SERVER_PORT_ERASE_OFFSET   120   ///* ��������¼TCP�ͻ���IP�������Ķ˿�  */
#define TCP_SERVER_TPYE_ERASE_OFFSET   220   ///* ��������¼TCP���ͣ������ӻ��Ƕ�����  */

#define LANGUAGE_Erase   122            ///* ��������¼���԰汾����  */
#define LANGUAGE_ERASE_OFFSET   0       ///* ��������¼���԰汾����  */

#define LANGUAGE_EN     0x5a            /*Ӣ��汾*/
#define LANGUAGE_CH     0xa5            /*���İ汾*/

#define NO_DHCP_Erase   135  //
#define NO_DHCP_IP_ERASE_OFFSET   0 //
#define NO_DHCP_Mask_ERASE_OFFSET   96     //
#define NO_DHCP_GATAWAY_ERASE_OFFSET   220     //
#define DHCP_OPEN_ERASE_OFFSET   320     //

#define LOCALSSID_Erase   134  //���������ݼ�¼�ͻ����õ�SSID ����
#define SSID_ERASE_OFFSET   300  //������ƫ�Ƶ�ַ�����洢SSID����
#define SSID_Len_ERASE_OFFSET   444  //������ƫ�Ƶ�ַ�����洢SSID���ݳ���
#define PSWD_ERASE_OFFSET   400  //������ƫ�Ƶ�ַ�����洢��������
#define PSWD_LEN_ERASE_OFFSET   440  //������ƫ�Ƶ�ַ�����洢���볤��


#define NTP_IP_Erase   127  //���������ݼ�¼�ͻ����õ�NTP ip
#define NTP_IP_ERASE_OFFSET   0 //������ƫ�Ƶ�ַ�����洢NTP ip
#define NTP_IP_LEN_ERASE_OFFSET   220     //������ƫ�Ƶ�ַ����NTP IP�ĳ���

#define DST_SET    0x5a
#define DST_NO_SET 0Xa5

#define DHCP_CLOSE 0x5a
#define DHCP_OPEN  0xa5

#define AUTO_ACCESS_WEB     '1'
#define NO_AUTO_ACCESS_WEB  '2'

#define DST_Erase   128  //���������ݼ�¼�ͻ����õ�����ʱ
#define ISSET_DST_ERASE_OFFSET   0    //������ƫ�Ƶ�ַ������¼�Ƿ�Ҫ��������ʱ
#define DSTSTART_HOUR_ERASE_OFFSET   32 //������ƫ�Ƶ�ַ��������ʱ��ʼСʱ
#define DSTSTART_SELE_WEEK_ERASE_OFFSET   64    /*������ƫ�Ƶ�ַ�����洢����ʱ����ѡ�� */
#define DSTSTART_WEEK_ERASE_OFFSET 96      /* ������ƫ�Ƶ�ַ�����洢����ʱ���� */
#define DSTSTART_MON_ERASE_OFFSET  128      /* ������ƫ�Ƶ�ַ�����洢����ʱ�·� */

#define DSTEND_HOUR_ERASE_OFFSET   160 //������ƫ�Ƶ�ַ��������ʱ����Сʱ
#define DSTEND_SELE_WEEK_ERASE_OFFSET   192    /*������ƫ�Ƶ�ַ�����洢����ʱ����ѡ�� */
#define DSTEND_WEEK_ERASE_OFFSET 224      /* ������ƫ�Ƶ�ַ�����洢����ʱ���� */
#define DSTEND_MON_ERASE_OFFSET  256      /* ������ƫ�Ƶ�ַ�����洢����ʱ�·� */

#define DST_STRAT  0X5a
#define DST_FINISH 0Xa5
#define DST_TIMEOUT_Erase   129
#define DST_TIMEOUT_ERASE_OFFSET   0      //������ƫ�Ƶ�ַ������¼����ʱ�Ƿ�ʼ

#define NTP_MODE    0x5a
#define APP_MODE    0XA5

#define NTPAPP_Erase   130
#define NTPAPP_ERASE_OFFSET   0      //������ƫ�Ƶ�ַ������¼��NTPģʽ����APPģʽ

#define SEVER1_FLASH_Erase 530    //��ҳ1�Ĵ洢��ַ
#define SEVER2_FLASH_Erase 525    //��ҳ2�Ĵ洢��ַ
#define SEVER3_FLASH_Erase 510    //��ҳ3�Ĵ洢��ַ
#define SEVER4_FLASH_Erase 515    //��ҳ4�Ĵ洢��ַ
#define SEVER5_FLASH_Erase 520    //��ҳ5�Ĵ洢��ַ
#define INIT_SET    0x5a    //������Ҫ��λ��������
#define INIT_NO_SET 0xa5    //������Ҫ��λ����������

#define APPLINK   0x5a
#define NOAPPLINK 0xa5

#define WEB_SET_SUCCESS 0
#define WEB_SET_FAIL    1
#define ERROR   2
typedef struct{
	uint32 NtpseverIPlen;
	int32 Timezone;
	uint32 IP[4];
	uint32 gw[4];
	uint32 netmask[4];
	uint32 IsSetDST;
	uint32 NtpAppMode;
	uint32 SsidLen;
	uint32 SsidPswdLen;
	uint32 AP_num;
	uint32 TimePutInterva;
	uint32 DHCP_State;
	uint32 Auto_Web_State;
	uint32 TCP_Server_Port;
	uint32 TcpServerIP_Len;
	uint32 Language_state;
}AllSetData;
extern uint32 SetDevideCount;
void Spi_FlashWrite(uint32 Erase_Point,uint32 Erase_Offset,uint32 * data_point,uint32 data_len);
void Spi_FlashRead(uint32 Erase_Point,uint32 Erase_Offset,uint32 * data_point,uint32 data_len);

#endif

