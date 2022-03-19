/*
 * uart0.c
 *
 *  Created on: 2017年1月24日
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
#include "init.h"
uint8 UART0_RX_BUFF[256];        //串口0接收数据数组
uint8  data_len;         //FIFO内数据量
extern struct espconn esp_conn_App;   //建立一个espconn结构体
/*-------------------------------------------------------------*/
/*函数功能：串口0接收中断函数                                                                           */
/*参       数：para ：指向RcvMsgBuff的指针                                                      */
/*返       回：无                                                                                                     */
/*-------------------------------------------------------------*/
extern uint32 NtpappMode;
LOCAL void uart0_rx_intr_handler(void *para)
{
    u32 uart0_intr_sta;   //保存中断状态

    u8  i;
    uint32 TcpType;
    uart0_intr_sta=READ_PERI_REG(UART_INT_ST(UART0));    //读取串口0中断状态

	if(UART_RXFIFO_TOUT_INT_ST==(uart0_intr_sta&UART_RXFIFO_TOUT_INT_ST))    //如果是串口接收超时中断
	{
		data_len=(READ_PERI_REG(UART_STATUS(UART0))>>UART_RXFIFO_CNT_S)&UART_RXFIFO_CNT; //获取FIFO内数据量
		if(data_len > 256)
		{
			WRITE_PERI_REG(UART_INT_CLR(UART0), 0xffff);         //清除全部中断标志
			return;
		}
		for(i=0;i<data_len;i++)
		{
		   UART0_RX_BUFF[i]=READ_PERI_REG(UART_FIFO(UART0))&0xFF; //拷贝数据到UART0_RX_BUFF数组
		}
		     UART0_RX_BUFF[data_len] = '\0';

			 if(strcmp(UART0_RX_BUFF,"Reset") == 0)
			  {
				 Init_Setup = INIT_SET;
				 Spi_FlashWrite(Init_Erase, 0,&Init_Setup, 1);
				 DNS_SERVER_DEBUG("Restart\n");
				 system_restore();
				 system_restart();      //重启
			  }
			 SendTcpClientData(UART0_RX_BUFF,data_len);/* 通过TCP发送数据 */

		//DNS_SERVER_DEBUG("data_len = %d\n%s\r\n",data_len,UART0_RX_BUFF);//收到的数据原样发回
		//os_memset(UART0_RX_BUFF,0,256);   //发送完数据后，清空UART0_RX_BUFF
	}

	WRITE_PERI_REG(UART_INT_CLR(UART0), 0xffff);         //清除全部中断标志
}

/*-------------------------------------------------------------*/
/*函数功能：串口0初始化                                                                                     */
/*参       数：Ucb：串口控制块                                                                              */
/*返       回：无                                                                                                     */
/*-------------------------------------------------------------*/
void ICACHE_FLASH_ATTR uart0_init(UartControlblock *Ucb)
{
	ETS_UART_INTR_ATTACH(uart0_rx_intr_handler, NULL);         //设置串口中断函数
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD);       //设置功能是串口0TX
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_U0RXD);       //设置功能是串口0RX

	uart_div_modify(UART0, UART_CLK_FREQ / (Ucb->baut_rate));  //设置波特率
	SET_PERI_REG_MASK(UART_CONF0(UART0), ((Ucb->en_parity & UART_PARITY_EN_M)  <<  UART_PARITY_EN_S));   //设置校验使能与否
	SET_PERI_REG_MASK(UART_CONF0(UART0), ((Ucb->parity & UART_PARITY_M)  << UART_PARITY_S));             //设置校验方式
	SET_PERI_REG_MASK(UART_CONF0(UART0), ((Ucb->stop_bits & UART_STOP_BIT_NUM) << UART_STOP_BIT_NUM_S)); //设置停止位个数
	SET_PERI_REG_MASK(UART_CONF0(UART0), ((Ucb->data_bits & UART_BIT_NUM) << UART_BIT_NUM_S));           //设置数据位个数

	SET_PERI_REG_MASK(UART_CONF0(UART0), UART_RXFIFO_RST | UART_TXFIFO_RST);    //复位收发FIFO
	CLEAR_PERI_REG_MASK(UART_CONF0(UART0), UART_RXFIFO_RST | UART_TXFIFO_RST);  //停止复位收发FIFO

	SET_PERI_REG_MASK(UART_CONF1(UART0),((0x02 & UART_RX_TOUT_THRHD) << UART_RX_TOUT_THRHD_S)); //设置FIFO接收超时中断阈值2，  单位是接收1个字节的时间
	SET_PERI_REG_MASK(UART_CONF1(UART0),UART_RX_TOUT_EN);             //使能上述，超时中断功能
	SET_PERI_REG_MASK(UART_INT_ENA(UART0), UART_RXFIFO_TOUT_INT_ENA); //使能超时接收中断
	WRITE_PERI_REG(UART_INT_CLR(UART0), 0xffff);                      //清除全部中断标志
	ETS_UART_INTR_ENABLE();                                           //开串口总中断
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

