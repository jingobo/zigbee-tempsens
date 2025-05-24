#include "uart.h"
#include <hal_uart.h>

// ���������� ����
static const uint8_t UART_DBG_PORT = HAL_UART_PORT_0;
// �������� ����
static const uint8_t UART_HTM_PORT = HAL_UART_PORT_1;

// ������� ���������� �����
static uart_recv_cb_t uart_recv_cb = NULL;

// ���������� ������� �����
static void uart_htm_event_cb(uint8_t port, uint8_t event)
{
    assert(port == UART_HTM_PORT);
    
    switch (event)
    {
        // ��������
        case HAL_UART_TX_FULL:
        case HAL_UART_TX_EMPTY:
            // �� �������������
            break;

        // ����
        case HAL_UART_RX_FULL:
        case HAL_UART_RX_ABOUT_FULL:
        case HAL_UART_RX_TIMEOUT:
            {
                // ���� � ������
                uint16_t avail = Hal_UART_RxBufLen(port);
                if (avail > UART_BUFFER_SIZE)
                    avail = UART_BUFFER_SIZE;
                
                // ������
                static __no_init uint8_t buffer[UART_BUFFER_SIZE];
                HalUARTRead(port, buffer, avail);
                
                // ����������
                if (uart_recv_cb == NULL)
                    return;
                uart_recv_cb(buffer, (uint8_t)avail);
                uart_recv_cb = NULL;
            }
            break;
            
        // ������
        default:
            assert(FALSE);
            return;
    }
}

void uart_init(void)
{
    // ���������� ����
    {
        halUARTCfg_t cfg =
        {
            .baudRate = HAL_UART_BR_115200,
        };
        
        HalUARTOpen(UART_DBG_PORT, &cfg);
    }

    // �������� ����
    {
        halUARTCfg_t cfg =
        {
            .baudRate = HAL_UART_BR_9600,
            .callBackFunc = uart_htm_event_cb,
        };
        
        HalUARTOpen(UART_HTM_PORT, &cfg);
    }
}

void uart_request(const void *source, uint8_t size, uart_recv_cb_t cb)
{
    assert(source != NULL);
    assert(size > 0);
    assert(size <= UART_BUFFER_SIZE);
    
    // ������ ��������
    uart_recv_cb = cb;
    HalUARTWrite(UART_HTM_PORT, (uint8_t *)source, size);
}

__near_func int putchar(int c)
{
    HalUARTWrite(UART_DBG_PORT, (uint8_t *)&c, 1);
    return c;
}
