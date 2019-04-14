/*******************************************************************************
 *
 * Copyright (C) 2019 by Shilpi Gupta
 *
 ******************************************************************************/

/*
 * @file    main_uart..c
 * @brief   A program for UART communication on the Freedom Freescale (FRDM)
 *          KL25Z Microcontroller (MCU).
 * @version Project 2
 * @date    April 13, 2019
 */

#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKL25Z4.h"
#include "fsl_debug_console.h"
#include "uart.h"
#include "led.h"

//#define TEST_LED
//#define USE_BLOCKING
#define USE_INTERRUPT

#define DELAY_MS 100

void compute_num_unique_chars();

int main(void) {

    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
    /* Init FSL debug console. */
    BOARD_InitDebugConsole();

#ifdef TEST_LED
    // Initialize blue led.
    led_blue_init();

    // Toggle blue led.
    while (1)
    {
        set_led_blue_on();
        delay(DELAY_MS);
        set_led_blue_off();
        delay(DELAY_MS);
    }
#endif

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
    // Initialize global variables.
    num_unique_chars = 0;

    // Disable interrupts (IRQs) globally for setup.
    __disable_irq();

    // Initialize ring buffer.
    uart_init_buff();

    // Initialize UART general hardware.
    uart_init();

    // Initialize UART hardware interrupts.
    uart_init_interrupt();

    // Initialize blue led.
    led_blue_init();

    // Enable interrupts (IRQs) globally for setup.
    __enable_irq();

    // Run forever while interrupts get called.
    while (1)
    {
        // Toggle an led when not in interrupt code.
    	set_led_blue_on();
        delay(DELAY_MS);
        set_led_blue_off();
        delay(DELAY_MS);

        // Count number of unique chars that have been received.
        compute_num_unique_chars();
    }
#endif

    return 0;
}

// TODO: BUG: This always reports one char less bc of timing of interrupt
// and when the number is computer.
void compute_num_unique_chars()
{
    num_unique_chars = 0;
    for (int i = 0; i < NUM_SYMBOLS; i++)
    {
        if (ascii[i] != 0)
        {
            num_unique_chars++;
        }
    }
}
