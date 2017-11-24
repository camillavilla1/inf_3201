import pycuda.autoinit
import pycuda.autoinit
import pycuda.driver as drv
import pycuda.gpuarray as gpuarray
import pycuda.compiler
import numpy as np
import hashlib

# Generate the source module
f = open("cuda_kernel.cu", 'r')
# lineinfo used to enable assembly profiling in nvvp
sm = pycuda.compiler.SourceModule(f.read(), options=['-lineinfo'])

# Get function pointers from the source module
func1 = sm.get_function("decrypt_bytes")
func2 = sm.get_function("reconstruct_secret")


def decrypt_bytes(bytes_in, key):
	#bytes_in is pointer to data

	size = 512;
	ha = np.fromstring(hashlib.md5(key).digest(), np.uint32)

	#Return a new array with the same shape and type as a given array.
	result = np.empty_like(bytes_in)


	# Copy data to and from the GPU, and call the function on it
	func1(drv.InOut(result), drv.In(bytes_in), drv.In(ha), block=(size,1,1), grid=(10,1,1))

	return result


def reconstruct_secret(secret):
	#secret -- the shuffled numpy array
	size = len(secret)

	#Return a new array with the same shape and type as a given array.
	result = np.empty_like(secret.astype(np.uint8))

	func2(drv.InOut(result), drv.In(secret), block=(1024,1,1), grid=(10,1,1))

	

	return result
