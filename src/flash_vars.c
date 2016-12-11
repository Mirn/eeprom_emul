/*
 * flash_vars.c
 *
 *  Created on: 11 дек. 2016 г.
 *      Author: Евгений
 */

#include "stm32kiss.h"
#include "flash_module.h"
#include "flash_vars.h"

#define log(...) {}
//#define log(...) printf(__VA_ARGS__)

typedef struct {
	uint32_t value;
	uint8_t  addr;
	uint8_t  addr_inv;
	uint8_t  crc_l;
	uint8_t  crc_h;
} tRecord;

typedef void (*tState)();

volatile uint32_t flash_vars[FLASH_VARS_COUNT];
static   uint32_t flash_vars_shadow[FLASH_VARS_COUNT];
static   uint32_t flash_vars_index;

static uint32_t page_sub_addr = 0;
static uint32_t page_index = 0;

static const uint32_t pages_addr_tbl[FLASH_VARS_PAGES_COUNT] = {
		FLASH_VARS_PAGE_0_ADDR,
		FLASH_VARS_PAGE_1_ADDR,
		FLASH_VARS_PAGE_2_ADDR
};
static const uint32_t pages_nums_tbl[FLASH_VARS_PAGES_COUNT] = {
		FLASH_VARS_PAGE_0_SEC_NUM,
		FLASH_VARS_PAGE_1_SEC_NUM,
		FLASH_VARS_PAGE_2_SEC_NUM
};

static bool init_done = false;
static bool init_error = false;

static tRecord record_buf;

static void state_none() {};
static void state_write_check();
static void state_newpage();

static tState state_func = NULL;

/*Name  : CRC-16 CCITT
  Poly  : 0x1021    x^16 + x^12 + x^5 + 1
  Init  : 0xFFFF
  Revert: false
  XorOut: 0x0000
  Check : 0x29B1 ("123456789")
  MaxLen: 4095 байт (32767 бит) - обнаружение одинарных, двойных, тройных и всех нечетных ошибок
*/
static uint16_t Crc16_CCITT(uint16_t crc, const uint8_t *pcBlock, size_t len)
{
    while (len--)
    {
        crc ^= *(pcBlock++) << 8;

        uint32_t i;
        for (i = 0; i < 8; i++)
            crc = crc & 0x8000 ? (crc << 1) ^ 0x1021 : crc << 1;
    }
    return crc;
}

static void record_build(tRecord *rec, const uint32_t value, const uint8_t addr)
{
	rec->value = value;
	rec->addr = addr;
	rec->addr_inv = ~addr;
	rec->crc_l = 0xFF;
	rec->crc_h = 0xFF;

	uint16_t crc_16bit = Crc16_CCITT(0xFFFF, (const uint8_t *)rec, sizeof(tRecord) - 2);

	rec->crc_l = (crc_16bit >> 0) & 0xFF;
	rec->crc_h = (crc_16bit >> 8) & 0xFF;
}

static bool record_check(const tRecord *rec)
{
	uint16_t real_crc = (((uint16_t)rec->crc_h) << 8) + ((uint16_t)rec->crc_l);
	uint16_t need_crc = Crc16_CCITT(0xFFFF, (const uint8_t *)rec, sizeof(tRecord) - 2);
	uint8_t addr_check = rec->addr_inv ^ 0xFF;

	bool crc_ok = (real_crc == need_crc);
	bool addr_ok = (rec->addr == addr_check);
	bool range_ok = (rec->addr < FLASH_VARS_COUNT);

	//log("0x%04X 0x%04X : %i %i %i\r\n", real_crc, need_crc, crc_ok, addr_ok, range_ok);
	return crc_ok &&
		   addr_ok &&
		   range_ok;
}

static void write_verify_callback(uint8_t *data, UNUSED uint16_t size)
{
	const tRecord *rec = (tRecord *)data;
	if (record_check(rec))
	{
		flash_vars_shadow[rec->addr] = rec->value;
		log("write_verify_callback(%i): OK\r\n", rec->addr);
	}
	else
		log("write_verify_callback(%i): ERROR\r\n", rec->addr);

	state_func = state_write_check;
	log("\r\n");
}

void state_write_check()
{
	if (flash_vars[flash_vars_index] == flash_vars_shadow[flash_vars_index])
		flash_vars_index = (flash_vars_index + 1) % FLASH_VARS_COUNT;

	if (flash_vars[flash_vars_index] == flash_vars_shadow[flash_vars_index])
		return;

	uint32_t addr = pages_addr_tbl[page_index] + page_sub_addr;
	uint8_t *data = (uint8_t *)&record_buf;
	uint16_t size = sizeof(record_buf);

	if ((page_sub_addr + size) > FLASH_VARS_PAGE_SIZE)
	{
		state_func = state_newpage;
		flash_vars_index = 0;
		page_sub_addr = 0;
		return;
	}

	record_build(&record_buf, flash_vars[flash_vars_index], flash_vars_index);

	if (flash_module_write(addr, data, size) == FLASH_SUCCESS)
		page_sub_addr += size;
	else
	{
		log("state_write_check(%2i, %04X): write busy\r\n", flash_vars_index, addr);
		return;
	}

	if (flash_module_read(addr, data, size, write_verify_callback) == FLASH_SUCCESS)
	{
		state_func = state_none;
		log("state_write_check(%2i, %04X): write and verify OK\r\n", flash_vars_index, addr);
	}
	else
		log("state_write_check(%2i, %04X): verify busy\r\n", flash_vars_index, addr);
}

static void newpage_verify_callback(uint8_t *data, UNUSED uint16_t size)
{
	const tRecord *rec = (tRecord *)data;
	if (record_check(rec))
	{
		if (flash_vars_shadow[rec->addr] == rec->value)
		{
			flash_vars_index++;
			log("newpage_verify_callback(%i): OK\r\n", rec->addr);
		}
		else
			log("newpage_verify_callback(%i): ERROR compare with flash_vars_shadow\r\n", rec->addr);
	}
	else
		log("newpage_verify_callback(%i): ERROR\r\n", rec->addr);
	state_func = state_newpage;
	log("\r\n");
}

void state_newpage()
{
	if (flash_vars_index >= FLASH_VARS_COUNT)
	{
		if (flash_module_erase(pages_nums_tbl[page_index]) == FLASH_SUCCESS)
		{
			page_index = (page_index + 1) % FLASH_VARS_PAGES_COUNT;
			flash_vars_index = 0;
			state_func = state_write_check;
			log("state_newpage(%2i, %04X): erase OK\r\n", flash_vars_index, page_sub_addr);
			log("\r\n");
			log("\r\n");
			return;
		}
		else
			log("state_newpage(%2i, %04X): erase busy\r\n", flash_vars_index, page_sub_addr);
		return;
	}

	uint8_t new_page_index = (page_index + 1) % FLASH_VARS_PAGES_COUNT;

	uint32_t addr = pages_addr_tbl[new_page_index] + page_sub_addr;
	uint8_t *data = (uint8_t *)&record_buf;
	uint16_t size = sizeof(record_buf);

	record_build(&record_buf, flash_vars[flash_vars_index], flash_vars_index);
	flash_vars_shadow[record_buf.addr] = record_buf.value;

	if (flash_module_write(addr, data, size) == FLASH_SUCCESS)
		page_sub_addr += size;
	else
	{
		log("state_newpage(%2i, %04X): write busy\r\n", flash_vars_index, addr);
		return;
	}

	if (flash_module_read(addr, data, size, newpage_verify_callback) == FLASH_SUCCESS)
	{
		state_func = state_none;
		log("state_newpage(%2i, %04X): write and verify OK\r\n", flash_vars_index, addr);
	}
	else
		log("state_newpage(%2i, %04X): verify busy\r\n", flash_vars_index, addr);
}

void flash_vars_init(bool first_start)
{
	if (first_start)
	{
		init_done = true;
		init_error = false;

		state_func = state_write_check;
		//flash_module_erase(FLASH_VARS_PAGE_0_SEC_NUM);
		//flash_module_erase(FLASH_VARS_PAGE_1_SEC_NUM);
		//flash_module_erase(FLASH_VARS_PAGE_2_SEC_NUM);
	}
}

bool flash_vars_read_init_done(void)
{
	return init_done;
}

bool flash_vars_read_init_error(void)
{
	return init_error;
}

void flash_vars_proc(void)
{
	if (state_func != NULL)
		(*state_func)();
}
