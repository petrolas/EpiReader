# change application name here (executable output name)
TARGET=main

# compiler
CC=gcc
# debug
DEBUG=-g
# optimisation
OPT=-O0

PTHREAD=-pthread

CCFLAGS=$(DEBUG) $(OPT)  $(PTHREAD) -pipe

GTKLIB=`pkg-config --cflags --libs gtk+-3.0`

# linker
LD=gcc
LDFLAGS=$(PTHREAD) $(GTKLIB) -export-dynamic -lm

OBJS=main.o

all: $(OBJS) 
	$(LD) -o $(TARGET) $(OBJS) $(LDFLAGS)
    
main.o: test2.c
	$(CC) -c $(CCFLAGS) test2.c $(GTKLIB) -o main.o


clean:
	rm main *.o
