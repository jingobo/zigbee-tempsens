#ifndef __MCU_H
#define __MCU_H

#include "typedefs.h"
#include <OnBoard.h>

// ������������ ������ ������
typedef enum
{
    // �� �������
    MCU_RESET_REASON_POR,
    // �� ������ ������
    MCU_RESET_REASON_PIN,
    // �� ��������
    MCU_RESET_REASON_WDT,
} mcu_reset_reason_t;

// ������� ������ (������ ������)
extern mcu_reset_reason_t mcu_reset_reason;

// ������������� ������
void mcu_init(void);
// ������ ������������� ������
void mcu_init_early(void);

// ��������� ��������
static void mcu_wdt_config(uint8_t wdti)
{
    WDCTL = WDCLP1 | WDEN | (wdti & WDINT);
    WDCTL = WDCLP2 | WDEN | (wdti & WDINT);
}

#endif // __MCU_H
