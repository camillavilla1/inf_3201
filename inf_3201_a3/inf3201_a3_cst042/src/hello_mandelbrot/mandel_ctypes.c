char solve(float, float);

int mandelbrot(char * result, float * x_coords, float * y_coords, unsigned int size) {
	int x,y;
#pragma omp parallel for private(y)
	for(x = 0; x < size; x++) {
		for(y = 0; y < size; y++) {
			result[x + (y*size)] = solve(x_coords[x], y_coords[y]);
		}
	}
	return 1;
}

char solve(float x, float y) {
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
