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

	state = (state + 7) % (FLASH_VARS_COUNT - 2);
	flash_vars[2 + state]++;

	state = (state + 7) % (FLASH_VARS_COUNT - 2);
	flash_vars[2 + state]++;

	state = (state + 7) % (FLASH_VARS_COUNT - 2);
	flash_vars[2 + state]++;

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

	if (flash_vars_read_init_error())
		for (uint32_t pos = 0; pos < FLASH_VARS_COUNT; pos++)
			flash_vars[pos] = pos;

	systick_on(1, second_tick);

	uint32_t divider = 0;
	while (true)
	{
		flash_module_proc();

		flash_vars[0]++;
		divider++;
		if (divider > 100000)
		{
			divider = 0;
			flash_vars[1]++;
//			static uint8_t buf;
//			while (flash_module_read(0, &buf, 1, NULL) == FLASH_SUCCESS)
//				printf("flash_module_read!\r\n");
//			flash_vars[0]++;
		}

		flash_vars_proc();
	}
}
