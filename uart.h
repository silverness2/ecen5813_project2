/*******************************************************************************
 *
 * Copyright (C) 2019 by Shilpi Gupta
 *
 ******************************************************************************/

/*
 * @file    uart.h
 * @brief   Library declarations for UART communication on the FRDM KL25Z MCU.
 * @version Project 2
 * @date    April 13, 2019
 */

#ifndef __UART_H
#define __UART_H

#include "ring.h"

// Constants.
#define RING_BUFF_LEN 256 // must be a power of 2
#define NUM_SYMBOLS 256 // num ASCII chars
#define MAX_COUNT_DIGITS 3 // max number of places (digits) for char count

// Declare static (global) variables.
extern const char *table_title;
extern const char *unique_title;
extern int ascii[NUM_SYMBOLS]; // stores count of each ASCII char
extern int num_unique_chars; // number of unique chars that have been received
extern ring_t *ring_rx;
extern ring_t *ring_tx;

// Functions.
void init_ascii_table();
void generate_tx_ring_report();
void generate_unique_chars_report();
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

