#include "stm32kiss.h"
#include "hw.h"
#include "mem.h"
#include "mem_cmd.h"
#include "flash_module.h"

const char src_ver_main[] = __DATE__"\t"__TIME__"\t"__FILE__"\r";

void rd_func(uint8_t *data, uint16_t size)
{
	for (uint32_t pos = 0; pos < size; pos++)
		printf("%02X ", data[pos]);
	printf("\r\n");
}

void main(void)
{
	printf("\r\n\r\n========================================\r\nEEPROM_EMUL program started\r\n\r\n");
	printf("SystemCoreClock = %i Hz\r\n", SystemCoreClock);
	printf("Chip ID number: %x %x %x %x; %x\r\n", DEVICE_ID_1, DEVICE_ID_2, DEVICE_ID_3, DEVICE_ID_4, DEVICE_ID_HASH);
	printf("GCC version %i.%i.%i\r\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
	printf("\r\n");

	POWER_ON();
	delay_ms(10);

	flash_module_init();

	printf("=============== memtest ===========\r\n");

	uint32_t pos = 0;
	uint8_t buf[16] = {0};
	uint32_t addr = 0x40000;

	mem_sector_erase(addr);

	memset(buf, 0xAA, sizeof(buf));
	mem_read(addr, buf, sizeof(buf));
	for (pos = 0; pos < LENGTH(buf); pos++)
		printf("%02X ", buf[pos]);
	printf("\r\n");

	uint8_t a = 0xF0;
	mem_write(addr+3, &a, 1);

	memset(buf, 0xAA, sizeof(buf));
	mem_read(addr, buf, sizeof(buf));
	for (pos = 0; pos < LENGTH(buf); pos++)
		printf("%02X ", buf[pos]);
	printf("\r\n");

	uint8_t b = 0x0F;
	mem_write(addr+3, &b, 1);

	memset(buf, 0xAA, sizeof(buf));
	mem_read(addr, buf, sizeof(buf));
	for (pos = 0; pos < LENGTH(buf); pos++)
		printf("%02X ", buf[pos]);
	printf("\r\n");

	memset(buf, 0xAA, sizeof(buf));
	mem_read(0x10000, buf, sizeof(buf));
	for (pos = 0; pos < LENGTH(buf); pos++)
		printf("%02X ", buf[pos]);
	printf("\r\n");

	flash_module_read(addr, buf, sizeof(buf), rd_func);
	flash_module_read(0x10000, buf, sizeof(buf), rd_func);
	flash_module_erase(4);
	flash_module_read(0x10001, buf, sizeof(buf), rd_func);
	flash_module_read(0x10002, buf, sizeof(buf), rd_func);
	flash_module_read(0x10003, buf, sizeof(buf), rd_func);
	printf("\r\n");

	uint8_t b_a[4] = {0x1F, 0x2F, 0x3F, 0x4F};
	uint8_t b_b[4] = {0xFE, 0xFD, 0xFC, 0xFB};

	flash_module_read(addr, buf, sizeof(buf), rd_func);
	flash_module_write(addr + 4, b_a, sizeof(b_a));
	flash_module_read(addr, buf, sizeof(buf), rd_func);
	flash_module_write(addr + 5, b_b, sizeof(b_b));
	flash_module_read(addr, buf, sizeof(buf), rd_func);
	printf("\r\n");

	printf("===================================\r\n\r\n");

	while (true)
		flash_module_proc();
}
