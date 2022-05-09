/*
 * alarm_drive.h
 *
 *  Created on: 2021年11月1日
 *      Author: admin
 */

#ifndef ALARM_LED_RA6M4_ALARM_DRIVE_H_
#define ALARM_LED_RA6M4_ALARM_DRIVE_H_

#include "mlos.h"

void group0_alarmled_ctrl(mlu8 yellow,mlu8 green,mlu8 red);
void group1_alarmled_ctrl(mlu8 yellow,mlu8 green,mlu8 red);
void group2_alarmled_ctrl(mlu8 yellow,mlu8 green,mlu8 red);
void group0_Buzzer_ctrl(mlu8 sta);
void group1_Buzzer_ctrl(mlu8 sta);
void group2_Buzzer_ctrl(mlu8 sta);

#endif /* ALARM_LED_RA6M4_ALARM_DRIVE_H_ */
