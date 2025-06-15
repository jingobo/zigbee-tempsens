#include "mcu.h"
#include "zed.h"
#include "uart.h"
#include "sens.h"

// Главня точка входа
__noreturn void main(void)
{
    mcu_init();
    zed_init();
    uart_init();
    sens_init();
    
    osal_init_system();

    // Цикл обработки сообщений
    IRQ_ENABLE();
    for (;;)
        osal_start_system();
}

// Ранняя точка входа
__root char __low_level_init(void);
__root char __low_level_init(void)
{
    mcu_init_early();
    return 1;
}
