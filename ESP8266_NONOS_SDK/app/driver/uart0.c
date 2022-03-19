/*
 * uart0.c
 *
 *  Created on: 2017��1��24��
 *      Author: dell
 */

#include "uart0.h"
#include "soft_timer.h"
#include "ets_sys.h"
#include "osapi.h"
#include "uart.h"
#include "osapi.h"
#include "uart_register.h"
#include "mem.h"
#include "os_type.h"
#include "espconn.h"
#include "tcpserver.h"
#include "tcpclient.h"
uint8 UART0_RX_BUFF[256];        //����0������������
uint8  data_len;         //FIFO��������
extern struct espconn esp_conn_App;   //����һ��espconn�ṹ��
/*-------------------------------------------------------------*/
/*�������ܣ�����0�����жϺ���                                                                           */
/*��       ����para ��ָ��RcvMsgBuff��ָ��                                                      */
/*��       �أ���                                                                                                     */
/*-------------------------------------------------------------*/
extern uint32 NtpappMode;
LOCAL void uart0_rx_intr_handler(void *para)
{
    u32 uart0_intr_sta;   //�����ж�״̬

    u8  i;
    uint32 TcpType;
    uart0_intr_sta=READ_PERI_REG(UART_INT_ST(UART0));    //��ȡ����0�ж�״̬

	if(UART_RXFIFO_TOUT_INT_ST==(uart0_intr_sta&UART_RXFIFO_TOUT_INT_ST))    //����Ǵ��ڽ��ճ�ʱ�ж�
	{
		data_len=(READ_PERI_REG(UART_STATUS(UART0))>>UART_RXFIFO_CNT_S)&UART_RXFIFO_CNT; //��ȡFIFO��������
		if(data_len > 256)
		{
			WRITE_PERI_REG(UART_INT_CLR(UART0), 0xffff);         //���ȫ���жϱ�־
			return;
		}
		for(i=0;i<data_len;i++)
		{
		   UART0_RX_BUFF[i]=READ_PERI_REG(UART_FIFO(UART0))&0xFF; //�������ݵ�UART0_RX_BUFF����
		}
		     UART0_RX_BUFF[data_len] = '\0';

			 if(strcmp(UART0_RX_BUFF,"Reset") == 0)
			  {
				 Init_Setup = INIT_SET;
				 Spi_FlashWrite(Init_Erase, 0,&Init_Setup, 1);
				 os_printf("Restart\n");
				 system_restore();
				 system_restart();      //����
			  }
			 SendTcpClientData(UART0_RX_BUFF,data_len);/* ͨ��TCP�������� */

		//os_printf("data_len = %d\n%s\r\n",data_len,UART0_RX_BUFF);//�յ�������ԭ������
		//os_memset(UART0_RX_BUFF,0,256);   //���������ݺ����UART0_RX_BUFF
	}

	WRITE_PERI_REG(UART_INT_CLR(UART0), 0xffff);         //���ȫ���жϱ�־
}

/*-------------------------------------------------------------*/
/*�������ܣ�����0��ʼ��                                                                                     */
/*��       ����Ucb�����ڿ��ƿ�                                                                              */
/*��       �أ���                                                                                                     */
/*-------------------------------------------------------------*/
void ICACHE_FLASH_ATTR uart0_init(UartControlblock *Ucb)
{
	ETS_UART_INTR_ATTACH(uart0_rx_intr_handler, NULL);         //���ô����жϺ���
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD);       //���ù����Ǵ���0TX
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_U0RXD);       //���ù����Ǵ���0RX

	uart_div_modify(UART0, UART_CLK_FREQ / (Ucb->baut_rate));  //���ò�����
	SET_PERI_REG_MASK(UART_CONF0(UART0), ((Ucb->en_parity & UART_PARITY_EN_M)  <<  UART_PARITY_EN_S));   //����У��ʹ�����
	SET_PERI_REG_MASK(UART_CONF0(UART0), ((Ucb->parity & UART_PARITY_M)  << UART_PARITY_S));             //����У�鷽ʽ
	SET_PERI_REG_MASK(UART_CONF0(UART0), ((Ucb->stop_bits & UART_STOP_BIT_NUM) << UART_STOP_BIT_NUM_S)); //����ֹͣλ����
	SET_PERI_REG_MASK(UART_CONF0(UART0), ((Ucb->data_bits & UART_BIT_NUM) << UART_BIT_NUM_S));           //��������λ����

	SET_PERI_REG_MASK(UART_CONF0(UART0), UART_RXFIFO_RST | UART_TXFIFO_RST);    //��λ�շ�FIFO
	CLEAR_PERI_REG_MASK(UART_CONF0(UART0), UART_RXFIFO_RST | UART_TXFIFO_RST);  //ֹͣ��λ�շ�FIFO

	SET_PERI_REG_MASK(UART_CONF1(UART0),((0x02 & UART_RX_TOUT_THRHD) << UART_RX_TOUT_THRHD_S)); //����FIFO���ճ�ʱ�ж���ֵ2��  ��λ�ǽ���1���ֽڵ�ʱ��
	SET_PERI_REG_MASK(UART_CONF1(UART0),UART_RX_TOUT_EN);             //ʹ����������ʱ�жϹ���
	SET_PERI_REG_MASK(UART_INT_ENA(UART0), UART_RXFIFO_TOUT_INT_ENA); //ʹ�ܳ�ʱ�����ж�
	WRITE_PERI_REG(UART_INT_CLR(UART0), 0xffff);                      //���ȫ���жϱ�־
	ETS_UART_INTR_ENABLE();                                           //���������ж�
}
/******************************************************************************
 * FunctionName : uart1_tx_one_char
 * Description  : Internal used function
 *                Use uart1 interface to transfer one char
 * Parameters   : uint8 TxChar - character to tx
 * Returns      : OK
*******************************************************************************/
 STATUS uart_tx_one_char1(uint8 uart, uint8 TxChar)
{
    while (true){
        uint32 fifo_cnt = READ_PERI_REG(UART_STATUS(uart)) & (UART_TXFIFO_CNT<<UART_TXFIFO_CNT_S);
        if ((fifo_cnt >> UART_TXFIFO_CNT_S & UART_TXFIFO_CNT) < 126) {
            break;
        }
    }
    WRITE_PERI_REG(UART_FIFO(uart) , TxChar);
    return OK;
}
 /******************************************************************************
  * FunctionName : uart0_tx_buffer
  * Description  : use uart0 to transfer buffer
  * Parameters   : uint8 *buf - point to send buffer
  *                uint16 len - buffer len
  * Returns      :
 *******************************************************************************/
// void ICACHE_FLASH_ATTR
// uart0_tx_buffer(uint8 *buf, uint16 len)
// {
//     uint16 i;
//     for (i = 0; i < len; i++)
//     {
//         uart_tx_one_char1(UART0, buf[i]);
//     }
// }

