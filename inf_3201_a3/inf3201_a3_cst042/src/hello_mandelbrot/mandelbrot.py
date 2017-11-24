#!/usr/bin/env python

SHOW_IMAGES=False
SIZE=1024

import numpy as np
if SHOW_IMAGES:
    from matplotlib import pyplot as plt
import time

import mandelc
import mandelcuda
import mandelctypes
import mandelc_wrapper

# Timer functions
def start_time():
    global TICK
    TICK = time.time()

def stop_time(prefix):
    global TICK
    old = TICK
    TICK = time.time()
    print(prefix + " " + str(TICK-old) + " seconds")

def main():
    print("Calculating a mandelbrot image using different implementations")

    # Creating arrays to store the results
    result_python = np.zeros((SIZE,SIZE), dtype=np.uint8)
    result_c = np.zeros_like(result_python)
    result_cuda = np.zeros_like(result_python)
    result_ctypes = np.zeros_like(result_python)

    # Generate coordinates in mandelbrot space
    x_c, y_c = generate_coords(result_python)

    start_time()

    # Python test
    #mandelbrot(result_python, (x_c, y_c), SIZE)
    stop_time("Python")

    # C extension test
    mandelc_wrapper.mandelbrot(result_c, (x_c, y_c), SIZE)
    stop_time("C extension")

    # Ctypes test
    mandelctypes.mandelbrot(result_ctypes, (x_c, y_c), SIZE)
    stop_time("ctypes")

    # Cuda test
    mandelcuda.mandelbrot(result_cuda, (x_c, y_c), SIZE)
    stop_time("Cuda")

    #Showing the images for verification
    if(SHOW_IMAGES):
        plt.imshow(result_cuda)
        plt.show()
        plt.imshow(result_python)
        plt.show()
        plt.imshow(result_c)
        plt.show()
        plt.imshow(result_ctypes)
        plt.show()


# Simple python implementation of the mandelbrot calculations
def mandelbrot(array, coords, size):
    for x in range(size):
        for y in range(size):
            array[y][x] = solve(coords[0][x], coords[1][y])

def solve(x, y):
    r = 0.0
    s = 0.0
    itt = 0

    while(r*r + s*s <= 4.0):
        itt += 1
        next_r = r * r - s * s + x
        next_s = 2 * r * s + y
        r = next_r
        s = next_s
        if(itt == 255):
            return itt
    return itt 

# Generates X and Y coordinates for the mandelbrot calculations
def generate_coords(dims):
    box_x_min=-1.5
    box_x_max=0.5
    box_y_min=-1.0
    box_y_max=1.0
    result_x = np.zeros(SIZE, dtype=np.float32)
    result_y = np.zeros(SIZE, dtype=np.float32)
    for i in range(SIZE):
        result_x[i] = (((box_x_max - box_x_min) / SIZE) * i) + box_x_min
        result_y[i] = (((box_y_max - box_y_min) / SIZE) * i) + box_y_min
    return (result_x, result_y)

if __name__ == "__main__":
    main()
