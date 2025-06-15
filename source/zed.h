#ifndef __ZED_H
#define __ZED_H

// ZED - Zigbee End Device

#include "typedefs.h"

// ����� �������� �����
#define ZED_ENDPOINT        8

// ��������� ���������� ����������
extern bool_t zed_light_state;
// ����� �� ���������� ������������� [���]
extern uint16_t zed_identify_time;

// ������������� ������
void zed_init(void);
// ������������� ������
void zed_init_task(uint8_t task_id);
// ��������� ������� �������
uint16 zed_event_loop(uint8_t task_id, uint16_t events);

// ���������� ������� ��������� �������� �������
void zed_attr_bat_changed(void);
// ���������� ������� ��������� �������� �����������
void zed_attr_temp_changed(void);

#endif // __ZED_H
