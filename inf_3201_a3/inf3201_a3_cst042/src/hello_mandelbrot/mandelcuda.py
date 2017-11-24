import pycuda.autoinit
import pycuda.driver as drv
import pycuda.gpuarray as gpuarray
import pycuda.compiler
import numpy as np

# Generate the source module
f = open("cuda_kernel.cu", 'r')
# lineinfo used to enable assembly profiling in nvvp
sm = pycuda.compiler.SourceModule(f.read(), options=['-lineinfo'])

def mandelbrot(result, coords, size):
    # Get a function pointer from the source module
    func = sm.get_function("mandelbrot")
    # Reshaping for simplicity here, not really needed usually
    result = np.reshape(result, size*size)
    # Copy data to and from the GPU, and call the function on it
    # Grid and block size simplified here, probably easier to understand the code if it was 2D
    func(drv.InOut(result), drv.In(coords[0]), drv.In(coords[1]), np.int32(size), block=(64,1,1), grid=(size*size/64,1,1))
    # Reshaping the results
    result = np.reshape(result,(size,size))
