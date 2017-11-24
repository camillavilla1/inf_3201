CC = gcc

# Remove -DGRAPHICS to compile with no graphic support (e.g. for measurements)
CFLAGS = -std=c99 -O2 -Wall #-DGRAPHICS
RRLIB = ../lib

# Use this for SDL (change the path to sdl-config to the correct
# location if you're not running on rocks)
INC = -I$(RRLIB) `sdl-config --cflags`
LIB = -L$(RRLIB) -L/usr/X11R6/lib64 -L/usr/X11R6/lib -lX11 -lgraphicsscreen  `sdl-config --libs` #-pg -lprofiler

all: FrostTrap 

FrostTrap: FrostTrap.c
	$(CC) $(CFLAGS) -fopenmp -W FrostTrap.c -o FrostTrap $(INC) $(LIB)


clean:
	rm -f *.o *~ core* FrostTrap
