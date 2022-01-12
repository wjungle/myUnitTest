CC = gcc
INC = .
CFLAG = -g -Wall 
obj = thread.o rs232.o common_layer_model.o

all: myuart
myuart: $(obj)
	$(CC) -o myuart $(obj)
thread.o: thread.c 
	$(CC) -I$(INC) $(CFLAG) -c thread.c
rs232.o: rs232.c rs232.h
	$(CC) -I$(INC) $(CFLAG) -c rs232.c
common_layer_model.o: common_layer_model.c canbus_data.h
	$(CC) -I$(INC) $(CFLAG) -c common_layer_model.c
clean:
	rm myuart $(obj)
