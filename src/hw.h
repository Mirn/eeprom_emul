#ifndef __HW_H_
#define __HW_H_

#define PIN_LCD_CLK   PIN_A3
#define PIN_LCD_DAT   PIN_A4

#ifdef PCB_V3
#define PIN_LCD_nCS   PIN_A0
#define PIN_LCD_nRST  PIN_A1
#define PIN_LCD_nDC   PIN_A2
#else
#ifdef PCB_V2
#define PIN_LCD_nDC   PIN_A1
#define PIN_LCD_nRST  PIN_A0
#define PIN_LCD_nCS   PIN_A2
#else
#define PIN_LCD_nDC   PIN_A2
#define PIN_LCD_nRST  PIN_A1
#define PIN_LCD_nCS   PIN_A0
#endif
#endif


#define LINES_MISS 0


typedef volatile uint32_t uint32_t_bb;

#define PIN_MEM_CS    PIN_A5
#define PIN_MEM_OUT   PIN_A6
#define PIN_MEM_OUT_BB (*(uint32_t_bb *)(PERIPH_BB_BASE + ((((uint32_t)&GPIOA->IDR) - PERIPH_BASE) * 0x20) + 6 * 4))
#define PIN_MEM_WP    PIN_A7
#define PIN_MEM_CLK   PIN_B11
#define PIN_MEM_IN    PIN_B10

#define _PIN_POWER_ON PIN_B0

#define PIN_KEY_0 PIN_B1
#define PIN_KEY_1 PIN_B12
#define PIN_KEY_2 PIN_A14
#define PIN_KEY_3 PIN_B7

#define PIN_CHARGE_STATUS PIN_B9

void SetSysClockTo8(void);
void SetSysClockTo24(void);

#define POWER_ON()  pin_output_v(_PIN_POWER_ON, 1)
#define POWER_OFF() pin_output_v(_PIN_POWER_ON, 0)

void printf_usart_reinit();
unsigned int root1(unsigned int a);

typedef enum
{
	Charge_NC = 0,
	Charge_Process = 1,
	Charge_Complete = 2,
	Charge_Error = 3

} tCharge_Status;

tCharge_Status charge_status_read();
const char *charge_status_str(tCharge_Status status);

#if (defined(USE_STDPERIPH_DRIVER) | defined(STM32F10X_MD_VL))
#define WINAPI

#else
#include <windows.h>

typedef struct
{
	volatile bool value;
	volatile bool invert;
} __attribute__((packed)) KS_PIN;

extern KS_PIN __PIN_B0 ;
extern KS_PIN __PIN_B1 ;
extern KS_PIN __PIN_B2 ;
extern KS_PIN __PIN_B3 ;
extern KS_PIN __PIN_B4 ;
extern KS_PIN __PIN_B5 ;
extern KS_PIN __PIN_B6 ;
extern KS_PIN __PIN_B7 ;
extern KS_PIN __PIN_B8 ;
extern KS_PIN __PIN_B9 ;
extern KS_PIN __PIN_B10;
extern KS_PIN __PIN_B11;
extern KS_PIN __PIN_B12;
extern KS_PIN __PIN_B13;
extern KS_PIN __PIN_B14;
extern KS_PIN __PIN_B15;

extern KS_PIN __PIN_A0 ;
extern KS_PIN __PIN_A1 ;
extern KS_PIN __PIN_A2 ;
extern KS_PIN __PIN_A3 ;
extern KS_PIN __PIN_A4 ;
extern KS_PIN __PIN_A5 ;
extern KS_PIN __PIN_A6 ;
extern KS_PIN __PIN_A7 ;
extern KS_PIN __PIN_A8 ;
extern KS_PIN __PIN_A9 ;
extern KS_PIN __PIN_A10;
extern KS_PIN __PIN_A11;
extern KS_PIN __PIN_A12;
extern KS_PIN __PIN_A13;
extern KS_PIN __PIN_A14;
extern KS_PIN __PIN_A15;

#define PIN_B0   (&__PIN_B0 )
#define PIN_B1   (&__PIN_B1 )
#define PIN_B2   (&__PIN_B2 )
#define PIN_B3   (&__PIN_B3 )
#define PIN_B4   (&__PIN_B4 )
#define PIN_B5   (&__PIN_B5 )
#define PIN_B6   (&__PIN_B6 )
#define PIN_B7   (&__PIN_B7 )
#define PIN_B8   (&__PIN_B8 )
#define PIN_B9   (&__PIN_B9 )
#define PIN_B10  (&__PIN_B10)
#define PIN_B11  (&__PIN_B11)
#define PIN_B12  (&__PIN_B12)
#define PIN_B13  (&__PIN_B13)
#define PIN_B14  (&__PIN_B14)
#define PIN_B15  (&__PIN_B15)

#define PIN_A0   (&__PIN_A0 )
#define PIN_A1   (&__PIN_A1 )
#define PIN_A2   (&__PIN_A2 )
#define PIN_A3   (&__PIN_A3 )
#define PIN_A4   (&__PIN_A4 )
#define PIN_A5   (&__PIN_A5 )
#define PIN_A6   (&__PIN_A6 )
#define PIN_A7   (&__PIN_A7 )
#define PIN_A8   (&__PIN_A8 )
#define PIN_A9   (&__PIN_A9 )
#define PIN_A10  (&__PIN_A10)
#define PIN_A11  (&__PIN_A11)
#define PIN_A12  (&__PIN_A12)
#define PIN_A13  (&__PIN_A13)
#define PIN_A14  (&__PIN_A14)
#define PIN_A15  (&__PIN_A15)



void pin_input_down(KS_PIN *pin);
void pin_input_up(KS_PIN *pin);
void pin_input(KS_PIN *pin);
static inline void pin_output(KS_PIN *pin) {};
static inline bool pin_output_v(KS_PIN *pin, bool new_value) {return new_value;};
static inline bool pin_write(const KS_PIN *pin, bool new_value) {return new_value;};
static inline bool pin_write_0(const KS_PIN *pin) {return false;};
static inline bool pin_write_1(const KS_PIN *pin) {return true;};
bool pin_read(const KS_PIN *pin);


#define USART1 NULL
#define USART_DeInit(v)

static inline void adc_on() {};
static inline uint16_t adc_read() { return 23000;};
static inline void adc_off() {};

void systick_on(uint16_t freq, void (*func)());

void __WFI();
static inline void delay_ms(uint32_t ms) {Sleep(ms);};

static inline void mem_init() {};
void mem_read(uint32_t addr, uint8_t *data, uint32_t size);

static inline void uploader_run() {};
static inline void test_all() {};
static inline void PrintChar(char c) {putchar(c);};

#endif
#endif
