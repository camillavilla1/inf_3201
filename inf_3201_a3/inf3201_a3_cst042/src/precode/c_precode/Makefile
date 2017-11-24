CC=nvcc
CFLAGS= -O3 -D_FORCE_INLINES
SOURCES= decrypter.cu md5.cu
EXECNAME= run

all:
	$(CC) -o $(EXECNAME) $(SOURCES) $(CFLAGS)
clean:
	rm $(EXECNAME)