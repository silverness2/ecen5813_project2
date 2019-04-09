#include "uart.h"
#include "MKL25Z4.h"

int uart_init()
{
    // Enable clock for UART0.
    // Clock register for UARTs found in the SIM_SCGC4 (System Clock Gating
    // Control) register. UART0 clock is at bit 10.
    SIM->SCGC4 |= SIM_SCGC4_UART0_MASK; // mask is 0x400 = bit 10

    // Set source for baud rate generator clock for UART0 as FLL (41.94 MHz).
    // FLL = Frequency Locked Loop (vs. PLL = Phase Locked Loop)
    SIM->SOPT2 |= 0x4000000; // set bit 27-26 to 01 to select clock source
    SIM->SOPT2 |= SIM_SOPT2_PLLFLLSEL_MASK; // set bit 16 to 0 to select FLL

    // Turn off UART0 before making configuration changes.
    UART0->C2 = 0; // clear the C2 register

    // Set baud rate for UART0. Baud Rate = Clock Source / (OSR + 1) * SBR
    // SBR = concat of UART0_BDH and UART0_BDL
    // * Confused about this!
    UART0->BDH = 0x00;
    UART0->BDL = 0x17; // SBR of 0x17 corresponds to baud rate of 115,200

    // Set Over Sampling Ratio value to 16 (0x0F) for receiver.
    // * Should only be changed when transmitter and receiver are both disabled.
    UART0->C4 = 0x0F;

    // Set control register value: no parity (bit 1), 8-bit data size and 1 stop
    // bit (bit 4).
    UART0->C1 = 0x00;

    // Enable the transmitter for UART0.
    UART0->C2 = 0x08; // bit 4 is for TE (transmit enable)

// Enable the receiver for UART0.
//UART->C2 = 0x04; // bit 3 is for RE (receive enable)

    // Enable clock for PORTA.
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK; // mask is 0x200 = bit 9

    // Select alt func 2 (ALT2) (bits 10-8: MUX = 010) for PA2 (UART0_Tx) pins.
    PORTA->PCR[2] |= 0x0200;

// Select alt func 2 (ALT2) (bits 10-8: MUX = 010) for PA1 (UART0_Tx) pins.
//PORTA->PCR[1] |= 0x0200;
}

int uart_can_transmit()
{
    // Method 1
    // The TC (Transmit Complete) flag (bit 6):
    // 0 = Transmission in porgress (shift reg occupied)
    // 1 = No transmission in progress (both shift reg and data reg empty)
    //if ((UART0->S1 & 0x40) == 0)
    // * Shouldn't I use Method 1?

    // Method 2
    // The TDRE (Transmit Data Register Empty) flag (bit 7):
    // 0 = Shift register is loaded and shifting. An additional byte is waiting
    //     in the Data Register.
    // 1 = The Data Register is empty and ready for the next byte.
    if ((UART0->S1 & 0x80) == 0)
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
    if (uart_can_transmit)
    {
        // Transmit character (writing to this reg initiates a transmission from
        // the uart).
        UART0_D = c;
    }
}

void uart_transmit_blocking(char c)
{
    // Wait for transmit buffer to be ready.
    while (!uart_can_transmit) {}

    // Transmit character.
    uart_transmit(c);
}

int uart_can_receive()
{
    // The RDRF (Receive Data Register Full) flag (bit 5):
    // 0 = No data available in uart data reg.
    // 1 = Data available in uart data reg and ready to be picked up.
    if ((UART0->S1 & 0x20) == 0)
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
    if (uart_can_receive())
    {
        // Get character.
        return UART0_D;
    }
}

char uart_receive_blocking()
{
    // Wait for receive buffer to be ready.
    while (!uart_can_receive()) {}

    return uart_receive();
}
