/*
 * alarm_drive.c
 *
 *  Created on: 2021年11月1日
 *      Author: admin
 */
#include "alarm_drive.h"

void group0_alarmled_ctrl(mlu8 yellow,mlu8 green,mlu8 red)
{
    R_IOPORT_PinWrite (&g_ioport_ctrl, BSP_IO_PORT_04_PIN_09, yellow);
    R_IOPORT_PinWrite (&g_ioport_ctrl, BSP_IO_PORT_04_PIN_07, green);
    R_IOPORT_PinWrite (&g_ioport_ctrl, BSP_IO_PORT_04_PIN_06, red);
}

void group1_alarmled_ctrl(mlu8 yellow,mlu8 green,mlu8 red)
{
    R_IOPORT_PinWrite (&g_ioport_ctrl, BSP_IO_PORT_00_PIN_01, yellow);
    R_IOPORT_PinWrite (&g_ioport_ctrl, BSP_IO_PORT_00_PIN_03, green);
    R_IOPORT_PinWrite (&g_ioport_ctrl, BSP_IO_PORT_00_PIN_00, red);
}

void group2_alarmled_ctrl(mlu8 yellow,mlu8 green,mlu8 red)
{
    R_IOPORT_PinWrite (&g_ioport_ctrl, BSP_IO_PORT_00_PIN_05, yellow);
    R_IOPORT_PinWrite (&g_ioport_ctrl, BSP_IO_PORT_00_PIN_07, green);
    R_IOPORT_PinWrite (&g_ioport_ctrl, BSP_IO_PORT_04_PIN_04, red);
}

void group0_Buzzer_ctrl(mlu8 sta)
{
    R_IOPORT_PinWrite (&g_ioport_ctrl, BSP_IO_PORT_04_PIN_08, sta);
}

void group1_Buzzer_ctrl(mlu8 sta)
{
    R_IOPORT_PinWrite (&g_ioport_ctrl, BSP_IO_PORT_00_PIN_02, sta);
}

void group2_Buzzer_ctrl(mlu8 sta)
{
    R_IOPORT_PinWrite (&g_ioport_ctrl, BSP_IO_PORT_00_PIN_06, sta);
}




















