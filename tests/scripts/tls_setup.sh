#!/bin/sh
echo "Creating certification authorities"
mkdir -v CA1 CA2 || exit $?
cp -v $ETCPATH/openssl.cnf CA1 || exit $?
cp -v $ETCPATH/openssl.cnf CA2 || exit $?
# entering first CA directory
cd CA1 || exit $?
mkdir certs crl newcerts private || exit $?
echo "01" > serial || exit $?
cp /dev/null index.txt || exit $?
openssl req -new -x509 -keyout private/cakey.pem -out cacert.pem -days 365 -config openssl.cnf -subj "/C=IT/ST=TV/L=Mogliano Veneto/O=FLoM Software/OU=R and D/CN=FLoM CA 1" -passout pass:flomca1 || exit $?
