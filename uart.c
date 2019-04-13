#include "uart.h"
#include "led.h"
#include "MKL25Z4.h"

/*
NOTES:
"We monitor poll) the TC flag bit to make sure that all the bits of the
last byte are transmitted. By the same logic, we monitor (poll) the RDRF
flag to see if a byte of data is received. The transmitter is double buffered.
While the shift register is shifting the last byte out, the program may
write another byte of data to the Data Register to wait for the shift
register to be ready. The transfer of data between the data register and
the shift register is automatic and the program does not have to worry
about it."

TCIE (Transmission Complete Interrupt Enable) = bit 6 in UART0_C2.
Used for interrupt-driven UART.
TCIE 0 = TC Interrupt Request is disabled.
TCIE 1 = TC Interrupt Request is enabled.

Enable the transmitter bit for interrupt-driven UART, TIE (Transmitter Full
Interrupt Enable) (mask = 0x80 = bit 7 of UART0_C2 = TIE).

Transmit Interrupt Enable for TDRE
TIE 0 - TDRE interrupt requests disabled (i.e. use polling).
TIE 1 - TDRE interrupt request enabled

TDRE (Transmit Data Register Empty) (set in UART_S1).
TDRE 0: Shift register is loaded and shifting. An additional byte is waiting
in the data register.
TDRE 1: Data register empty and ready for next byte.
*/

//#define SIMPLE_ECHO_TEST
//#define ECHO_TEST
#define PRINT_TABLE

void init_ascii_table()
{
    for (int i = 0; i < NUM_SYMBOLS; i++)
    {
    	ascii[i] = 0;
    }
}

void uart_init_buff()
{
    // Initialize ring buffer for receiving chars from host UART.
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

    // Set baud rate for UART0. baud rate = clock rate / ((OSR + 1) * SBR)
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

    // Enable the transmitter for UART0, TE (Transmit Enable)
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
	//
	// RIE 0 = RDRF	Interrupt Request is disabled.
	// RIE 1 = RDRF	Interrupt Request is enabled.
	//
	// RDRF (Receive Data Register Full flag (set in UART0_S1).
	// RDRF 0 = No data available in UART data register.
	// RDRF 1 = Data available in UART data register and ready to be picked up.
    UART0->C2 |= UART0_C2_RIE(1);

    // First disable/clear the interrupt for UART0 = IRQ #12 = bit 12 of ICER[0]
    // = 0x1000.
    NVIC_DisableIRQ(UART0_IRQn);

    // Enable the interrupt for UART0 = IRQ #12 = bit 12 of ISER[0] = 0x1000.
    NVIC_EnableIRQ(UART0_IRQn);
}

int uart_can_transmit()
{
    /* The TDRE (Transmit Data Register Empty) flag (bit 7 = 0x80):
       0 = Shift register is loaded and shifting. An additional byte is waiting
           in the Data Register.
       1 = The Data Register is empty and ready for the next byte.

       The TC (Transmit Complete) flag (bit 6 = 0x40):
       0 = Transmission in progress (shift reg occupied)
       1 = No transmission in progress (both shift reg and data reg empty)
    */

    // TDRE = 0x80 = bit 7
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
    // Transmit char. Writing to this reg starts a transmission from uart.
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
    // The RDRF (Receive Data Register Full) flag (bit 5):
    // 0 = No data available in uart data reg.
    // 1 = Data available in uart data reg and ready to be picked up.
    // RDRF = 0x20 = bit 5
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

void populate_tx_table_ring(char c)
{
	// Increment count for char tc.
	ascii[(int)c]++;

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
			// For ex: If the number of 'a' chars received is 12, we want to transmit
			// the value 12. So, using sprintf to convert the dec = 12 value to a char
			// string, and then inserting the chars '1' and '2' into the buffer.
			char count_as_char[MAX_COUNT_DIGITS + 1]; // +1 for '\0' char
			int num_digits = sprintf(count_as_char, "%d", ascii[i]);

			// Each line is in the format: char - #
			// Ex. b - 3
            insert(ring_tx, (char)i); // the char from uart
            insert(ring_tx, ' ');     // a space
            insert(ring_tx, '-');     // a dash
            insert(ring_tx, ' ');     // a space
            for (int j = 0; j < num_digits; j++) // the char's count
            {
                insert(ring_tx, count_as_char[j]);
            }
            insert(ring_tx, '\r'); //
            insert(ring_tx, '\n');
		}
	}
}

void UART0_IRQHandler(void)
{
    NVIC_DisableIRQ(UART0_IRQn);

#ifdef SIMPLE_ECHO_TEST
    // Device receive char from host uart.
    if (uart_can_receive())
    {
        // Get char from device uart.
    	char c = uart_receive();

    	// Insert char into app ring.
    	insert(ring_rx, c);
    }

    // Device transmit char to host uart.
    if (uart_can_transmit())
    {
        if (entries(ring_rx) > 0)
    	{
    		// Remove char from rx ring.
    		char c;
            my_remove(ring_rx, &c);

            // Transmit char to host uart.
            uart_transmit(c);

        } // end if entries > 0
    } // end if uart can transmit
#endif

#ifdef ECHO_TEST

    // Device receive char from host uart.
    if (uart_can_receive())
    {
        // Get char from device uart.
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
            // TODO: Need to do a check here to see if ok to enable?
            UART0->C2 |= UART0_C2_TIE(1);
    	}
    }
    // Device transmit char to host uart.
    else if ((UART0->C2 & UART0_C2_TCIE(1)) == 0)
    {
        while (uart_can_transmit() && entries(ring_tx) > 0)
    	{
    		// Remove char from tx ring.
    		char tc;
            my_remove(ring_tx, &tc);

            // Transmit char to host uart.
            uart_transmit(tc);
        }

        // Disable transmit interrupts so not constantly entering the interrupt handler.
        UART0->C2 &= ~UART_C2_TIE(1);
    }
#endif

#ifdef PRINT_TABLE_WITH_TIE

    // Device receive char from host uart.
    if (uart_can_receive())
    {
        // Get char from device uart.
    	char rc = uart_receive();

    	// Insert char into rx ring.
    	insert(ring_rx, rc);

        if (entries(ring_rx) > 0)
    	{
    		// Remove a char from rx ring.
    		char tc;
            int ret = my_remove(ring_rx, &tc);

        	if (ret)
            {
            	// Add char to tx ring. Tx ring is formatted as a table.
            	populate_tx_table_ring(tc);
            }

            // Enable transmit interrupts.
            // TODO: Need to do a check here to see if ok to enable?
            UART0->C2 |= UART0_C2_TIE(1);
    	}
    }
    // Device transmit char to host uart.
    else if ((UART0->C2 & UART0_C2_TCIE(1)) == 0)
    {
        // Transmit tx ring table to host UART from device UART.
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

        // Disable transmit interrupts so not constantly entering the interrupt handler.
        UART0->C2 &= ~UART_C2_TIE(1);
    }
#endif
    NVIC_EnableIRQ(UART0_IRQn);
}

