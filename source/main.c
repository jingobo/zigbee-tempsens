#include "mcu.h"
#include "zed.h"
#include "uart.h"

// Главня точка входа
__noreturn void main(void)
{
    // Микроконтроллер
    mcu_init();

    // Initialize NV System
    osal_nv_init();

    // Конечное устройство
    zed_init();

    // Отладочный порт
    uart_init();
    
    // Initialize the operating system
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
