#ifndef __SENS_H
#define __SENS_H

#include "typedefs.h"

// ������� �������� ����������� (������ ������)
extern int16_t sens_temp_current;

// �������� ��������� ����������� [C]
extern const int16_t SENS_TEMP_RANGE_MIN,
                     SENS_TEMP_RANGE_MAX;

// �������� ��������� ����������� [C]
extern const uint16_t SENS_TEMP_TOLERANCE;

// ������ �������� ���������� (������ ������)
extern uint8_t sens_bat_volatage;
// ������ �������� �������� ������������� (������ ������)
extern uint8_t sens_bat_precentage;

// ������������� �������
void sens_init(void);

#endif // __SENS_H
