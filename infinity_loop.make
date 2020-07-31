INFINITY_LOOP_C_FLAGS=-O2 -Wall -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winline -Wlong-long -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wno-import -Wpointer-arith -Wredundant-decls -Wshadow -Wstrict-prototypes -Wwrite-strings
INFINITY_LOOP_OBJS=infinity_loop.o mtrand.o mp_utils.o

infinity_loop: ${INFINITY_LOOP_OBJS}
	gcc -o infinity_loop ${INFINITY_LOOP_OBJS}

infinity_loop.o: infinity_loop.c infinity_loop.make
	gcc -c ${INFINITY_LOOP_C_FLAGS} -o infinity_loop.o infinity_loop.c

mtrand.o: mtrand.h mtrand.c infinity_loop.make
	gcc -c ${INFINITY_LOOP_C_FLAGS} -o mtrand.o mtrand.c

mp_utils.o: mp_utils.h mp_utils.c infinity_loop.make
	gcc -c ${INFINITY_LOOP_C_FLAGS} -o mp_utils.o mp_utils.c

clean:
	rm -f infinity_loop ${INFINITY_LOOP_OBJS}
