/**
 * @defgroup flash_module Flash module
 * @{
 * @ingroup flash_module
 *
 * @brief API for flash module.
 *        API functions shouldn't be called from interrupts.
 *        There are two mandatory functions:
 *        1. flash_module_init()  - initialization, should be called before main application loop.
 *        2. flash_module_proc()  - main procedure, should be called in main application loop.
 *        Module requirements:
 *        Soft timers module
 */

#ifndef FLASH_MODULE_H
#define FLASH_MODULE_H

#include <stdint.h>
#include <string.h>


/*!
  @brief    Module callback for application layer,
            used to indicate about finishing of reading operation.
  @param    data - pointer to get read data.
  @param    size - length of received data.
  @retval   None.
*/
typedef void (*flash_module_handler_t)(uint8_t *data, uint16_t size);


/**
 * @brief enumeration for statuses in module.
 */
typedef enum
{
    FLASH_SUCCESS = 0,
    FLASH_ERR_INV_PARAMS,
    FLASH_ERR_BUSY,
    FLASH_ERR_OP_FAILED
}flash_module_err_t;



/*!
  @brief    Initialization if flash module.
  @param    None.
  @retval   FLASH_ERR_OP_FAILED - Initialization error.
  @retval   FLASH_SUCCESS - Successfull initialization.
  @note     Must be called after Soft timers module initialization.
*/
int flash_module_init(void);




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
int flash_module_read(uint32_t addr, uint8_t * data, uint16_t size, flash_module_handler_t cb);




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
int flash_module_write(uint32_t addr, uint8_t *data, uint16_t size);


/*!
  @brief    Erase sector in flash module. This function puts erase command in internal commands queue.
  @param    sector - number of sector to erase. For current flash module number of sectors is 8.
            Sectors enumerated from 0 to 7. Sector size is 0x10000 (65536) bytes.
  @retval   FLASH_SUCCESS - Inavlid input parameters.
  @retval   FLASH_ERR_INV_PARAMS - Inavlid input parameters.
  @retval   FLASH_ERR_BUSY - Flash module's commands queue is full.
  @note     This driver requires Soft timers module.
*/
int flash_module_erase(uint16_t sector);


/*!
  @brief    Shutdown flash module.
  @retval   None
  @note     Currently is not implemented.
*/
void flash_module_shutdown(void);

/*!
  @brief    Flash module procedure.
  @retval   None
  @note     This driver requires Soft timers module,
            This procedure must be called in the application main loop.
*/
void flash_module_proc(void);

#endif
