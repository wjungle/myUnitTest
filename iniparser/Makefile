CC = gcc
INC = .
CFLAG = -g -Wall 
obj = dictionary.o iniparser.o main.o

all: initest
initest: $(obj)
	$(CC) -o initest $(obj)
main.o: main.c
	$(CC) -I$(INC) $(CFLAG) -c main.c
dictionary.o: dictionary.c dictionary.h
	$(CC) -I$(INC) $(CFLAG) -c dictionary.c
iniparser.o: iniparser.c iniparser.h
	$(CC) -I$(INC) $(CFLAG) -c iniparser.c
clean:
	rm initest $(obj)
