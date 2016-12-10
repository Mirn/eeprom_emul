#ifndef __MEM_H_
#define __MEM_H_

#define MEM_TOTAL_SIZE 4194304

void mem_init();
void mem_done();

void mem_wr(uint8_t in);
uint8_t mem_io(uint8_t in);
uint8_t mem_rd();

void mem_start();
void mem_stop();

void mem_read(uint32_t addr, uint8_t *data, uint32_t size);
bool mem_write(uint32_t addr, void *data, uint32_t size);
void mem_write_raw(uint32_t addr, uint8_t *data, uint32_t size);

void mem_cmd(uint8_t cmd);
uint8_t mem_status();

bool mem_sector_erase(uint32_t addr);
void mem_sector_erase_raw(uint32_t addr);
bool mem_bulk_erase();

#endif
