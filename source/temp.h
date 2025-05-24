#ifndef __TEMP_H
#define __TEMP_H

#include "typedefs.h"

// ������� �������� ����������� (������ ������)
extern int16_t temp_current;

// �������� ��������� ����������� [C]
extern const int16_t TEMP_RANGE_MIN,
                     TEMP_RANGE_MAX;

// ������������� �������
void temp_init_task(uint8_t task);
// ���������� �������� �����
uint16_t temp_event_loop(uint8_t task_id, uint16_t events);

// ���������� ������ �����������
void temp_query(void);

#endif // __TEMP_H
