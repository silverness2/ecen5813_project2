/**
 * @file    test1-project2.c
 * @brief   Application entry point.
 * 
 * ATTRIBUTIONS:
 * KL25 Sub-Family Reference Manual by NXP.
 * Freescale ARM Cortex-M Embedded Programming by Muhammad Ali Mazidi et al.
 */

/**
 * @file    test1-project2.c
 * @brief   Application entry point.
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

#define TEST_LED
//#define USE_BLOCKING
//#define USE_INTERRUPT
//#define USE_BLOCKING_WITH_APP

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

#ifdef TEST_LED
    // Initialize blue led.
    led_blue_init();

    // Toggle blue led.
    while (1)
    {
        set_led_blue_on();
        delay(500);
        set_led_blue_off();
        delay(500);
    }
#endif

#ifdef USE_BLOCKING_WITH_APP
    // Initialize UART hardware.
    uart_init();

    // Initialize ring buffer.
    #include "ring.h"
    #define RING_BUFF_LEN 256
    ring_t *ring_rx;
    ring_t *ring_tx;
    // Initialize ring buffer for receiving chars from host UART.
    ring_rx = init(RING_BUFF_LEN);
    // Initialize ring buffer for transmitting chars from device UART.
    ring_tx = init(RING_BUFF_LEN);
    const char *table_title = "Characters\r\n";

    // Initialize character count.
    #define MAX_COUNT_DIGITS 3
    #define NUM_SYMBOLS 256
    int ascii[NUM_SYMBOLS];
    for (int i = 0; i < NUM_SYMBOLS; i++)
    {
    	ascii[i] = 0;
    }

    int gc = 0;

    while (1)
    {
    	char rc, tc;
        int ret;

        // UART on device receives char from UART on host.
        rc = uart_receive_blocking();

        // Insert char into the rx ring.
        ret = insert(ring_rx, rc);

        // UART on device transmits char back to UART on host.
        if (ret)
        {
        	// Remove char from the rx ring.
        	ret = my_remove(ring_rx, &tc);
            if (ret)
            {
            	// Increment count for char tc.
            	ascii[(int)tc]++;

            	// Insert table title into tx ring.
            	const char *p = table_title;
            	while (*p != '\0')
            	{
            		insert(ring_tx, *p);
            		p++;
            	}

            	// Insert chars that have a count > 0 into tx ring.
            	for (int i = 0; i < NUM_SYMBOLS; i++)
            	{
            		if (ascii[i] != 0)
            		{
            			char count_as_char[MAX_COUNT_DIGITS + 1];
            			int num_digits = sprintf(count_as_char, "%d", ascii[i]);

                        insert(ring_tx, (char)i);
                        insert(ring_tx, ' ');
                        insert(ring_tx, '-');
                        insert(ring_tx, ' ');
                        for (int j = 0; j < num_digits; j++)
                        {
                            insert(ring_tx, count_as_char[j]);
                        }
                        insert(ring_tx, '\r');
                        insert(ring_tx, '\n');
            		}
            	}

            	// Print the table to host UART by transmitting from device UART.
            	while (entries(ring_tx) > 0)
            	{
                	// Remove char from the tx ring.
            		char tx_c;
                	ret = my_remove(ring_tx, &tx_c);
                    if (ret)
                    {
                    	uart_transmit(tx_c);
                    }
            	}
            }
        }
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

    return 0;
}
