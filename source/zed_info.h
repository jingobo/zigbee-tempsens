#ifndef __ZED_INFO_H
#define __ZED_INFO_H

#include <zcl.h>
#include <AF.h>

// Количество поддерживаемых команд
#define ZED_INFO_COMMANDS_COUNT     1

// Список поддерживаемых команд
extern CONST zclCommandRec_t ZED_INFO_COMMANDS[];

// Количество поддерживаемых аттрибутов
#define ZED_INFO_ATTRIBUTES_COUNT       17

// Список поддерживаемых аттрибутов
extern CONST zclAttrRec_t ZED_INFO_ATTRIBUTES[];

// Дескриптор приложения
extern SimpleDescriptionFormat_t ZED_INFO_APP_DESC;

#endif // __ZED_INFO_H
