#include "stm32kiss.h"
#include "hw.h"
#include "mem.h"
#include "mem_cmd.h"
#include "flash_module.h"
#include "mem_tst.h"
#include "flash_vars.h"

void second_tick()
{
	static uint32_t state = 0;
	state = (state + 7) % FLASH_VARS_COUNT;
	flash_vars[state]++;
	state = (state + 7) % FLASH_VARS_COUNT;
	flash_vars[state]++;
	state = (state + 7) % FLASH_VARS_COUNT;
	flash_vars[state]++;

	//printf("\r\n");
	//printf("tick\t");
	for (uint32_t pos = 0; pos < FLASH_VARS_COUNT; pos++)
		printf("%i\t", flash_vars[pos]);
	printf("\r\n");
	//printf("\r\n");
}

void main(void)
{
	//SetSysClockTo24();
	//printf_usart_reinit();

	printf("\r\n\r\n========================================\r\nEEPROM_EMUL program started\r\n\r\n");
	printf("SystemCoreClock = %i Hz\r\n", SystemCoreClock);
	printf("Chip ID number: %x %x %x %x; %x\r\n", DEVICE_ID_1, DEVICE_ID_2, DEVICE_ID_3, DEVICE_ID_4, DEVICE_ID_HASH);
	printf("GCC version %i.%i.%i\r\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
	printf("\r\n");

	POWER_ON();
	delay_ms(10);

	flash_module_init();
	flash_vars_init(false);

	//mem_tst();

	while (!flash_vars_read_init_done())
	{
		flash_module_proc();
		flash_vars_proc();
	}

	//delay_ms(1000);

//	if (flash_vars_read_init_error())
//		for (uint32_t pos = 0; pos < FLASH_VARS_COUNT; pos++)
//			flash_vars[pos] = pos;

	systick_on(100, second_tick);

	while (true)
	{
		flash_module_proc();
		flash_vars_proc();
	}
}
