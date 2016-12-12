################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/crc16_ccitt.c \
../src/flash_module.c \
../src/flash_vars.c \
../src/hw.c \
../src/main.c \
../src/mem.c \
../src/mem_cmd.c \
../src/mem_tst.c 

OBJS += \
./src/crc16_ccitt.o \
./src/flash_module.o \
./src/flash_vars.o \
./src/hw.o \
./src/main.o \
./src/mem.o \
./src/mem_cmd.o \
./src/mem_tst.o 

C_DEPS += \
./src/crc16_ccitt.d \
./src/flash_module.d \
./src/flash_vars.d \
./src/hw.d \
./src/main.d \
./src/mem.d \
./src/mem_cmd.d \
./src/mem_tst.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -O2 -fmessage-length=0 -ffunction-sections -fdata-sections -ffreestanding -Wunused -Wuninitialized -Wall -Wextra -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000 -DSTM32F10X_LD_VL -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f1-stdperiph" -I../src -I../system/include/stm32kiss -std=gnu11 -Wbad-function-cast -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


