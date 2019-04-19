/*******************************************************************************
 *
 * Copyright (C) 2019 by Shilpi Gupta
 *
 ******************************************************************************/

/*
 * @file    uart.c
 * @brief   Library definitions for UART communication on the FRDM KL25Z MCU.
 * @version Project 2
 * @date    April 13, 2019
 * 
 * Tutorial Attribution:
 * Freescale ARM Cortex-M Embedded Programming by Muhammad Ali Mazidi et al.
 */

/*
IMPORTANT:
For this code, use a baud rate of 460,800 when setting up the serial port on the
host.

NOTES:
From "Freescale ARM Cortex-M Embedded Programming" by Muhammad Ali Mazidi et al.

"We monitor (poll) the TC flag bit to make sure that all the bits of the
last byte are transmitted. By the same logic, we monitor (poll) the RDRF
flag to see if a byte of data is received. The transmitter is double buffered.
While the shift register is shifting the last byte out, the program may
write another byte of data to the Data Register to wait for the shift
register to be ready. The transfer of data between the data register and
the shift register is automatic and the program does not have to worry
about it."

TCIE (Transmission Complete Interrupt Enable) - Enable the transmitter bit for
interrupt-driven UART, TIE:
0 = TC Interrupt Request is disabled.
1 = TC Interrupt Request is enabled.

TIE: Transmit Interrupt Enable for TDRE:
0 - TDRE interrupt requests disabled (i.e. use polling).
1 - TDRE interrupt request enabled

RIE
0 = RDRF Interrupt Request is disabled.
1 = RDRF Interrupt Request is enabled.

TDRE (Transmit Data Register Empty) (set in UART_S1).
0: Shift register is loaded and shifting. An additional byte is waiting
   in the data register.
1: Data register empty and ready for next byte.

TC (Transmit Complete) flag (bit 6 = 0x40):
0 = Transmission in progress (shift reg occupied)
1 = No transmission in progress (both shift reg and data reg empty)

RDRF (Receive Data Register Full flag (set in UART0_S1):
0 = No data available in UART data register.
1 = Data available in UART data register and ready to be picked up.
*/

#include "uart.h"
#include "led.h"
#include "MKL25Z4.h"
#include <stdio.h> // for sprintf

//#define ECHO_RX_ONLY // echo char with no tx interrupts
//#define ECHO_RX_TX // echo char with both rx and tx interrupts
//#define PRINT_TABLE_USE_RX_TX_RING // use a rx ring to store chars received by device uart
#define PRINT_TABLE_USE_TX_ONLY_RING // don't use an rx ring

#define SHOW_UNIQUE_IN_REPORT

// Define static variables.
const char *table_title = "\r\nCharacters\r\n";
const char *unique_title = "\r\n# unique chars: ";
int ascii[NUM_SYMBOLS];
int num_unique_chars;
ring_t *ring_rx;
ring_t *ring_tx;

void init_ascii_table()
{
    for (int i = 0; i < NUM_SYMBOLS; i++)
    {
    	ascii[i] = 0;
    }
}

void uart_init_buff()
{
    // Initialize ring buffer for receiving chars from host serial terminal.
    ring_rx = init(RING_BUFF_LEN);

    // Initialize ring buffer for transmitting chars from device UART.
    ring_tx = init(RING_BUFF_LEN);
}

void uart_init()
{
    // Enable clock for PORTA (bit 9 = 0x200).
    SIM->SCGC5 |= SIM_SCGC5_PORTA(1);

    // Enable clock for UART0.
    // Clock register for UARTs found in the SIM_SCGC4 (System Clock Gating
    // Control) register. UART0 clock is at bit 10 = 0x400.
    SIM->SCGC4 |= SIM_SCGC4_UART0(1);

    // Set source for baud rate generator clock for UART0 as FLL (41.94 MHz).
    // FLL = Frequency Locked Loop (vs. PLL = Phase Locked Loop)
    SIM->SOPT2 |= SIM_SOPT2_UART0SRC(1);

    // Select alt func 2 (ALT2) (bits 10-8: MUX = 010) for PA2 (UART0_Tx) pin
    // (i.e. make PTA2 the UART0_Tx pin) (0x02 = 010).
    PORTA->PCR[2] |= PORT_PCR_MUX(0x02);

    // Select alt func 2 (ALT2) (bits 10-8: MUX = 010) for PA1 (UART0_Rx) pin
    // (i.e. make PTA1 the UART0_Rx pin) (0x02 = 010).
    PORTA->PCR[1] |= PORT_PCR_MUX(0x02);

    // Turn off UART0 before making configuration changes.
    UART0->C2 = 0; // clear the C2 register (this includes disabling Tx & Rx)

    // Set baud rate for UART0. baud rate = clock rate / ((OSR + 1) * SBR).
    // OSR = 15, SBR = concat of UART0_BDH and UART0_BDL
    // DEFAULT_SYSTEM_CLOCK = 20971520u
    // 460,800 = 20971520 / ((15 + 1) * SBR)
    // For my code, using baud rate of 460,800 with SBR of 3 is what works.
    UART0->BDH = 0x00;
    UART0->BDL = UART0_BDL_SBR(3);

    // Set Over Sampling Ratio value to 16 (0x0F) for receiver.
    UART0->C4 |= (0x0F);

    // Set control register flags:
    // No parity (bit 1), 8-bit data size and 1 stop bit (bit 4)
    UART0->C1 = 0x00;

    // Enable the transmitter for UART0, TE (Transmit Enable).
    // Same as: UART0->C2 |= UART0_C2_TE_MASK; // mask = 0x8 = bit 4 is for TE
    UART0->C2 |= UART0_C2_TE(1);

    // Enable the receiver for UART0, RE (Receive Enable).
    // Same as: UART0->C2 |= UART0_C2_RE_MASK; // mask = 0x4 = bit 3 is for RE
    UART0->C2 |= UART_C2_RE(1);
}

void uart_init_interrupt()
{
    /* 1: Enable interrupt for the UART0 peripheral module.
       Each pin in a port can be used as an interrupt source. PORTxPCRn bits
       19-16 are for IRQC (Interrupt Configuration).
  
       2: Enable interrupt for NVIC (Nested Vector Interrupt Controller) module.
       There is an interrupt enable bit for each entry in the interrupt vector
       table and are located in the ISER (Interrupt Set Enable) registers of the
       NVIC. Each register holds 32 IRQ interrupts, accessed as register
       ISER[0], ISER[1], etc.  Similarly, to disable an interrupt, use the ICER
       (Interrupt Clear Enable) registers.
    */

    // Enable the receiver bit for interrupt-driven UART, RIE (Receiver Full
    // Interrupt Enable) (mask = 0x20 = bit 5 of UART0_C2 = RIE).
    UART0->C2 |= UART0_C2_RIE(1);

    // First disable/clear the interrupt for UART0 = IRQ #12 = bit 12 of ICER[0]
    // = 0x1000.
    NVIC_DisableIRQ(UART0_IRQn);

    // Enable the interrupt for UART0 = IRQ #12 = bit 12 of ISER[0] = 0x1000.
    NVIC_EnableIRQ(UART0_IRQn);
}

int uart_can_transmit()
{
    // Check the TDRE (Transmit Data Register Empty) flag (bit 7 = 0x80)
    // and the TC (Transmit Complete) flag (bit 6 = 0x40).
    if ((UART0->S1 & UART0_S1_TDRE(1)) == 0 ||
        (UART0->S1 & UART0_S1_TC(1)) == 0)
    {
        return 0; // shift reg still loaded
    }
    else
    {
        return 1; // data reg empty and ready 
    }
}

void uart_transmit(char c)
{
    // Transmit char. Writing to this reg starts a transmission from UART.
    UART0->D = c;
}

void uart_transmit_blocking(char c)
{
    // Wait for transmit buffer to be ready.
    while (!uart_can_transmit()) {}

    // Transmit character.
    uart_transmit(c);
}

int uart_can_receive()
{
    // Check the RDRF (Receive Data Register Full) flag (bit 5 = 0x20).
    if ((UART0->S1 & UART_S1_RDRF(1)) == 0)
    {
        return 0; // no data available
    }
    else
    {
        return 1; // data available
    }
}

char uart_receive()
{
    // Get character.
    return UART0->D;
}

char uart_receive_blocking()
{
    // Wait for receive buffer to be ready.
    while (!uart_can_receive()) {}

    return uart_receive();
}

void generate_tx_ring_report()
{
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
	    // Get the char string representation of the ascii[i] integer value.
	    // Ex: If the number of 'a' chars received is 12, we want to
            // transmit the value 12. sprintf is used to convert the dec = 12
            // value to a char string, and then the chars '1' and '2' are
            // inserted into the tx buffer.
    	    char count_as_char[MAX_COUNT_DIGITS + 1]; // + 1 for '\0' char
	    int num_digits = sprintf(count_as_char, "%d", ascii[i]);

   	    // Insert the char and its count, formatted.
	    // Each line is in the format: char - #\r\n (ex. b - 12).
            insert(ring_tx, (char)i); // the char from uart
            insert(ring_tx, ' ');     // a space
            insert(ring_tx, '-');     // a dash
            insert(ring_tx, ' ');     // a space
            for (int j = 0; j < num_digits; j++) // the char count
            {
                insert(ring_tx, count_as_char[j]);
            }
            insert(ring_tx, '\r');    // needed for new line
            insert(ring_tx, '\n');    // new line
 	}
    }
}

void generate_unique_chars_report()
{
    char count_as_char[MAX_COUNT_DIGITS + 1]; // + 1 for '\0' char
    int num_digits = sprintf(count_as_char, "%d", num_unique_chars);

    // Insert unique chars title into tx ring.
    // Line is in the format: unique chars: #
    const char *p = unique_title;
    while (*p != '\0')
    {
 	insert(ring_tx, *p);
	p++;
    }
    insert(ring_tx, ' ');
    for (int j = 0; j < num_digits; j++) // the char count
    {
    	insert(ring_tx, count_as_char[j]);
    }
    insert(ring_tx, '\r');
    insert(ring_tx, '\n');
}

void UART0_IRQHandler(void)
{
    // Prevent more UART0 interrupts from coming in.
    NVIC_DisableIRQ(UART0_IRQn);

#ifdef ECHO_RX_ONLY
    // Device UART receive char from host serial terminal.
    if (uart_can_receive())
    {
        // Get char from device UART.
    	char c = uart_receive();

    	// Insert char into app ring.
    	insert(ring_rx, c);
    }

    // Device UART transmit char to host serial terminal.
    if (uart_can_transmit())
    {
        if (entries(ring_rx) > 0)
    	{
      	    // Remove char from rx ring.
    	    char c;
            my_remove(ring_rx, &c);

            // Transmit char to host serial terminal.
            uart_transmit(c);
        }
    }
#endif

#ifdef ECHO_RX_TX

    // Device UART receive char from host serial terminal.
    if (uart_can_receive())
    {
        // Get char from device UART.
    	char rc = uart_receive();

    	// Insert char into rx ring.
    	insert(ring_rx, rc);

        if (entries(ring_rx) > 0)
    	{
    	    // Remove a char from rx ring.
    	    char tc;
            my_remove(ring_rx, &tc);

            // Add that char to tx ring.
            insert(ring_tx, tc);

            // Enable transmit interrupts.
            UART0->C2 |= UART0_C2_TIE(1);
    	}
    }
    // Device UART transmit char to host serial terminal.
    else if ((UART0->C2 & UART0_C2_TCIE(1)) == 0)
    {
        while (uart_can_transmit() && entries(ring_tx) > 0)
    	{
    	    // Remove char from tx ring.
    	    char tc;
            my_remove(ring_tx, &tc);

            // Transmit char to host serial terminal.
            uart_transmit(tc);
        }

        // Disable transmit interrupts so not constantly entering the interrupt
        // handler.
        UART0->C2 &= ~UART_C2_TIE(1);
    }
#endif

#ifdef PRINT_TABLE_USE_RX_TX_RING

    // Device UART receive char from host serial terminal.
    if (uart_can_receive())
    {
        // Get char from device UART.
    	char rc = uart_receive();

    	// Insert char into rx ring.
    	int ret = insert(ring_rx, rc);

        if (ret && entries(ring_rx) > 0)
    	{
    	    // Remove a char from rx ring.
    	    char tc;
            int ret = my_remove(ring_rx, &tc);

            if (ret)
            {
            	// Increment count for char tc.
            	ascii[(int)rc]++;

            	// Add char to tx ring. The tx ring is formatted as a table.
            	generate_tx_ring_report();

                // Enable transmit interrupts.
                UART0->C2 |= UART0_C2_TIE(1);
            }
    	}
    }
    // Device UART transmit char to host serial terminal.
    else if ((UART0->C2 & UART0_C2_TCIE(1)) == 0)
    {
        // Transmit tx ring table to host serial terminal from device UART.
        while (entries(ring_tx) > 0)
        {
            // Remove char from the tx ring.
            char tx_c;
            int ret = my_remove(ring_tx, &tx_c);
            if (ret && uart_can_transmit())
            {
                uart_transmit(tx_c);
            }
        }

        // Disable transmit interrupts so not constantly entering the interrupt
        // handler.
        UART0->C2 &= ~UART_C2_TIE(1);
    }
#endif

#ifdef PRINT_TABLE_USE_TX_ONLY_RING

    // Device UART receive char from host serial terminal.
    if (uart_can_receive())
    {
        // Get char from device UART.
    	char rc = uart_receive();

    	// Increment count for received char rc.
    	ascii[(int)rc]++;

        // Generate a report. The tx ring contains a count of received chars and
    	// is formatted as a simple table.
    	generate_tx_ring_report();

#ifdef SHOW_UNIQUE_IN_REPORT
    	// Report the number of unique chars received.
        generate_unique_chars_report();
#endif

        // Enable transmit interrupts.
        UART0->C2 |= UART0_C2_TIE(1);
    }
    // Device UART transmit char to host serial terminal. TCIE 1 means TC is interrupt enabled.
    else if ((UART0->C2 & UART0_C2_TCIE(1)) == 0)
    {
        // Transmit tx ring table to host serial terminal from device UART.
        while (entries(ring_tx) > 0)
        {
            // Remove char from the tx ring.
            char tc;
            int ret = my_remove(ring_tx, &tc);
            if (ret && uart_can_transmit())
            {
                uart_transmit(tc);
            }
        }

        // Disable transmit interrupts so not constantly entering the interrupt
        // handler.
        UART0->C2 &= ~UART_C2_TIE(1);
    }
#endif
    // Renable UART0 interrupts.
    NVIC_EnableIRQ(UART0_IRQn);
}
