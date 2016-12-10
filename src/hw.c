#include "stm32kiss.h"
#include "hw.h"

void SetSysClockTo24(void)
{
#if defined(USE_STDPERIPH_DRIVER) | defined(STM32F10X_MD_VL)
    RCC_DeInit();
    RCC_HSICmd(ENABLE);

    FLASH_PrefetchBufferCmd( FLASH_PrefetchBuffer_Enable);
    FLASH_SetLatency(FLASH_Latency_0);

    RCC_HCLKConfig(RCC_SYSCLK_Div1);
    RCC_PCLK2Config(RCC_HCLK_Div1);
    RCC_PCLK1Config(RCC_HCLK_Div1);
    RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_6);

    RCC_PLLCmd(ENABLE);
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
    while (RCC_GetSYSCLKSource() != 0x08);
    SystemCoreClockUpdate();
	//printf_usart_reinit();
#endif
};

void SetSysClockTo8(void)
{
#if defined(USE_STDPERIPH_DRIVER) | defined(STM32F10X_MD_VL)
    RCC_DeInit();
    SystemCoreClockUpdate();
	//printf_usart_reinit();
#endif
};

unsigned int root1(unsigned int a)
{
   unsigned int x;
   x = (a/0x3f + 0x3f)>>1;
   x = (a/x + x)>>1;
   x = (a/x + x)>>1;
   return(x);
}

tCharge_Status charge_status_read()
{
#if defined(USE_STDPERIPH_DRIVER) | defined(STM32F10X_MD_VL)
	bool status_up = false;
	bool status_down = false;

	pin_input_down(PIN_CHARGE_STATUS);
	delay_us(50);
	status_down = pin_read(PIN_CHARGE_STATUS);

	pin_input_up(PIN_CHARGE_STATUS);
	delay_us(50);
	status_up = pin_read(PIN_CHARGE_STATUS);

	pin_input(PIN_CHARGE_STATUS);

	if ((status_down == true)  && (status_up == false)) return Charge_Error;
	if ((status_down == false) && (status_up == true))  return Charge_NC;
	if ((status_down == false) && (status_up == false)) return Charge_Process;
	if ((status_down == true)  && (status_up == true))  return Charge_Complete;
	return Charge_Error;
#else
	return Charge_Complete;
#endif
}

const char *charge_status_str(tCharge_Status status)
{
	if (status == Charge_NC) return "Charge_NC";
	if (status == Charge_Process) return "Charge_Process";
	if (status == Charge_Complete) return "Charge_Complete";
	return "Charge_Error";
}


#if !(defined(USE_STDPERIPH_DRIVER) | defined(STM32F10X_MD_VL))

KS_PIN __PIN_B0;
KS_PIN __PIN_B1;
KS_PIN __PIN_B2;
KS_PIN __PIN_B3;
KS_PIN __PIN_B4;
KS_PIN __PIN_B5;
KS_PIN __PIN_B6;
KS_PIN __PIN_B7;
KS_PIN __PIN_B8;
KS_PIN __PIN_B9;
KS_PIN __PIN_B10;
KS_PIN __PIN_B11;
KS_PIN __PIN_B12;
KS_PIN __PIN_B13;
KS_PIN __PIN_B14;
KS_PIN __PIN_B15;

KS_PIN __PIN_A0;
KS_PIN __PIN_A1;
KS_PIN __PIN_A2;
KS_PIN __PIN_A3;
KS_PIN __PIN_A4;
KS_PIN __PIN_A5;
KS_PIN __PIN_A6;
KS_PIN __PIN_A7;
KS_PIN __PIN_A8;
KS_PIN __PIN_A9;
KS_PIN __PIN_A10;
KS_PIN __PIN_A11;
KS_PIN __PIN_A12;
KS_PIN __PIN_A13;
KS_PIN __PIN_A14;
KS_PIN __PIN_A15;

void pin_input_down(KS_PIN *pin)
{
	pin->value  = false;
	pin->invert = false;
}

void pin_input_up(KS_PIN *pin)
{
	pin->value  = true;
	pin->invert = true;
}

void pin_input(KS_PIN *pin)
{
	pin->value  = false;
	pin->invert = false;
}

bool pin_read(const KS_PIN *pin)
{
	return pin->value;
}

void printf_usart_reinit()
{
}

static void (*systick_func)() = NULL;

void systick_on(uint16_t freq, void (*func)())
{
	systick_func = func;
}

HWND hDrawWindow = 0;
#define WM_CW5_REDRAW   WM_USER + 0x1E16
#define WM_CW5_REDRAW_A 0x92349312
#define WM_CW5_REDRAW_B 0x1AB4C129

void __WFI()
{
	if (hDrawWindow != 0)
		SendMessageA(hDrawWindow, WM_CW5_REDRAW, WM_CW5_REDRAW_A, WM_CW5_REDRAW_B);

/*	static uint32_t next_time = 0;
	static uint8_t  div = 0;

	if (next_time == 0)
		next_time = GetTickCount();

	next_time += ((div % 4) == 0) ? 32 : 31;
	div++;

	while (GetTickCount() < next_time)
		Sleep(1);*/

	static LARGE_INTEGER freq = {QuadPart:0};
	if (freq.QuadPart == 0)
		QueryPerformanceFrequency(&freq);

	static LARGE_INTEGER adder = {QuadPart:0};
	if (adder.QuadPart == 0)
		adder.QuadPart = freq.QuadPart / 32;

	static LARGE_INTEGER time = {QuadPart:0};
	QueryPerformanceCounter(&time);

	static LARGE_INTEGER next_time = {QuadPart:0};
	if (next_time.QuadPart == 0)
		next_time.QuadPart = time.QuadPart;
	next_time.QuadPart += adder.QuadPart;

	int32_t ms = 0;
	uint32_t dlt = 16;

	do
	{
		ms = 1000.0 * ((double)(next_time.QuadPart - time.QuadPart)) / ((double)freq.QuadPart);
		if (ms > dlt)
			Sleep(dlt);
		else
			dlt = dlt >> 1;
		QueryPerformanceCounter(&time);
	}
	while ((next_time.QuadPart > time.QuadPart) && (dlt > 0));


	if (systick_func != NULL)
		(*systick_func)();
}

uint8_t *flash_mem_data = NULL;
const uint32_t flash_mem_size = 0x400000;

void mem_read(uint32_t addr, uint8_t *data, uint32_t size)
{
	if (flash_mem_data == NULL)
		memset(data, 0xFF, size);
	else
		while (size--)
			*(data++) = flash_mem_data[(addr++) & (flash_mem_size - 1)];
}

void time_set(uint8_t hour, uint8_t minute){};
void backup_work_mode(bool value){};

#endif
