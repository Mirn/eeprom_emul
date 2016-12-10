#include "stm32kiss.h"
#include "hw.h"
#include "mem.h"
#include "mem_cmd.h"

#define MEM_MAX_SIZE 64

void mem_cmd_io(const char *name, uint8_t command, uint8_t void_cnt, uint8_t *data, uint8_t data_cnt, uint8_t *after, uint8_t after_cnt)
{
	uint8_t total = 1 + void_cnt + data_cnt + after_cnt;
	uint8_t pos = 0;

	if (total > MEM_MAX_SIZE)
	{
		printf("\r\rmem_cmd_io: (total > MEM_MAX_SIZE)\r\r");
		while (1);
	}

	static uint8_t cmd[MEM_MAX_SIZE];
	uint8_t *cmd_void  = cmd + 1;
	uint8_t *cmd_data  = cmd + 1 + void_cnt;
	uint8_t *cmd_after = cmd + 1 + void_cnt + data_cnt;


	cmd[0] = command;
	for (pos=0; pos < void_cnt;  pos++) cmd_void[pos]  = 0xFF;
	for (pos=0; pos < data_cnt;  pos++) cmd_data[pos]  = (data != NULL) ? data[pos] : 0xFF;
	for (pos=0; pos < after_cnt; pos++) cmd_after[pos] = 0xFF;

	printf("CMD: %s\r", name);
	printf("WR:");
	for (pos=0; pos < total; pos++)
		printf(" %2X", cmd[pos]);
	PrintChar('\r');


	mem_start();
	for (pos=0; pos < total; pos++)
		cmd[pos] = mem_io(cmd[pos]);
	mem_stop();

	printf("RD:");
	//for (pos=0; pos < data_cnt; pos++)
	//	printf(":%2X", (data != NULL) ? data[pos] : 0xFF);

	for (pos=0; pos < total; pos++)
		printf(" %2X", cmd[pos]);
	PrintChar(' ');

	PrintChar('"');
	for (pos=0; pos < after_cnt; pos++)
	{
		char c = cmd_after[pos];
		if (c < ' ')
			PrintChar('_');
		else
			PrintChar(c);
	}
	printf("\"\r");
	PrintChar('\r');

	if (after != NULL)
		for (pos=0; pos < after_cnt; pos++)
			after[pos] = cmd_after[pos];
}

