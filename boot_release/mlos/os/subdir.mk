################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../mlos/os/mlos.c \
../mlos/os/mlos_clock.c \
../mlos/os/mlos_list.c \
../mlos/os/mlos_ltimer.c \
../mlos/os/mlos_malloc.c \
../mlos/os/mlos_mpump.c \
../mlos/os/mlos_que.c \
../mlos/os/mlos_rtimer.c \
../mlos/os/mlos_task.c 

OBJS += \
./mlos/os/mlos.o \
./mlos/os/mlos_clock.o \
./mlos/os/mlos_list.o \
./mlos/os/mlos_ltimer.o \
./mlos/os/mlos_malloc.o \
./mlos/os/mlos_mpump.o \
./mlos/os/mlos_que.o \
./mlos/os/mlos_rtimer.o \
./mlos/os/mlos_task.o 

C_DEPS += \
./mlos/os/mlos.d \
./mlos/os/mlos_clock.d \
./mlos/os/mlos_list.d \
./mlos/os/mlos_ltimer.d \
./mlos/os/mlos_malloc.d \
./mlos/os/mlos_mpump.d \
./mlos/os/mlos_que.d \
./mlos/os/mlos_rtimer.d \
./mlos/os/mlos_task.d 


# Each subdirectory must supply rules for building sources it contributes
mlos/os/%.o: ../mlos/os/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -O2 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal  -g -D_RENESAS_RA_ -DCOMPILE_BOOT_CODE -I"D:\e2workspace\mc_r7fa6m4\src" -I"D:\e2workspace\mc_r7fa6m4\ra\fsp\inc" -I"D:\e2workspace\mc_r7fa6m4\ra\fsp\inc\api" -I"D:\e2workspace\mc_r7fa6m4\ra\fsp\inc\instances" -I"D:\e2workspace\mc_r7fa6m4\ra\arm\CMSIS_5\CMSIS\Core\Include" -I"D:\e2workspace\mc_r7fa6m4\ra_gen" -I"D:\e2workspace\mc_r7fa6m4\ra_cfg\fsp_cfg\bsp" -I"D:\e2workspace\mc_r7fa6m4\ra_cfg\fsp_cfg" -I"D:\e2workspace\mc_r7fa6m4\mlos\os" -I"D:\e2workspace\mc_r7fa6m4\boot" -I"D:\e2workspace\mc_r7fa6m4\mllib\iap" -I"D:\e2workspace\mc_r7fa6m4\mllib\iap\RA6M4" -I"D:\e2workspace\mc_r7fa6m4\mllib\iap\mc_ra6m4_iap_protocol" -I"D:\e2workspace\mc_r7fa6m4\mllib\mlflash\mcuflash" -I"D:\e2workspace\mc_r7fa6m4\mllib\ntp" -I"D:\e2workspace\mc_r7fa6m4\mllib\ntp\w5500" -I"D:\e2workspace\mc_r7fa6m4\mllib\ntp\w5500\ra6m4" -I"D:\e2workspace\mc_r7fa6m4\mllib\tool" -I"D:\e2workspace\mc_r7fa6m4\mllib\upper" -I"D:\e2workspace\mc_r7fa6m4\mlos\port\RA6M4" -I"D:\e2workspace\mc_r7fa6m4\mlos\services" -I"D:\e2workspace\mc_r7fa6m4\public" -I"D:\e2workspace\mc_r7fa6m4\ra_cfg\driver" -I"D:\e2workspace\mc_r7fa6m4\ra_cfg\driver" -I"D:\e2workspace\mc_r7fa6m4\ra\fsp\src\bsp\cmsis\Device\RENESAS\Include" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" -x c "$<"
	@echo 'Finished building: $<'
	@echo ' '

