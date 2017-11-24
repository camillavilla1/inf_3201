#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION

#include <Python.h>
#include <numpy/arrayobject.h>
#include <stdio.h>

char solve(float, float);

static PyObject *mandelbrot(PyObject *self, PyObject *args) {
	PyArrayObject *result;
	PyArrayObject *x_coords;
	PyArrayObject *y_coords;
	unsigned int size;
	int x,y;
	if(!PyArg_ParseTuple(args, "O!(O!O!)I", &PyArray_Type, &result, &PyArray_Type, &x_coords, &PyArray_Type, &y_coords, &size))
		return NULL;

#pragma omp parallel for private(y)
	for(x = 0; x < size; x++) {
		for(y = 0; y < size; y++) {
			*(char *)(PyArray_GETPTR2(result, y, x)) = solve(*(float *)(PyArray_GETPTR1(x_coords, x)), *(float *)(PyArray_GETPTR1(y_coords, y)));
		}
	}
	return Py_BuildValue("i",1);
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

static PyMethodDef mandel_methods[] = {
	{"mandelc", mandelbrot, METH_VARARGS},
	{NULL, NULL}
};

void initmandelc(void)
{
	(void) Py_InitModule("mandelc", mandel_methods);
	import_array();
}
