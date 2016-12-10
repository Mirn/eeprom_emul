#include "stm32kiss.h"
#include "hw.h"
#include "mem.h"
#include "mem_cmd.h"

typedef uint8_t tMEM_ADDR[3];

void mem_init()
{
	pin_output_v(PIN_MEM_CS,  1);
	pin_input(PIN_MEM_OUT);
	pin_output_v(PIN_MEM_WP,  1);
	pin_output_v(PIN_MEM_CLK, 0);
	pin_output_v(PIN_MEM_IN,  0);
}

void mem_done()
{
	pin_input(PIN_MEM_CS);
	pin_input(PIN_MEM_OUT);
	pin_input(PIN_MEM_WP);
	pin_input(PIN_MEM_CLK);
	pin_input(PIN_MEM_IN);
}

uint8_t mem_io(uint8_t in)
{
	uint8_t result = 0;
	pin_write(PIN_MEM_IN, (in & (1 << 7))); pin_write_vcc(PIN_MEM_CLK);    result = (result << 1) | pin_read(PIN_MEM_OUT);    pin_write_gnd(PIN_MEM_CLK);
	pin_write(PIN_MEM_IN, (in & (1 << 6))); pin_write_vcc(PIN_MEM_CLK);    result = (result << 1) | pin_read(PIN_MEM_OUT);    pin_write_gnd(PIN_MEM_CLK);
	pin_write(PIN_MEM_IN, (in & (1 << 5))); pin_write_vcc(PIN_MEM_CLK);    result = (result << 1) | pin_read(PIN_MEM_OUT);    pin_write_gnd(PIN_MEM_CLK);
	pin_write(PIN_MEM_IN, (in & (1 << 4))); pin_write_vcc(PIN_MEM_CLK);    result = (result << 1) | pin_read(PIN_MEM_OUT);    pin_write_gnd(PIN_MEM_CLK);
	pin_write(PIN_MEM_IN, (in & (1 << 3))); pin_write_vcc(PIN_MEM_CLK);    result = (result << 1) | pin_read(PIN_MEM_OUT);    pin_write_gnd(PIN_MEM_CLK);
	pin_write(PIN_MEM_IN, (in & (1 << 2))); pin_write_vcc(PIN_MEM_CLK);    result = (result << 1) | pin_read(PIN_MEM_OUT);    pin_write_gnd(PIN_MEM_CLK);
	pin_write(PIN_MEM_IN, (in & (1 << 1))); pin_write_vcc(PIN_MEM_CLK);    result = (result << 1) | pin_read(PIN_MEM_OUT);    pin_write_gnd(PIN_MEM_CLK);
	pin_write(PIN_MEM_IN, (in & (1 << 0))); pin_write_vcc(PIN_MEM_CLK);    result = (result << 1) | pin_read(PIN_MEM_OUT);    pin_write_gnd(PIN_MEM_CLK);
	return result;
}

uint8_t mem_rd()
{
	uint8_t result = 0;
	pin_write_vcc(PIN_MEM_CLK);    result = (result << 1) | PIN_MEM_OUT_BB;    pin_write_gnd(PIN_MEM_CLK);
	pin_write_vcc(PIN_MEM_CLK);    result = (result << 1) | PIN_MEM_OUT_BB;    pin_write_gnd(PIN_MEM_CLK);
	pin_write_vcc(PIN_MEM_CLK);    result = (result << 1) | PIN_MEM_OUT_BB;    pin_write_gnd(PIN_MEM_CLK);
	pin_write_vcc(PIN_MEM_CLK);    result = (result << 1) | PIN_MEM_OUT_BB;    pin_write_gnd(PIN_MEM_CLK);
	pin_write_vcc(PIN_MEM_CLK);    result = (result << 1) | PIN_MEM_OUT_BB;    pin_write_gnd(PIN_MEM_CLK);
	pin_write_vcc(PIN_MEM_CLK);    result = (result << 1) | PIN_MEM_OUT_BB;    pin_write_gnd(PIN_MEM_CLK);
	pin_write_vcc(PIN_MEM_CLK);    result = (result << 1) | PIN_MEM_OUT_BB;    pin_write_gnd(PIN_MEM_CLK);
	pin_write_vcc(PIN_MEM_CLK);    result = (result << 1) | PIN_MEM_OUT_BB;    pin_write_gnd(PIN_MEM_CLK);
	return result;
}

void mem_wr(uint8_t in)
{
	pin_write(PIN_MEM_IN, (in & (1 << 7))); pin_write_vcc(PIN_MEM_CLK);    pin_write_gnd(PIN_MEM_CLK);
	pin_write(PIN_MEM_IN, (in & (1 << 6))); pin_write_vcc(PIN_MEM_CLK);    pin_write_gnd(PIN_MEM_CLK);
	pin_write(PIN_MEM_IN, (in & (1 << 5))); pin_write_vcc(PIN_MEM_CLK);    pin_write_gnd(PIN_MEM_CLK);
	pin_write(PIN_MEM_IN, (in & (1 << 4))); pin_write_vcc(PIN_MEM_CLK);    pin_write_gnd(PIN_MEM_CLK);
	pin_write(PIN_MEM_IN, (in & (1 << 3))); pin_write_vcc(PIN_MEM_CLK);    pin_write_gnd(PIN_MEM_CLK);
	pin_write(PIN_MEM_IN, (in & (1 << 2))); pin_write_vcc(PIN_MEM_CLK);    pin_write_gnd(PIN_MEM_CLK);
	pin_write(PIN_MEM_IN, (in & (1 << 1))); pin_write_vcc(PIN_MEM_CLK);    pin_write_gnd(PIN_MEM_CLK);
	pin_write(PIN_MEM_IN, (in & (1 << 0))); pin_write_vcc(PIN_MEM_CLK);    pin_write_gnd(PIN_MEM_CLK);
}

void mem_start()
{
	pin_write_vcc(PIN_MEM_CS);
	pin_write_gnd(PIN_MEM_CLK);
	pin_write_gnd(PIN_MEM_IN);
	pin_write_gnd(PIN_MEM_CS);
}

void mem_stop()
{
	pin_write_gnd(PIN_MEM_CLK);
	pin_write_gnd(PIN_MEM_IN);
	pin_write_vcc(PIN_MEM_CS);
}

void mem_read(uint32_t addr, uint8_t *data, uint32_t size)
{
	mem_start();
	mem_wr(MEM_CMD_READ);
	mem_wr(addr >> (8 * 2));
	mem_wr(addr >> (8 * 1));
	mem_wr(addr >> (8 * 0));

	while (size-- > 0)
		*(data++) = mem_rd();

	mem_stop();
}

void mem_cmd(uint8_t cmd)
{
	mem_start();
	mem_wr(cmd);
	mem_stop();
}

uint8_t mem_status()
{
	mem_start();
	mem_wr(MEM_CMD_STATUS_READ);
	uint8_t result = mem_rd();
	mem_stop();
	return result;
}

bool mem_status_wait(uint8_t mask, bool need, int32_t timeout_us)
{
	while (timeout_us > 0)
	{
		int32_t wait_us = MIN(timeout_us, 50);
		delay_us(wait_us);
		timeout_us -= wait_us;

		bool value = (mem_status() & mask) != 0;
		if (value == need)
			return true;
	}
	return false;
}

bool mem_wait_program(uint32_t timeout_us)
{
	bool result = mem_status_wait(MEM_STATUS_WRITE_PROGRESS, 0, timeout_us);
	if (!result)
		printf("ERROR: write timeout\r");
	return result;
}

void mem_write_raw(uint32_t addr, uint8_t *data, uint32_t size)
{
	mem_start();

	mem_wr(MEM_CMD_PAGE_PROGRAM);
	mem_wr((addr >> (8 * 2)) & 0xFF);
	mem_wr((addr >> (8 * 1)) & 0xFF);
	mem_wr((addr >> (8 * 0)) & 0xFF);

	while (size--)
		mem_wr(*(data++));

	mem_stop();
}

bool mem_write(uint32_t addr, void *data, uint32_t size)
{
	mem_write_enable();
	mem_write_raw(addr, (uint8_t*)data, size);
	bool result = mem_wait_program(MEM_TIMEOUT_PAGE_PROGRAM);
	mem_write_disable();
	return result;
}

bool mem_bulk_erase()
{
	mem_write_enable();
	mem_cmd(MEM_CMD_BULK_ERASE);
	bool result = mem_wait_program(MEM_TIMEOUT_BULK_ERASE);
	mem_write_disable();
	return result;
}

void mem_sector_erase_raw(uint32_t addr)
{
	mem_start();

	mem_wr(MEM_CMD_SECTOR_ERASE);
	mem_wr((addr >> (8 * 2)) & 0xFF);
	mem_wr((addr >> (8 * 1)) & 0xFF);
	mem_wr((addr >> (8 * 0)) & 0xFF);

	mem_stop();
}

bool mem_sector_erase(uint32_t addr)
{
	mem_write_enable();
	mem_sector_erase_raw(addr);
	bool result = mem_wait_program(MEM_TIMEOUT_SECTOR_ERASE);
	mem_write_disable();
	return result;
}
