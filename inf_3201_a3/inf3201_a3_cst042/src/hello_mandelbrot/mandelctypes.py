import numpy as np
import ctypes

def mandelbrot(array, coords, size):
    # Load the library from disk
    mandelctypes = ctypes.CDLL("./libmandel.so")
    # Call the function in the library, using the ctypes part of the numpy library for compatability
    mandelctypes.mandelbrot(array.ctypes.data_as(ctypes.POINTER(ctypes.c_char)),
                            coords[0].ctypes.data_as(ctypes.POINTER(ctypes.c_float)),
                            coords[1].ctypes.data_as(ctypes.POINTER(ctypes.c_float)),
                            ctypes.c_uint32(size))
