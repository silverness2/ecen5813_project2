#include <stdio.h>
#include <stdlib.h>
#include "uart.h"

int main()
{
    // Initialize UART hardware.
    uart_init();

    while (1)
    {
        uart_transmit_blocking('H');
        char c = uart_receive_blocking();
    }

    return (EXIT_SUCCESS);
}
