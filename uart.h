#ifndef __UART_H
#define __UART_H

#include "ring.h"

// Constants.
#define RING_BUFF_LEN 256
#define NUM_SYMBOLS 256 // num ASCII chars
#define MAX_COUNT_DIGITS 3 // max number of places (digits) for char count
static const char *table_title = "\r\nCharacters\r\n";

// Global variables.
static int ascii[NUM_SYMBOLS]; // stores count of each ASCII char
static ring_t *ring_rx;
static ring_t *ring_tx;

// Functions.
void init_ascii_table();
void populate_tx_table_ring(char c);

void uart_init_buff();
void uart_init();
void uart_init_interrupt();
int uart_can_transmit();
void uart_transmit(char c);
void uart_transmit_blocking(char c);
int uart_can_receive();
char uart_receive();
char uart_receive_blocking();
void UART0_IRQHandler(void);

#endif

