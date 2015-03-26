#!/usr/bin/env python

"""
setup.py file for SWIG generated module
"""

from distutils.core import setup, Extension

flom_module = Extension('_flom',
	include_dirs=['..'],
	sources=['flom_wrap.c'],
	library_dirs=['../.libs'],
	runtime_library_dirs=['/usr/local/lib'],
	libraries=['flom','glib-2.0','gthread-2.0']
	)

setup (name = 'flom',
	version = '0.1',
	author = "Christian Ferrari",
	description = """FLoM python module""",
	ext_modules = [flom_module],
	py_modules = ["flom"],
	)
