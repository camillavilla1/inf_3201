CC=gcc

all: pybuild cbuild


cbuild: setup.py
	python setup.py build -f
	python setup.py install --user

pybuild: mandel_ctypes.c
	gcc -c -fPIC -fopenmp mandel_ctypes.c -o mandel_ctypes.o
	gcc -shared -fopenmp -Wl,-soname,libmandel.so -o libmandel.so mandel_ctypes.o

clean:
	rm *.pyc
	rm -rf build
	rm libmandel.so
	rm mandel_ctypes.o
