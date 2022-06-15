CC      = gcc
CFLAGS  = -Wall -Wextra -Werror -std=c99 -O3 $(shell sdl-config --libs) 
LDFLAGS = -O3 -export-dynamic $(shell sdl-config --libs) -lSDL_image
TARGETS = qrcode

$(TARGETS): main.o pbm.o decoder.o rs.o bch.o blocks.o modules.o data.o image2qr.o
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o

destroy: clean
	rm -f $(TARGETS)

rebuild: destroy $(TARGETS)
