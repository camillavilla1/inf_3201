__device__ char solve(float, float);


// Global function, visible from the CPU code
__global__ void mandelbrot(char *result, float *x, float *y, int size) {
	// Getting the thread ID
	const int tx = threadIdx.x + (blockIdx.x * blockDim.x);

	// Calculating the X and Y pixel coordinates, wouldn't need to do this if the kernel was invoked with a 2D grid of threads
	int ax = tx % size;
	int ay = (tx - ax) / size;

	result[tx] = solve(x[ax], y[ay]);
}

// Device function, only callable from device code
__device__ char solve(float x, float y) {
	double r=0.0,s=0.0;
	double next_r,next_s;
	int itt=0;

	while((r*r+s*s)<=4.0) {
		next_r=r*r-s*s+x;
		next_s=2*r*s+y;
		r=next_r; s=next_s;
		if(++itt==255)break;
	}	    
	return itt;
}
