

CC 		= gcc
CFLAGS 	= -O3 -I./src -lm -lpthread -lGL -lGLU -lglut
RM		= rm -f

default:	gnuplot_i.o

gnuplot_i.o: src/gnuplot_i.c src/gnuplot_i.h
	$(CC) $(CFLAGS) -c -o gnuplot_i.o src/gnuplot_i.c

tests:		test/anim test/example test/sinepng

main: 		test/main

test/anim:	test/anim.c gnuplot_i.o
	$(CC) $(CFLAGS) -o test/anim test/anim.c gnuplot_i.o

test/example:	test/example.c gnuplot_i.o
	$(CC) $(CFLAGS) -o test/example test/example.c gnuplot_i.o

test/sinepng:	test/sinepng.c gnuplot_i.o
	$(CC) $(CFLAGS) -o test/sinepng test/sinepng.c gnuplot_i.o

test/main: 	test/main.c gnuplot_i.o
	$(CC) -o test/main test/main.c gnuplot_i.o $(CFLAGS)

clean:
	$(RM) gnuplot_i.o test/anim test/example test/sinepng test/main

