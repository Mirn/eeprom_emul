/*
 * flash_vars.c
 *
 *  Created on: 11 дек. 2016 г.
 *      Author: Евгений
 */

#include <stdio.h>
#include <stdint.h>
#include <strings.h>

#include "flash_module.h"
#include "crc16_ccitt.h"
#include "flash_vars.h"

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wformat"    //for custom printf implementation which not support -lu and etc formats
#pragma GCC diagnostic error "-Wpadded"      //tStatRec and tRecord must be without padding!!
#define UNUSED __attribute__((unused))
#else
#error "Unknow compiler"
#endif

#define log(...) {}
//#define log(...) printf(__VA_ARGS__);

//#define log_init(...) {}
#define log_init(...) printf(__VA_ARGS__);

typedef struct {
	uint32_t value;
	uint8_t  addr;
	uint8_t  addr_inv;
	uint16_t crc;
} tRecord;

typedef struct {
	uint16_t good_cnt;
	uint16_t bad_cnt;
} tStatRec;

#define FLASH_VARS_RECORDS_IN_PAGE (FLASH_VARS_PAGES_COUNT / sizeof(tRecord))


typedef void (*tState)();

volatile uint32_t flash_vars[FLASH_VARS_COUNT] = {0};
static   uint32_t flash_vars_shadow[FLASH_VARS_COUNT] = {0};
static   uint32_t flash_vars_index = 0;

static uint32_t page_sub_addr = 0;
static uint32_t page_index = 0;

static const uint32_t pages_addr_tbl[FLASH_VARS_PAGES_COUNT] = {
		FLASH_VARS_PAGE_0_ADDR,
		FLASH_VARS_PAGE_1_ADDR,
		FLASH_VARS_PAGE_2_ADDR,
#if (FLASH_VARS_PAGES_COUNT > 3)
		FLASH_VARS_PAGE_3_ADDR,
#if (FLASH_VARS_PAGES_COUNT > 4)
		FLASH_VARS_PAGE_4_ADDR,
#endif
#endif
};
static const uint32_t pages_sectors_tbl[FLASH_VARS_PAGES_COUNT] = {
		FLASH_VARS_PAGE_0_SEC_NUM,
		FLASH_VARS_PAGE_1_SEC_NUM,
		FLASH_VARS_PAGE_2_SEC_NUM,
#if (FLASH_VARS_PAGES_COUNT > 3)
		FLASH_VARS_PAGE_3_SEC_NUM,
#if (FLASH_VARS_PAGES_COUNT > 4)
		FLASH_VARS_PAGE_4_SEC_NUM,
#endif
#endif
};

static tStatRec pages_stat[FLASH_VARS_PAGES_COUNT] = {{0}};

static bool init_done = false;
static bool init_error = false;

static bool need_renew_page = false;

static tRecord record_buf = {0};

static void state_none() {};
static void state_write_check();
static void state_newpage();
static void state_read_stat();
static void state_read_restore();

static tState state_func = NULL;

static void record_build(tRecord *rec, const uint32_t value, const uint8_t addr)
{
	rec->value = value;
	rec->addr = addr;
	rec->addr_inv = ~addr;

	rec->crc = Crc16_CCITT(0xFFFF, (const uint8_t *)rec, sizeof(tRecord) - 2);
}

static bool record_check_unprogrammed(const tRecord *rec)
{
	return  (rec->value == 0xFFFFFFFF) &&
			(rec->addr == 0xFF) &&
			(rec->addr_inv == 0xFF) &&
			(rec->crc == 0xFFFF);
}

static bool record_check(const tRecord *rec)
{
	uint16_t real_crc = rec->crc;
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
		log("write_verify_callback(%2i): OK\r\n", rec->addr);
	}
	else
		log("write_verify_callback(%2i): ERROR\r\n", rec->addr);

	state_func = state_write_check;
}

static void state_write_check()
{
	for (uint16_t num = 0; num < FLASH_VARS_COUNT; num++)
	{
		flash_vars_index = (flash_vars_index + 1) % FLASH_VARS_COUNT;

		uint32_t index  = flash_vars_index;
		uint32_t value  = flash_vars[flash_vars_index];
		uint32_t shadow = flash_vars_shadow[flash_vars_index];

		if (value == shadow)
			continue;

		uint32_t addr = pages_addr_tbl[page_index] + page_sub_addr;
		uint8_t *data = (uint8_t *)&record_buf;
		uint16_t size = sizeof(record_buf);

		if ((page_sub_addr + size) > FLASH_VARS_PAGE_SIZE)
		{
			uint32_t new_page = (page_index + 1) % FLASH_VARS_PAGES_COUNT;
			if (flash_module_erase(pages_sectors_tbl[new_page]) == FLASH_SUCCESS)
			{
				state_func = state_newpage;
				flash_vars_index = 0;
				page_sub_addr = 0;
				log("state_write_check: erase page #%i OK\r\n", new_page);
			}
			else
				log("state_write_check: erase page #%i busy\r\n", new_page);
			return;
		}

		record_build(&record_buf, value, index);

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
			log("state_write_check(%2i, %04X): write and verify OK    ", flash_vars_index, addr);
		}
		else
			log("state_write_check(%2i, %04X): verify busy\r\n", flash_vars_index, addr);

		return;
	}
}

static void newpage_verify_callback(uint8_t *data, UNUSED uint16_t size)
{
	const tRecord *rec = (tRecord *)data;
	if (record_check(rec))
	{
		if (flash_vars_shadow[rec->addr] == rec->value)
		{
			flash_vars_index++;
			log("newpage_verify_callback(%2i): OK\r\n", rec->addr);
		}
		else
			log("newpage_verify_callback(%2i): ERROR compare with flash_vars_shadow\r\n", rec->addr);
	}
	else
		log("newpage_verify_callback(%i): ERROR\r\n", rec->addr);
	state_func = state_newpage;
}

static void state_newpage()
{
	if (flash_vars_index >= FLASH_VARS_COUNT)
	{
		if (flash_module_erase(pages_sectors_tbl[page_index]) == FLASH_SUCCESS)
		{
			log("state_newpage(page = %2i, addr=%04X): erase OK\r\n", page_index, page_sub_addr);
			page_index = (page_index + 1) % FLASH_VARS_PAGES_COUNT;
			flash_vars_index = 0;
			state_func = state_write_check;
			log("\r\n");
		}
		else
			log("state_newpage(page = %2i, addr=%04X): erase busy\r\n", page_index, page_sub_addr);
		return;
	}

	uint8_t new_page_index = (page_index + 1) % FLASH_VARS_PAGES_COUNT;

	uint32_t addr = pages_addr_tbl[new_page_index] + page_sub_addr;
	uint8_t *data = (uint8_t *)&record_buf;
	uint16_t size = sizeof(record_buf);

	if ((page_sub_addr + size) > FLASH_VARS_PAGE_SIZE)
	{
		if (flash_module_erase(pages_sectors_tbl[page_index]) == FLASH_SUCCESS)
		{
			log("state_newpage(%2i): erase OK\r\n", page_index);
			page_index = (page_index + 1) % FLASH_VARS_PAGES_COUNT;
			flash_vars_index = 0;
			page_sub_addr = 0;
			log("\r\n");
		}
		else
			log("state_newpage(%2i): erase busy\r\n", page_index);
		return;
	}

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
		log("state_newpage(%2i, %04X): write and verify OK    ", flash_vars_index, addr);
	}
	else
		log("state_newpage(%2i, %04X): verify busy\r\n", flash_vars_index, addr);
}

static void init_process_error()
{
	init_done = true;
	init_error = true;
	memset((void *)flash_vars, 0, sizeof(flash_vars));
	memset(flash_vars_shadow, 0, sizeof(flash_vars_shadow));

	page_index = 0;
	page_sub_addr = 0;
	flash_vars_index = 0;
	state_func = state_write_check;

	for (uint32_t index = 0; index < FLASH_VARS_PAGES_COUNT; index++)
		flash_module_erase(pages_sectors_tbl[index]);
}

static void init_restore_from(uint8_t page_num)
{
	page_index = page_num;
	page_sub_addr = 0;
	flash_vars_index = 0;
	need_renew_page = false;
	memset((void *)flash_vars, 0, sizeof(flash_vars));
	memset(flash_vars_shadow, 0xFF, sizeof(flash_vars_shadow));
	state_func = state_read_restore;
}

static void stat_process()
{
	const uint8_t NON_FOUND = 0xFF;

	uint8_t pages_full_good = 0;
	uint8_t pages_part_good = 0;
	uint8_t pages_clean = 0;
	uint8_t pages_errors = 0;

	uint8_t part_good_pos = NON_FOUND;
	uint8_t last_full_good = NON_FOUND;
	uint8_t first_full_good = NON_FOUND;

	log_init("\t\tClear\tGood\tBad\r\n");
	for (uint32_t index = 0; index < FLASH_VARS_PAGES_COUNT; index++)
	{
		uint32_t bad = pages_stat[index].bad_cnt;
		uint32_t good = pages_stat[index].good_cnt;
		uint32_t clean = (FLASH_VARS_PAGE_SIZE / sizeof(record_buf)) - good - bad;

		if (bad > 0)
			pages_errors++;

		if ((bad == 0) && (good == 0))
			pages_clean++;

		if ((bad == 0) && (good > 0) && (clean == 0))
		{
			if (first_full_good == NON_FOUND)
				first_full_good = index;
			else
				last_full_good = index;
			pages_full_good++;
		}

		if ((bad == 0) && (good > 0) && (clean > 0))
		{
			part_good_pos = index;
			pages_part_good++;
		}

		log_init("Stat page #%i:\t%i\t%i\t%i\r\n", index, clean, good, bad);
	}
	log_init("\r\n");

	log_init("pages_full_good\t%i\r\n", pages_full_good);
	log_init("pages_part_good\t%i\r\n", pages_part_good);
	log_init("pages_clean    \t%i\r\n", pages_clean);
	log_init("pages_errors   \t%i\r\n", pages_errors);
	log_init("\r\n");
	log_init("part_good_pos  \t%i\r\n", part_good_pos);
	log_init("last_full_good \t%i\r\n", last_full_good);
	log_init("first_full_good\t%i\r\n", first_full_good);
	log_init("\r\n");

	if (
		((pages_full_good == 0) && (pages_part_good == 0)) ||
		((pages_full_good >  1) && (pages_part_good == 1)) ||
		(pages_full_good == FLASH_VARS_PAGES_COUNT)	||
		(pages_part_good > 1) ||
		(pages_full_good > 2) ||
		((pages_full_good == 2) &&
				(
						((first_full_good > 0) && (last_full_good != (first_full_good + 1))) ||
						((first_full_good == 0) && (last_full_good != 1) && (last_full_good != (FLASH_VARS_PAGES_COUNT - 1)))
				))
	   )
	{
		init_process_error();
		log_init("Flash_var_init: ERROR\r\n");
		return;
	}

	if (pages_part_good == 1)
	{
		if (pages_stat[part_good_pos].good_cnt < FLASH_VARS_COUNT)
		{
			if (pages_full_good == 0)
			{
				init_process_error();
				log_init("Flash_var_init: ERROR: partitaly page not complite\r\n");
			}
			else
			{
				init_restore_from(first_full_good);
				log_init("Flash_var_init_A: done and ok, restore variables from full page #%i\r\n", first_full_good);
			}
		}
		else
		{
			init_restore_from(part_good_pos);
			log_init("Flash_var_init_A: done and ok, restore variables from part_page #%i\r\n", part_good_pos);
		}
		return;
	}

	if (pages_full_good == 1)
	{
		init_restore_from(first_full_good);
		log_init("Flash_var_init_B: done and ok, restore variables from single full page #%i\r\n", first_full_good);
		return;
	}

	if (pages_full_good > 1)
	{
		if (last_full_good == (FLASH_VARS_PAGES_COUNT - 1))
		{
			init_restore_from(first_full_good);
			log_init("Flash_var_init_C: done and ok, restore variables from multi full page #%i\r\n", first_full_good);
		}
		else
		{
			init_restore_from(last_full_good);
			log_init("Flash_var_init_C: done and ok, restore variables from multi full page #%i\r\n", last_full_good);
		}
		return;
	}
	return;
}


static void read_stat_callback(uint8_t *data, UNUSED uint16_t size)
{
	const tRecord *rec = (tRecord *)data;

	if (!record_check_unprogrammed(rec))
	{
		if (record_check(rec))
		{
			pages_stat[page_index].good_cnt++;
			log("read_stat_callback(%2i, %04X): OK\r\n", page_index, page_sub_addr);
		}
		else
		{
			pages_stat[page_index].bad_cnt++;
			log("read_stat_callback(%2i, %04X): ERROR\r\n", page_index, page_sub_addr);
		}
	}
	else
		log("read_stat_callback(%2i, %04X): VOID\r\n", page_index, page_sub_addr);

	page_sub_addr += sizeof(record_buf);
	if ((page_sub_addr + sizeof(record_buf)) > FLASH_VARS_PAGE_SIZE)
	{
		page_sub_addr = 0;
		page_index++;
	}

	state_func = state_read_stat;
}

static void state_read_stat()
{
	if (page_index >= FLASH_VARS_PAGES_COUNT)
	{
		stat_process();
		return;
	}

	uint32_t addr = pages_addr_tbl[page_index] + page_sub_addr;
	uint8_t *data = (uint8_t *)&record_buf;
	uint16_t size = sizeof(record_buf);

	if (flash_module_read(addr, data, size, read_stat_callback) == FLASH_SUCCESS)
	{
		state_func = state_none;
		log("state_read_stat(%2i, %04X): read OK    ", page_index, addr);
	}
	else
		log("state_read_stat(%2i, %04X): read busy\r\n", page_index, addr);
}

static void read_restore_callback(uint8_t *data, UNUSED uint16_t size)
{
	state_func = state_read_restore;

	const tRecord *rec = (tRecord *)data;
	if (record_check_unprogrammed(rec))
	{
		log("read_restore_callback: VOID\r\n");
		init_done = true;
		return;
	}

	page_sub_addr += size;
	if (page_sub_addr >= FLASH_VARS_PAGE_SIZE)
	{
		init_done = true;
		need_renew_page = true;
	}

	if (record_check(rec))
	{
		flash_vars[rec->addr] = rec->value;
		flash_vars_shadow[rec->addr] = rec->value;
		log("read_restore_callback(%2i): OK\r\n", rec->addr);
	}
	else
		log("read_restore_callback(%2i): ERROR\r\n", rec->addr);
}

static void state_read_restore()
{
	if (init_done)
	{
		for (uint32_t index = 0; index < FLASH_VARS_COUNT; index++)
			if (flash_vars[index] != flash_vars_shadow[index])
			{
				init_error = true;
				break;
			}

		if (init_error)
			log_init("Flash_vars_init_DONE ERROR\r\n")
		else
			log_init("Flash_vars_init_DONE all OK\r\n");
		log("\r\n");

		if (need_renew_page)
		{
			uint32_t new_page = (page_index + 1) % FLASH_VARS_PAGES_COUNT;
			if (flash_module_erase(pages_sectors_tbl[new_page]) == FLASH_SUCCESS)
			{
				log("state_read_restore: after done erase page #%i OK\r\n", new_page);
				need_renew_page = false;
				flash_vars_index = 0;
				page_sub_addr = 0;
				state_func = state_newpage;
			}
			else
				log("state_read_restore: after done erase page #%i Busy\r\n", new_page);
			return;
		}
		else
			state_func = state_write_check;
		return;
	}

	uint32_t addr = pages_addr_tbl[page_index] + page_sub_addr;
	uint8_t *data = (uint8_t *)&record_buf;
	uint16_t size = sizeof(record_buf);

	if (flash_module_read(addr, data, size, read_restore_callback) == FLASH_SUCCESS)
	{
		state_func = state_none;
		log("state_read_restore(%2i, %04X): read OK    ", page_index, addr);
	}
	else
		log("state_read_restore(%2i, %04X): verify busy\r\n", page_index, addr);
}

void flash_vars_init(bool first_start)
{
	memset((void *)flash_vars, 0x00, sizeof(flash_vars));
	memset(flash_vars_shadow, 0xFF, sizeof(flash_vars_shadow));
	memset(pages_stat, 0, sizeof(pages_stat));
	flash_vars_index = 0;

	page_sub_addr = 0;
	page_index = 0;

	if (first_start)
	{
		init_done = true;
		init_error = false;

		state_func = state_write_check;

		for (uint32_t index = 0; index < FLASH_VARS_PAGES_COUNT; index++)
			flash_module_erase(pages_sectors_tbl[index]);
	}
	else
	{
		init_done = false;
		init_error = false;
		state_func = state_read_stat;
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
