################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../boot/u_boot.c 

OBJS += \
./boot/u_boot.o 

C_DEPS += \
./boot/u_boot.d 


# Each subdirectory must supply rules for building sources it contributes
boot/%.o: ../boot/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -O2 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal  -g -D_RENESAS_RA_ -I"D:\e2workspace\mc_r7fa6m4\src" -I"D:\e2workspace\mc_r7fa6m4\ra\fsp\inc" -I"D:\e2workspace\mc_r7fa6m4\ra\fsp\inc\api" -I"D:\e2workspace\mc_r7fa6m4\ra\fsp\inc\instances" -I"D:\e2workspace\mc_r7fa6m4\ra\arm\CMSIS_5\CMSIS\Core\Include" -I"D:\e2workspace\mc_r7fa6m4\ra_gen" -I"D:\e2workspace\mc_r7fa6m4\ra_cfg\fsp_cfg\bsp" -I"D:\e2workspace\mc_r7fa6m4\ra_cfg\fsp_cfg" -I"D:\e2workspace\mc_r7fa6m4\mlos\os" -I"D:\e2workspace\mc_r7fa6m4\mlos\port" -I"D:\e2workspace\mc_r7fa6m4\app" -I"D:\e2workspace\mc_r7fa6m4\ra_cfg\driver" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" -x c "$<"
	@echo 'Finished building: $<'
	@echo ' '


