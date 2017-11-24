#!/usr/bin/env python
from distutils.core import setup, Extension

setup(name="MandelbrotC",
        ext_modules=[Extension('mandelc', ['mandel_c.c'],
                                extra_compile_args=['-fopenmp'],
                                extra_link_args=['-fopenmp'])])
