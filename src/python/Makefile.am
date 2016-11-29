dist_noinst_DATA = flom.i setup.py.i

# Produce FLoM wrapper using SWIG
flom_wrap.c: flom.i ../flom_errors.h.in ../flom_types.h ../flom_handle.h
	$(SWIG) -python -I.. flom.i

setup.py:	setup.py.i ../../config.status
	sed \
	-e 's|@libdir[@]|$(libdir)|g' \
        -e 's|@abs_top_builddir[@]|$(abs_top_builddir)|g' \
	-e 's|@_FLOM_PREFIX[@]|$(_FLOM_PREFIX)|g' \
	-e 's|@GLIB2_PYTHON[@]|$(GLIB2_PYTHON)|g' \
	-e 's|@GTHREAD2_PYTHON[@]|$(GTHREAD2_PYTHON)|g' \
	-e 's|@DBUS1_PYTHON[@]|$(DBUS1_PYTHON)|g' \
	-e 's|@OPENSSL_PYTHON[@]|$(OPENSSL_PYTHON)|g' \
	-e 's|@CRYPTO_PYTHON[@]|$(CRYPTO_PYTHON)|g' \
	-e 's|@PACKAGE_BUGREPORT[@]|$(PACKAGE_BUGREPORT)|g' \
	-e 's|@PACKAGE_NAME[@]|$(PACKAGE_NAME)|g' \
	-e 's|@PACKAGE_STRING[@]|$(PACKAGE_STRING)|g' \
	-e 's|@PACKAGE_VERSION[@]|$(PACKAGE_VERSION)|g' \
	$< >$@

# Build Python module extension
all-local:	flom_wrap.c setup.py
	python setup.py build_ext --inplace

clean-local:
	rm -f flom_wrap.c _flom.so flom.py setup.py

# Install Python module extension
install-exec-local:
	python setup.py install --record installed_files.txt

# Uninstall Python module extension
uninstall-local:
	cat installed_files.txt | xargs rm -rfv

