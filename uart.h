#ifndef __UART_H
#define __UART_H

int uart_init();
int uart_can_transmit();
void uart_transmit(char c);
void uart_transmit_blocking(char c);
int uart_can_receive();
char uart_receive();
char uart_receive_blocking();

#endif
