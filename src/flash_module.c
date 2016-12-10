/*
 * flash_module.c
 *
 *  Created on: 10 дек. 2016 г.
 *      Author: Евгений
 */

#include "stm32kiss.h"
#include "hw.h"
#include "mem.h"
#include "mem_cmd.h"
#include "flash_module.h"

#define FLASH_MEM_TOTAL_SIZE 0x400000
#define FLASH_MEM_SECTORS_COUNT 8
#define FLASH_MEM_SECTOR_SIZE 0x10000
#define FLASH_MEM_READ_MAX 1024
#define FLASH_MEM_WRITE_MAX 1024
#define QUEUE_LENGTH 32

//#define log(...)
#define log(...) printf(__VA_ARGS__)

typedef struct Flash_operation {
	uint8_t *data;
	uint32_t addr;
	uint32_t size;
	flash_module_handler_t callback;
	bool (*worker)(struct Flash_operation *op);
} tFlash_operation;

static tFlash_operation task;
static bool task_done;
static uint32_t task_time_start = 0;

static tFlash_operation task_queue[QUEUE_LENGTH];
static uint32_t task_queue_rd = 0;
static uint32_t task_pos_wr = 0;

static int32_t task_queue_count()
{
	return task_pos_wr - task_queue_rd;
}

static int32_t task_queue_free()
{
	return QUEUE_LENGTH - task_queue_count();
}

static void task_queue_init()
{
	task_queue_rd = 0;
	task_pos_wr = 0;
}

static bool task_queue_add(bool (*worker)(tFlash_operation *op), uint32_t addr, uint8_t * data, uint32_t size, flash_module_handler_t cb)
{
	if (task_queue_count() >= QUEUE_LENGTH)
		return false;

	tFlash_operation op = {0};
	op.addr = addr;
	op.data = data;
	op.size = size;
	op.callback = cb;
	op.worker = worker;

	uint32_t index = task_pos_wr % QUEUE_LENGTH;
	task_queue[index] = op;

	task_pos_wr++;
	return true;
}

static bool task_queue_extract(tFlash_operation *op)
{
	if (task_queue_count() >= QUEUE_LENGTH)
		return false;

	uint32_t index = task_queue_rd % QUEUE_LENGTH;
	*op = task_queue[index];
	task_queue_rd++;
	return true;
}

//////////////////////////////////////////////////////////////////

/*!
  @brief    Initialization if flash module.
  @param    None.
  @retval   FLASH_ERR_OP_FAILED - Initialization error.
  @retval   FLASH_SUCCESS - Successfull initialization.
  @note     Must be called after Soft timers module initialization.
*/
int flash_module_init(void)
{
	log("flash_module_init\r\n");

	task_done = true;
	task_queue_init();

	mem_init();
	delay_ms(10);

	uint8_t id[MEM_ID_LENGTH] = {0};

	mem_start();
	mem_wr(MEM_CMD_ID);
	for (uint32_t pos = 0; pos < LENGTH(id); pos++)
		id[pos] = mem_io(0xFF);
	mem_stop();

	log("MEM_CMD_ID:");
	for (uint32_t pos = 0; pos < LENGTH(id); pos++)
		log(" %02X", id[pos]);
	log("\r\n");

	if (id[MEM_ID_MANUFACTURER_POS] == MEM_ID_MANUFACTURER_VALUE)
	{
		log("flash_module_init: FLASH_SUCCESS\r\n");
		return FLASH_SUCCESS;
	}
	else
	{
		log("flash_module_init: FLASH_ERR_OP_FAILED\r\n");
		return FLASH_ERR_OP_FAILED;
	}
}

static bool worker_read(tFlash_operation *op)
{
	mem_read(op->addr, op->data, op->size);

	if (op->callback != NULL)
		(*op->callback)(op->data, op->size);
	return true;
}

static bool worker_write(tFlash_operation *op)
{
	mem_write_enable();
	mem_write_raw(op->addr, op->data, op->size);
	return true;
}

static bool worker_erase(tFlash_operation *op)
{
	mem_write_enable();
	mem_sector_erase_raw(op->addr);
	return true;
}

static bool worker_wait_prog(UNUSED tFlash_operation *op)
{
	bool done = (mem_status() & MEM_STATUS_WRITE_PROGRESS) == 0;

	if (!done)
	{
		uint32_t timeout_us = op->size;
		uint32_t wait_us = (((uint64_t)(DWT_CYCCNT - task_time_start)) * ((uint64_t)1000000LL)) / SystemCoreClock;

		if (timeout_us > 0)
		{
			if (wait_us > timeout_us)
			{
				log("Flash_module: worker_wait_prog timeout error\r\n");
				done = true;
			}
		}
	}

	if (done)
		mem_write_disable();
	return done;
}

/*!
  @brief    Read data from flash module. This function puts read command in internal commands queue.
  @param    addr - start memory address to read.
  @param    size - length of data to read, should be less or equal to
            1024 bytes.
  @param    data - pointer to applicataion buffer used for reading bytes to.
  @param    cb - callback to indicate read operation completion.
  @retval   FLASH_SUCCESS - Success.
  @retval   FLASH_ERR_INV_PARAMS - Inavlid input parameters.
  @retval   FLASH_ERR_BUSY - Flash module's commands queue is full.
  @note     This driver requires Soft timers module.
*/
int flash_module_read(uint32_t addr, uint8_t * data, uint16_t size, flash_module_handler_t cb)
{
	if (addr >= FLASH_MEM_TOTAL_SIZE)
		return FLASH_ERR_INV_PARAMS;

	if (size > FLASH_MEM_READ_MAX)
		return FLASH_ERR_INV_PARAMS;

	if ((addr + size) > FLASH_MEM_TOTAL_SIZE)
		return FLASH_ERR_INV_PARAMS;

	if (size == 0)
		return FLASH_ERR_INV_PARAMS;

	if (task_queue_free() < 1)
		return FLASH_ERR_BUSY;

	task_queue_add(worker_read, addr, data, size, cb);
	return FLASH_SUCCESS;
}


/*!
  @brief    Erase sector in flash module. This function puts erase command in internal commands queue.
  @param    sector - number of sector to erase. For current flash module number of sectors is 8.
            Sectors enumerated from 0 to 7. Sector size is 0x10000 (65536) bytes.
  @retval   FLASH_SUCCESS - Inavlid input parameters.
  @retval   FLASH_ERR_INV_PARAMS - Inavlid input parameters.
  @retval   FLASH_ERR_BUSY - Flash module's commands queue is full.
  @note     This driver requires Soft timers module.
*/
int flash_module_erase(uint16_t sector)
{
	if (sector >= FLASH_MEM_SECTORS_COUNT)
		return FLASH_ERR_INV_PARAMS;

	if (task_queue_free() < 2)
		return FLASH_ERR_BUSY;

	task_queue_add(worker_erase, sector * FLASH_MEM_SECTOR_SIZE, 0, 0, 0);
	task_queue_add(worker_wait_prog, 0, 0, MEM_TIMEOUT_SECTOR_ERASE, 0);
	return FLASH_SUCCESS;
}

/*!
  @brief    Write data to flash module. This function puts write command in internal commands queue.
  @param    addr - start memory address to write.
  @param    size - length of data to write, should be less or equal to
            1024 bytes.
  @param    data - pointer to applicataion buffer used for writing bytes from.
  @retval   FLASH_SUCCESS - Success.
  @retval   FLASH_ERR_INV_PARAMS - Inavlid input parameters.
  @retval   FLASH_ERR_BUSY - Flash module's commands queue is full.
  @note     This driver requires Soft timers module.
*/
int flash_module_write(uint32_t addr, uint8_t *data, uint16_t size)
{
	if (addr >= FLASH_MEM_TOTAL_SIZE)
		return FLASH_ERR_INV_PARAMS;

	if (size > FLASH_MEM_WRITE_MAX)
		return FLASH_ERR_INV_PARAMS;

	if ((addr + size) > FLASH_MEM_TOTAL_SIZE)
		return FLASH_ERR_INV_PARAMS;

	if (size == 0)
		return FLASH_ERR_INV_PARAMS;

	if (task_queue_free() < 2)
		return FLASH_ERR_BUSY;

	task_queue_add(worker_write, addr, data, size, 0);
	task_queue_add(worker_wait_prog, 0, 0, MEM_TIMEOUT_PAGE_PROGRAM, 0);
	return FLASH_SUCCESS;
}

/*!
  @brief    Shutdown flash module.
  @retval   None
  @note     Currently is not implemented.
*/
void flash_module_shutdown(void)
{
	//Currently is not implemented.
}

/*!
  @brief    Flash module procedure.
  @retval   None
  @note     This driver requires Soft timers module,
            This procedure must be called in the application main loop.
*/
void flash_module_proc(void)
{
	if (task_done)
	{
		if (task_queue_count() <= 0)
			return;
		task_queue_extract(&task);
		task_time_start = DWT_CYCCNT;
	}

	if (task.worker != NULL)
		task_done = (*task.worker)(&task);
	else
		task_done = true;
}
