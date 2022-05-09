################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../tray/alarm_drive.c \
../tray/alarm_led.c \
../tray/boot.c \
../tray/box.c \
../tray/channel.c \
../tray/entry.c \
../tray/flow.c \
../tray/func.c \
../tray/host.c \
../tray/log.c \
../tray/pb.c \
../tray/plc.c \
../tray/protect.c \
../tray/timer.c \
../tray/tray.c \
../tray/tray_bl.c \
../tray/uart.c 

OBJS += \
./tray/alarm_drive.o \
./tray/alarm_led.o \
./tray/boot.o \
./tray/box.o \
./tray/channel.o \
./tray/entry.o \
./tray/flow.o \
./tray/func.o \
./tray/host.o \
./tray/log.o \
./tray/pb.o \
./tray/plc.o \
./tray/protect.o \
./tray/timer.o \
./tray/tray.o \
./tray/tray_bl.o \
./tray/uart.o 

C_DEPS += \
./tray/alarm_drive.d \
./tray/alarm_led.d \
./tray/boot.d \
./tray/box.d \
./tray/channel.d \
./tray/entry.d \
./tray/flow.d \
./tray/func.d \
./tray/host.d \
./tray/log.d \
./tray/pb.d \
./tray/plc.d \
./tray/protect.d \
./tray/timer.d \
./tray/tray.d \
./tray/tray_bl.d \
./tray/uart.d 


# Each subdirectory must supply rules for building sources it contributes
tray/%.o: ../tray/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -O1 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wunused -Wuninitialized -Wall -Wextra -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal  -g -D_RENESAS_RA_ -I"D:\e2DiscExpr\mc_r7fa6m4\src" -I"D:\e2DiscExpr\mc_r7fa6m4\ra\fsp\inc" -I"D:\e2DiscExpr\mc_r7fa6m4\ra\fsp\inc\api" -I"D:\e2DiscExpr\mc_r7fa6m4\ra\fsp\inc\instances" -I"D:\e2DiscExpr\mc_r7fa6m4\ra\arm\CMSIS_5\CMSIS\Core\Include" -I"D:\e2DiscExpr\mc_r7fa6m4\ra_gen" -I"D:\e2DiscExpr\mc_r7fa6m4\ra_cfg\fsp_cfg\bsp" -I"D:\e2DiscExpr\mc_r7fa6m4\ra_cfg\fsp_cfg" -I"D:\e2DiscExpr\mc_r7fa6m4\mlos\os" -I"D:\e2DiscExpr\mc_r7fa6m4\mlos\port" -I"D:\e2DiscExpr\mc_r7fa6m4\app" -I"D:\e2DiscExpr\mc_r7fa6m4\ra_cfg\driver" -I"D:\e2DiscExpr\mc_r7fa6m4\mlos\services" -I"D:\e2DiscExpr\mc_r7fa6m4\mlos\port\RA6M4" -I"D:\e2DiscExpr\mc_r7fa6m4\mllib\utp" -I"D:\e2DiscExpr\mc_r7fa6m4\mllib\ntp" -I"D:\e2DiscExpr\mc_r7fa6m4\mllib\ctp" -I"D:\e2DiscExpr\mc_r7fa6m4\mllib\ntp\w5500" -I"D:\e2DiscExpr\mc_r7fa6m4\mllib\ctp\ra6m4" -I"D:\e2DiscExpr\mc_r7fa6m4\mllib\utp\ra6m4" -I"D:\e2DiscExpr\mc_r7fa6m4\mllib\ntp\w5500\ra6m4" -I"D:\e2DiscExpr\mc_r7fa6m4\mllib\ntp\w5100\ra6m4" -I"D:\e2DiscExpr\mc_r7fa6m4\mllib\ntp\w5100" -I"D:\e2DiscExpr\mc_r7fa6m4\ra\fsp\src\bsp\cmsis\Device\RENESAS\Include" -I"D:\e2DiscExpr\mc_r7fa6m4\mllib\tool" -I"D:\e2DiscExpr\mc_r7fa6m4\mllib\upper" -I"D:\e2DiscExpr\mc_r7fa6m4\mllib\mlflash\mx35xx" -I"D:\e2DiscExpr\mc_r7fa6m4\mllib\mlflash\mx35xx\RA6M4" -I"D:\e2DiscExpr\mc_r7fa6m4\mllib\mlflash\cssdcard\RA6M4" -I"D:\e2DiscExpr\mc_r7fa6m4\mllib\mlflash\cssdcard" -I"D:\e2DiscExpr\mc_r7fa6m4\public" -I"D:\e2DiscExpr\mc_r7fa6m4\mllib\iap" -I"D:\e2DiscExpr\mc_r7fa6m4\mllib\iap\RA6M4" -I"D:\e2DiscExpr\mc_r7fa6m4\mllib\mlflash\mcuflash" -I"D:\e2DiscExpr\mc_r7fa6m4\tray" -I"D:\e2DiscExpr\mc_r7fa6m4\mllib\dataStorage" -I"D:\e2DiscExpr\mc_r7fa6m4\combine" -I"D:\e2DiscExpr\mc_r7fa6m4\baseStruct" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" -x c "$<"
	@echo 'Finished building: $<'
	@echo ' '


