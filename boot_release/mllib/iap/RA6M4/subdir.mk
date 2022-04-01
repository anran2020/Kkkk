################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../mllib/iap/RA6M4/iap_mcu_port.c 

OBJS += \
./mllib/iap/RA6M4/iap_mcu_port.o 

C_DEPS += \
./mllib/iap/RA6M4/iap_mcu_port.d 


# Each subdirectory must supply rules for building sources it contributes
mllib/iap/RA6M4/%.o: ../mllib/iap/RA6M4/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -O2 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal  -g -D_RENESAS_RA_ -DCOMPILE_BOOT_CODE -I"D:\e2workspace\mc_r7fa6m4\src" -I"D:\e2workspace\mc_r7fa6m4\ra\fsp\inc" -I"D:\e2workspace\mc_r7fa6m4\ra\fsp\inc\api" -I"D:\e2workspace\mc_r7fa6m4\ra\fsp\inc\instances" -I"D:\e2workspace\mc_r7fa6m4\ra\arm\CMSIS_5\CMSIS\Core\Include" -I"D:\e2workspace\mc_r7fa6m4\ra_gen" -I"D:\e2workspace\mc_r7fa6m4\ra_cfg\fsp_cfg\bsp" -I"D:\e2workspace\mc_r7fa6m4\ra_cfg\fsp_cfg" -I"D:\e2workspace\mc_r7fa6m4\mlos\os" -I"D:\e2workspace\mc_r7fa6m4\boot" -I"D:\e2workspace\mc_r7fa6m4\mllib\iap" -I"D:\e2workspace\mc_r7fa6m4\mllib\iap\RA6M4" -I"D:\e2workspace\mc_r7fa6m4\mllib\iap\mc_ra6m4_iap_protocol" -I"D:\e2workspace\mc_r7fa6m4\mllib\mlflash\mcuflash" -I"D:\e2workspace\mc_r7fa6m4\mllib\ntp" -I"D:\e2workspace\mc_r7fa6m4\mllib\ntp\w5500" -I"D:\e2workspace\mc_r7fa6m4\mllib\ntp\w5500\ra6m4" -I"D:\e2workspace\mc_r7fa6m4\mllib\tool" -I"D:\e2workspace\mc_r7fa6m4\mllib\upper" -I"D:\e2workspace\mc_r7fa6m4\mlos\port\RA6M4" -I"D:\e2workspace\mc_r7fa6m4\mlos\services" -I"D:\e2workspace\mc_r7fa6m4\public" -I"D:\e2workspace\mc_r7fa6m4\ra_cfg\driver" -I"D:\e2workspace\mc_r7fa6m4\ra_cfg\driver" -I"D:\e2workspace\mc_r7fa6m4\ra\fsp\src\bsp\cmsis\Device\RENESAS\Include" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" -x c "$<"
	@echo 'Finished building: $<'
	@echo ' '


