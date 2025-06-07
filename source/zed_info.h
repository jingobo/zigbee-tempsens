#ifndef __ZED_INFO_H
#define __ZED_INFO_H

#include <zcl.h>
#include <AF.h>

// ���������� �������������� ������
#define ZED_INFO_COMMANDS_COUNT     1

// ������ �������������� ������
extern CONST zclCommandRec_t ZED_INFO_COMMANDS[];

// ���������� �������������� ����������
#define ZED_INFO_ATTRIBUTES_COUNT       17

// ������ �������������� ����������
extern CONST zclAttrRec_t ZED_INFO_ATTRIBUTES[];

// ���������� ����������
extern SimpleDescriptionFormat_t ZED_INFO_APP_DESC;

#endif // __ZED_INFO_H
