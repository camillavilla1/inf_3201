CC = mpicc
CCP = mpicc-vt
# Remove -DGRAPHICS to compile with no graphic support (e.g. for measurements)
CFLAGS = -O2 -Wall -g #-DGRAPHICS
RRDIR = ../lib

# Adapt this to your distribution if necessary
INCSDL = -I$(RRDIR) `sdl-config --cflags`
LIBSDL = -L/usr/X11R6/lib64 `sdl-config --libs`


INC = $(INCSDL) -I$(RRDIR) 
LIB = $(LIBSDL) -L$(RRDIR) -lX11 -lgraphicsscreen -lSDL -lStopWatch -lpthread -lm

all: RoadMap RoadMapProf RoadMapGProf RoadMapGPerf

RoadMap: RoadMap.c
	$(CC) $(CFLAGS) RoadMap.c -o RoadMap $(INC) $(LIB)

RoadMapProf: RoadMap.c
	$(CCP) $(CFLAGS) RoadMap.c -o RoadMapProf $(INC) $(LIB)

RoadMapGProf: RoadMap.c
	$(CC) $(CFLAGS) -pg RoadMap.c -o RoadMapGProf $(INC) $(LIB)

RoadMapGPerf: RoadMap.c
	        $(CC) $(CFLAGS) RoadMap.c -o RoadMapGPerf $(INC) $(LIB) -L/state/partition1/apps/lib -lprofiler

clean:
	rm -f RoadMap RoadMapProf* result* RoadMapGProf RoadMapGPerf gmon.* *~
