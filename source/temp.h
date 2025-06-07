#ifndef __TEMP_H
#define __TEMP_H

#include "typedefs.h"

// ������� �������� ����������� (������ ������)
extern int16_t temp_current;

// �������� ��������� ����������� [C]
extern const int16_t TEMP_RANGE_MIN,
                     TEMP_RANGE_MAX;

// �������� ��������� ����������� [C]
extern const uint16_t TEMP_TOLERANCE;

// ������������� �������
void temp_init_task(uint8_t task);
// ���������� �������� �����
uint16_t temp_event_loop(uint8_t task_id, uint16_t events);

#endif // __TEMP_H
