#ifndef __MCU_H
#define __MCU_H

#include "typedefs.h"
#include <OnBoard.h>

// Перечисление причин сброса
typedef enum
{
    // По питанию
    MCU_RESET_REASON_POR,
    // По выходу сброса
    MCU_RESET_REASON_PIN,
    // По ватчдогу
    MCU_RESET_REASON_WDT,
} mcu_reset_reason_t;

// Причина сброса (только чтение)
extern mcu_reset_reason_t mcu_reset_reason;

// Инициализация модуля
void mcu_init(void);
// Ранняя инициализация модуля
void mcu_init_early(void);

// Настройка ватчдога
static void mcu_wdt_config(uint8_t wdti)
{
    WDCTL = WDCLP1 | WDEN | (wdti & WDINT);
    WDCTL = WDCLP2 | WDEN | (wdti & WDINT);
}

#endif // __MCU_H
