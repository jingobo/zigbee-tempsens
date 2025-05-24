#include "mcu.h"
#include "zed.h"
#include "uart.h"

// ������ ����� �����
__noreturn void main(void)
{
    // ���������������
    mcu_init();

    // Initialize NV System
    osal_nv_init();

    // �������� ����������
    zed_init();

    // ���������� ����
    uart_init();
    
    // Initialize the operating system
    osal_init_system();

    // ���� ��������� ���������
    IRQ_ENABLE();
    for (;;)
        osal_start_system();
}

// ������ ����� �����
__root char __low_level_init(void);
__root char __low_level_init(void)
{
    mcu_init_early();
    return 1;
}
