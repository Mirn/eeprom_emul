#ifndef __MEM_CMD_H_
#define __MEM_CMD_H_

#define MEM_CMD_WRITE_ENABLE   0x06
#define MEM_CMD_WRITE_DISABLE  0x04
#define MEM_CMD_ID             0x9F
#define MEM_CMD_STATUS_READ    0x05
#define MEM_CMD_STATUS_WRITE   0x01
#define MEM_CMD_READ           0x03
#define MEM_CMD_FAST_READ      0x0B
#define MEM_CMD_PAGE_PROGRAM   0x02
#define MEM_CMD_SECTOR_ERASE   0xD8
#define MEM_CMD_BULK_ERASE     0xC7
#define MEM_CMD_POWER_DOWN     0xB9
#define MEM_CMD_POWER_UP       0xAB

#define MEM_STATUS_WRITE_PROGRESS 0x01
#define MEM_STATUS_WRITE_ENABLED  0x02
#define MEM_STATUS_BLOCK_BITS     0x1C
#define MEM_STATUS_WRITE_PROTECT  0x80

#define MEM_ID_LENGTH               20
#define MEM_ID_MANUFACTURER_VALUE 0x20
#define MEM_ID_MANUFACTURER_POS      0

#define MEM_SIZE_SECTOR  0x10000
#define MEM_SIZE_FLASH  0x400000

#define MEM_TIMEOUT_BULK_ERASE      80000000
#define MEM_TIMEOUT_SECTOR_ERASE     3000000
#define MEM_TIMEOUT_PAGE_PROGRAM        5000

#define mem_write_enable()  mem_cmd(MEM_CMD_WRITE_ENABLE)
#define mem_write_disable() mem_cmd(MEM_CMD_WRITE_DISABLE)

void mem_cmd_io(const char *name, uint8_t command, uint8_t void_cnt, uint8_t *data, uint8_t data_cnt, uint8_t *after, uint8_t after_cnt);

#endif
