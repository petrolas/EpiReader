CFLAGS= -Wall -Wextra -Werror

all = reader

readerSoutenance:
	gcc -o readerSoutenance readerSoutenance.c

.PHONY: clean

clean: ${RM} readerSoutenance.o
	${RM} reader