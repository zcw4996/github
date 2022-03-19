#include "Rgbled.h"

void  RgbInit(void)
{


	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO4);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO5);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);
    GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 1);
	GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 1);
    GPIO_OUTPUT_SET(GPIO_ID_PIN(4), 1);
	GPIO_OUTPUT_SET(GPIO_ID_PIN(5), 1);
}

void ICACHE_FLASH_ATTR LedBlue(uint8_t State)
{
	if(State == 1)
	{
		GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 0);//GPIO14为高
	}
	else
	{
		GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 1);//GPIO14为高
	}
}
void ICACHE_FLASH_ATTR LedGreen(uint8_t State)
{
	if(State == 1)
	{
		GPIO_OUTPUT_SET(GPIO_ID_PIN(4), 0);//GPIO14为高
	}
	else
	{
		GPIO_OUTPUT_SET(GPIO_ID_PIN(4), 1);//GPIO14为高
	}
}
void ICACHE_FLASH_ATTR LedRed(uint8_t State)
{
	if(State == 1)
	{
		GPIO_OUTPUT_SET(GPIO_ID_PIN(5), 0);//GPIO14为高
	}
	else
	{
		GPIO_OUTPUT_SET(GPIO_ID_PIN(5), 1);//GPIO14为高
	}
}
void ICACHE_FLASH_ATTR LedCloseALL(void)
{
	LedBlue(0);
	LedRed(0);
	LedGreen(0);
}
void ICACHE_FLASH_ATTR LedOpenChoose(uint8_t State)
{
	switch(State)
	{
		case REDLED:LedRed(1); break;
		case GREENLED:LedGreen(1);break;
		case BLUELED:LedBlue(1);break;
		default:break;
	}
}
void ICACHE_FLASH_ATTR LedCloseChoose(uint8_t State)
{
	switch(State)
	{
		case REDLED:LedRed(0); break;
		case GREENLED:LedGreen(0);break;
		case BLUELED:LedBlue(0);break;
		default:break;
	}
}
