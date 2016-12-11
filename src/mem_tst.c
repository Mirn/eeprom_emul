/*
 * mem_tst.c
 *
 *  Created on: 11 дек. 2016 г.
 *      Author: Евгений
 */

#include "stm32kiss.h"
#include "hw.h"
#include "mem.h"
#include "mem_cmd.h"
#include "flash_module.h"

static uint8_t rd_buf[16] = {0};
static uint8_t wr_buf_a[4] = {0x1F, 0x2F, 0x3F, 0x4F};
static uint8_t wr_buf_b[4] = {0xFE, 0xFD, 0xFC, 0xFB};

static void rd_func(uint8_t *data, uint16_t size)
{
	for (uint32_t pos = 0; pos < size; pos++)
		printf("%02X ", data[pos]);
	printf("\r\n");
}

void mem_tst()
{
	printf("=============== memtest ===========\r\n");

	uint32_t pos = 0;
	uint32_t addr = 0x40000;

	mem_sector_erase(addr);

	memset(rd_buf, 0xAA, sizeof(rd_buf));
	mem_read(addr, rd_buf, sizeof(rd_buf));
	for (pos = 0; pos < LENGTH(rd_buf); pos++)
		printf("%02X ", rd_buf[pos]);
	printf("\r\n");

	uint8_t a = 0xF0;
	mem_write(addr+3, &a, 1);

	memset(rd_buf, 0xAA, sizeof(rd_buf));
	mem_read(addr, rd_buf, sizeof(rd_buf));
	for (pos = 0; pos < LENGTH(rd_buf); pos++)
		printf("%02X ", rd_buf[pos]);
	printf("\r\n");

	uint8_t b = 0x0F;
	mem_write(addr+3, &b, 1);

	memset(rd_buf, 0xAA, sizeof(rd_buf));
	mem_read(addr, rd_buf, sizeof(rd_buf));
	for (pos = 0; pos < LENGTH(rd_buf); pos++)
		printf("%02X ", rd_buf[pos]);
	printf("\r\n");

	memset(rd_buf, 0xAA, sizeof(rd_buf));
	mem_read(0x10000, rd_buf, sizeof(rd_buf));
	for (pos = 0; pos < LENGTH(rd_buf); pos++)
		printf("%02X ", rd_buf[pos]);
	printf("\r\n");

	flash_module_read(addr, rd_buf, sizeof(rd_buf), rd_func);
	flash_module_read(0x10000, rd_buf, sizeof(rd_buf), rd_func);
	flash_module_erase(4);
	flash_module_read(0x10001, rd_buf, sizeof(rd_buf), rd_func);
	flash_module_read(0x10002, rd_buf, sizeof(rd_buf), rd_func);
	flash_module_read(0x10003, rd_buf, sizeof(rd_buf), rd_func);
	printf("\r\n");

	flash_module_read(addr, rd_buf, sizeof(rd_buf), rd_func);
	flash_module_write(addr + 4, wr_buf_a, sizeof(wr_buf_a));
	flash_module_read(addr, rd_buf, sizeof(rd_buf), rd_func);
	flash_module_write(addr + 5, wr_buf_b, sizeof(wr_buf_b));
	flash_module_read(addr, rd_buf, sizeof(rd_buf), rd_func);
	printf("\r\n");

	printf("===================================\r\n\r\n");
}
