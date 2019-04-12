ifndef __UART_H
#define __UART_H

#include "ring.h"

#define RING_BUFF_LEN 1024

static ring_t *ring_rx;
static ring_t *ring_tx;

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
