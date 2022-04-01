################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ra/fsp/src/bsp/mcu/all/bsp_clocks.c \
../ra/fsp/src/bsp/mcu/all/bsp_common.c \
../ra/fsp/src/bsp/mcu/all/bsp_delay.c \
../ra/fsp/src/bsp/mcu/all/bsp_group_irq.c \
../ra/fsp/src/bsp/mcu/all/bsp_guard.c \
../ra/fsp/src/bsp/mcu/all/bsp_io.c \
../ra/fsp/src/bsp/mcu/all/bsp_irq.c \
../ra/fsp/src/bsp/mcu/all/bsp_register_protection.c \
../ra/fsp/src/bsp/mcu/all/bsp_rom_registers.c \
../ra/fsp/src/bsp/mcu/all/bsp_sbrk.c \
../ra/fsp/src/bsp/mcu/all/bsp_security.c 

OBJS += \
./ra/fsp/src/bsp/mcu/all/bsp_clocks.o \
./ra/fsp/src/bsp/mcu/all/bsp_common.o \
./ra/fsp/src/bsp/mcu/all/bsp_delay.o \
./ra/fsp/src/bsp/mcu/all/bsp_group_irq.o \
./ra/fsp/src/bsp/mcu/all/bsp_guard.o \
./ra/fsp/src/bsp/mcu/all/bsp_io.o \
./ra/fsp/src/bsp/mcu/all/bsp_irq.o \
./ra/fsp/src/bsp/mcu/all/bsp_register_protection.o \
./ra/fsp/src/bsp/mcu/all/bsp_rom_registers.o \
./ra/fsp/src/bsp/mcu/all/bsp_sbrk.o \
./ra/fsp/src/bsp/mcu/all/bsp_security.o 

C_DEPS += \
./ra/fsp/src/bsp/mcu/all/bsp_clocks.d \
./ra/fsp/src/bsp/mcu/all/bsp_common.d \
./ra/fsp/src/bsp/mcu/all/bsp_delay.d \
./ra/fsp/src/bsp/mcu/all/bsp_group_irq.d \
./ra/fsp/src/bsp/mcu/all/bsp_guard.d \
./ra/fsp/src/bsp/mcu/all/bsp_io.d \
./ra/fsp/src/bsp/mcu/all/bsp_irq.d \
./ra/fsp/src/bsp/mcu/all/bsp_register_protection.d \
./ra/fsp/src/bsp/mcu/all/bsp_rom_registers.d \
./ra/fsp/src/bsp/mcu/all/bsp_sbrk.d \
./ra/fsp/src/bsp/mcu/all/bsp_security.d 


# Each subdirectory must supply rules for building sources it contributes
ra/fsp/src/bsp/mcu/all/%.o: ../ra/fsp/src/bsp/mcu/all/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -O2 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal  -g -D_RENESAS_RA_ -DCOMPILE_BOOT_CODE -I"D:\e2workspace\mc_r7fa6m4\src" -I"D:\e2workspace\mc_r7fa6m4\ra\fsp\inc" -I"D:\e2workspace\mc_r7fa6m4\ra\fsp\inc\api" -I"D:\e2workspace\mc_r7fa6m4\ra\fsp\inc\instances" -I"D:\e2workspace\mc_r7fa6m4\ra\arm\CMSIS_5\CMSIS\Core\Include" -I"D:\e2workspace\mc_r7fa6m4\ra_gen" -I"D:\e2workspace\mc_r7fa6m4\ra_cfg\fsp_cfg\bsp" -I"D:\e2workspace\mc_r7fa6m4\ra_cfg\fsp_cfg" -I"D:\e2workspace\mc_r7fa6m4\mlos\os" -I"D:\e2workspace\mc_r7fa6m4\boot" -I"D:\e2workspace\mc_r7fa6m4\mllib\iap" -I"D:\e2workspace\mc_r7fa6m4\mllib\iap\RA6M4" -I"D:\e2workspace\mc_r7fa6m4\mllib\iap\mc_ra6m4_iap_protocol" -I"D:\e2workspace\mc_r7fa6m4\mllib\mlflash\mcuflash" -I"D:\e2workspace\mc_r7fa6m4\mllib\ntp" -I"D:\e2workspace\mc_r7fa6m4\mllib\ntp\w5500" -I"D:\e2workspace\mc_r7fa6m4\mllib\ntp\w5500\ra6m4" -I"D:\e2workspace\mc_r7fa6m4\mllib\tool" -I"D:\e2workspace\mc_r7fa6m4\mllib\upper" -I"D:\e2workspace\mc_r7fa6m4\mlos\port\RA6M4" -I"D:\e2workspace\mc_r7fa6m4\mlos\services" -I"D:\e2workspace\mc_r7fa6m4\public" -I"D:\e2workspace\mc_r7fa6m4\ra_cfg\driver" -I"D:\e2workspace\mc_r7fa6m4\ra_cfg\driver" -I"D:\e2workspace\mc_r7fa6m4\ra\fsp\src\bsp\cmsis\Device\RENESAS\Include" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" -x c "$<"
	@echo 'Finished building: $<'
	@echo ' '


