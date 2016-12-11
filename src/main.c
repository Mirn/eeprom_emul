#include "stm32kiss.h"
#include "hw.h"
#include "mem.h"
#include "mem_cmd.h"
#include "flash_module.h"
#include "mem_tst.h"

void main(void)
{
	SetSysClockTo24();
	printf_usart_reinit();

	printf("\r\n\r\n========================================\r\nEEPROM_EMUL program started\r\n\r\n");
	printf("SystemCoreClock = %i Hz\r\n", SystemCoreClock);
	printf("Chip ID number: %x %x %x %x; %x\r\n", DEVICE_ID_1, DEVICE_ID_2, DEVICE_ID_3, DEVICE_ID_4, DEVICE_ID_HASH);
	printf("GCC version %i.%i.%i\r\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
	printf("\r\n");

	POWER_ON();
	delay_ms(10);

	flash_module_init();

	mem_tst();

	while (true)
	{
		flash_module_proc();
	}
}
