CC = gcc
INC = .
CFLAG = -g -Wall 
obj = main.o rs232.o

all: myuart
myuart: $(obj)
	$(CC) -o myuart $(obj)
main.o: main.c
	$(CC) -I$(INC) $(CFLAG) -c main.c
rs232.o: rs232.c rs232.h
	$(CC) -I$(INC) $(CFLAG) -c rs232.c
clean:
	rm myuart $(obj)
