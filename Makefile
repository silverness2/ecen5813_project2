CFLAGS = -Wall -Werror
LDFLAGS = -lm
UNIT_LDFLAGS = -lcunit
TARGET = main
TEST = unit_test
UART_TARGET = main_uart


# MAIN FOR UART

$(UART_TARGET): $(UART_TARGET).o uart.o
	gcc -o $(UART_TARGET) $(UART_TARGET).o ring.o $(LDFLAGS)

uart.o: uart.c uart.h
	gcc $(CFLAGS) -c uart.c -o uart.o


# UNIT TEST AND MAIN FOR RING BUFFER

$(TEST): ring_test.o ring.o
	gcc -o $(TEST) ring_test.o ring.o $(LDFLAGS) $(UNIT_LDFLAGS)

ring_test.o: ring_test.c ring.h
	gcc $(CFLAGS) -c ring_test.c -o ring_test.o

$(TARGET): $(TARGET).o ring.o
	gcc -o $(TARGET) $(TARGET).o ring.o $(LDFLAGS)

$(TARGET).o: $(TARGET).c ring.h
	gcc $(CFLAGS) -c $(TARGET).c -o $(TARGET).o

ring.o:	ring.c ring.h
	gcc $(CFLAGS) -c ring.c -o ring.o

# CLEAN FOR ALL

clean:
	rm -rf *.o $(TARGET) $(TEST) $(UART_TARGET)
