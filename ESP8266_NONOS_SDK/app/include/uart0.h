/*
 * uart0.h
 *
 *  Created on: 2017年1月24日
 *      Author: dell
 */

#ifndef APP_INCLUDE_UART0_H_
#define APP_INCLUDE_UART0_H_

#include "ets_sys.h"
#include "osapi.h"
#include "uart0.h"
#include "uart_register.h"
#include "uart.h"
#define UART0   0

//typedef enum
//{
//	FIVE_BITS = 0x0,
//	SIX_BITS = 0x1,
//	SEVEN_BITS = 0x2,
//	EIGHT_BITS = 0x3
//} UartBitsNum; //数据位个数：5、6、7、8

//typedef enum
//{
//	ONE_STOP_BIT             = 0x1,
//	ONE_HALF_STOP_BIT        = 0x2,
//	TWO_STOP_BIT             = 0x3
//} UartStopBitsNum;  //停止位个数：1、1.5、2

//typedef enum
//{
//	NONE_BITS = 0x2,
//	ODD_BITS   = 1,
//	EVEN_BITS = 0
//} UartParityMode;   //校验方式：无、奇校验、偶校验

//typedef enum
//{
//	STICK_PARITY_DIS   = 0,
//	STICK_PARITY_EN    = 1
//} UartParityEN;    //开启校验与否：开、 不开

//typedef enum
//{
//	BIT_RATE_300 = 300,
//	BIT_RATE_600 = 600,
//	BIT_RATE_1200 = 1200,
//	BIT_RATE_2400 = 2400,
//	BIT_RATE_4800 = 4800,
//	BIT_RATE_9600   = 9600,
//	BIT_RATE_19200  = 19200,
//	BIT_RATE_38400  = 38400,
//	BIT_RATE_57600  = 57600,
//	BIT_RATE_74880  = 74880,
//	BIT_RATE_115200 = 115200,
//	BIT_RATE_230400 = 230400,
//	BIT_RATE_460800 = 460800,
//	BIT_RATE_921600 = 921600,
//	BIT_RATE_1843200 = 1843200,
//	BIT_RATE_3686400 = 3686400,
//} UartBautRate;         //波特率

typedef struct
{
	UartBautRate 	  baut_rate;      //波特率
	UartBitsNum4Char       data_bits;      //数据位个数
	UartExistParity      en_parity;      //校验开启与否
	UartParityMode 	  parity;         //校验方式
	UartStopBitsNum   stop_bits;      //停止位个数
} UartControlblock;                   //串口控制单元，包含5项

void ICACHE_FLASH_ATTR uart0_init(UartControlblock *Ucb);
LOCAL void uart0_rx_intr_handler(void *para);
//void ICACHE_FLASH_ATTR uart_tx_one_char(u8 data);
STATUS uart_tx_one_char1(uint8 uart, uint8 TxChar);
void ICACHE_FLASH_ATTR uart0_tx_buffer(uint8 *buf, uint16 len);
#endif /* APP_INCLUDE_UART0_H_ */
