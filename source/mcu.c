#include "mcu.h"
#include <hal_drivers.h>

// Причина сброса
__no_init mcu_reset_reason_t mcu_reset_reason;

void mcu_init(void)
{
    SLEEPCMD &= ~OSC_PD;                                                        // Turn on 16MHz RC and 32MHz XOSC
    while ((SLEEPSTA & XOSC_STB) == 0)                                          // Wait for 32MHz XOSC stable 
    { }
    __no_operation();                                                           // Chip bug workaround
  
    for (uint16_t i = 0; i < 504; i++)                                          // Require 63us delay for all revs
        __no_operation();
  
    CLKCONCMD = CLKCONCMD_32MHZ | OSC_32KHZ;                                    // Select 32MHz XOSC and the source for 32K clock
    while (CLKCONSTA != CLKCONCMD_32MHZ | OSC_32KHZ)                            // Wait for the change to be effective
    { }
  
    SLEEPCMD |= OSC_PD;                                                         // Turn off 16MHz RC
    FCTL = 0x08;                                                                // Turn on cache prefetch mode 

    // Отключение светодиодов
    HAL_TURN_OFF_LED1();
    LED1_DDR |= LED1_BV;
    HAL_TURN_OFF_LED2();
    LED2_DDR |= LED2_BV;
    HAL_TURN_OFF_LED3();
    LED3_DDR |= LED3_BV;
    
    // Инициалзиация HAL
    HalDriverInit();
    osal_nv_init();
}

void mcu_init_early(void)
{
    // Отключение прерываний
    IRQ_DISABLE();
    
    // Причина сброса
    const uint8 rib = SLEEPSTA & LRESET;
    switch (rib)
    {
        case RESETPO:
            mcu_reset_reason = MCU_RESET_REASON_POR;
            break;
            
        case RESETEX:
            mcu_reset_reason = MCU_RESET_REASON_PIN;
            break;
            
        case RESETWD:
        default:
            mcu_reset_reason = MCU_RESET_REASON_WDT;
            break;
    }
    
    // Map flash bank #1 into XDATA for access to "ROM mapped as data"
    MEMCTR = (MEMCTR & 0xF8) | 0x01;
}
