/**
 * @file    test1-project2.c
 * @brief   Application entry point.
 * 
 *ATTRIBUTIONS: BOARD_Initx functions taken from Copyright 2016-2018 NXP.
 */

#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKL25Z4.h"
#include "fsl_debug_console.h"

#define USE_BLOCKING
//#define USE_INTERRUPT

/*
 * @brief   Application entry point.
 */
int main(void) {

    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
    /* Init FSL debug console. */
    BOARD_InitDebugConsole();

#ifdef USE_BLOCKING
    // Initialize UART hardware.
    uart_init();

    while (1)
    {
        // UART on device receives char from UART on host.
        char c = uart_receive_blocking();

        // UART on device transmits char back to UART on host.
        uart_transmit_blocking(c);
    }
#endif

#ifdef USE_INTERRUPT

    // Disable interrupts (IRQs) globally for setup using C pseudo function.
    __disable_irq();

    // Initialize ring buffer.
    uart_init_buff();

    // Initialize UART general hardware.
    uart_init();

    // Initialize UART hardware interrupts.
    uart_init_interrupt();

    // Enable interrupts (IRQs) globally for setup.
    __enable_irq();

    // Run forever while interrupts get called.
    while (1) {}
#endif

    return 0 ;
}
