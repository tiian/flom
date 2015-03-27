#!/usr/bin/env python

"""
setup.py file for SWIG generated module
"""

from distutils.core import setup, Extension

flom_module = Extension('_@_FLOM_PREFIX@',
	include_dirs=['..'],
	sources=['flom_wrap.c'],
	library_dirs=['../.libs'],
	runtime_library_dirs=['@libdir@'],
	libraries=['@_FLOM_PREFIX@','@GTHREAD2_PYTHON@','@GLIB2_PYTHON@']
	)

setup (name = '@_FLOM_PREFIX@',
	version = '@PACKAGE_VERSION@',
	author = 'Christian Ferrari',
        author_email = '@PACKAGE_BUGREPORT@',
	description = """@PACKAGE_NAME@ python module""",
	ext_modules = [@_FLOM_PREFIX@_module],
	py_modules = ['@_FLOM_PREFIX@'],
	)
