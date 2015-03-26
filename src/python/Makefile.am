dist_noinst_DATA = flom.i

# Produce FLoM wrapper using SWIG
flom_wrap.c: flom.i ../flom_errors.h.in ../flom_types.h ../flom_handle.h
	$(SWIG) -python -I.. flom.i

# Build Python module extension
all-local:	flom_wrap.c setup.py
	python setup.py build_ext --inplace

clean-local:
	rm -f flom_wrap.c _flom.so flom.py

# Install Python module extension
install-exec-local:
	python setup.py install --record installed_files.txt

# Uninstall Python module extension
uninstall-local:
	cat installed_files.txt | xargs rm -rfv

