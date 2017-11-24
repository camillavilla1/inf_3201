import mandelc

def mandelbrot(array, coords, size):
    # Calls the mandelbrot calculation in the C library
    mandelc.mandelc(array, coords, size)
