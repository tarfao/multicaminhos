

CC 		= gcc
CFLAGS 	= -O3 -I./src -lm -lpthread -lGL -lGLU -lglut
RM		= rm -f

default:	gnuplot_i.o

gnuplot_i.o: src/gnuplot_i.c src/gnuplot_i.h
	$(CC) $(CFLAGS) -c -o gnuplot_i.o src/gnuplot_i.c

main: 		codigos/main

codigos/main: 	codigos/main.c gnuplot_i.o
	$(CC) -o codigos/main codigos/main.c gnuplot_i.o $(CFLAGS)

clean:
	$(RM) gnuplot_i.o codigos/main ./*.txt

