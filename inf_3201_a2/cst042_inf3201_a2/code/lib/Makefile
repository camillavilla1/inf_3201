CC = gcc -O3

all: libgraphicsscreen.a

libgraphicsscreen.a: graphicsScreen.o
	ar cru libgraphicsscreen.a graphicsScreen.o

graphicsScreen.o: graphicsScreen.c
	$(CC) $(INC) -c graphicsScreen.c


clean:
	rm -f *.o *~ *.a
