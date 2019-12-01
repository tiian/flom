#!/bin/sh
#
# Copyright (c) 2013-2019, Christian Ferrari <tiian@users.sourceforge.net>
# All rights reserved.
#
# This file is part of FLOM.
#
# FLOM is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as published
# by the Free Software Foundation.
#
# FLOM is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with FLOM.  If not, see <http://www.gnu.org/licenses/>.
#

# retrieving unique id
UNIQUE_ID=$(flom --unique-id)
echo "FLoM unique ID is $UNIQUE_ID"
echo "Creating certification authorities and certificates"

create_ca () {
	echo "Creating CA $CA"
	mkdir -v ${CA} || exit $?
	cp -v $ETCPATH/flom_openssl.conf ${CA} || exit $?
	# entering first CA directory
	cd $CA || exit $?
	mkdir certs crl newcerts private || exit $?
	echo "01" > serial || exit $?
	cp /dev/null index.txt || exit $?
	echo "unique_subject = yes" > index.txt.attr || exit $?
	# creating CA certificate
	openssl req -new -x509 -keyout private/cakey.pem -out cacert.pem -days 365 -config flom_openssl.conf -subj "/C=IT/ST=TV/L=Mogliano Veneto/O=FLoM Software/OU=R and D/CN=FLoM $CA" -passout pass:flom$CA || exit $?
}

create_cert() {
	echo "Creating certificate for peer $PEER"
	# creating peers' certificates
	openssl req -nodes -new -x509 -keyout ${PEER}_${CA}_key.pem -out ${PEER}_${CA}_req.pem -days 365 -config flom_openssl.conf -subj "/C=IT/ST=TV/L=Mogliano Veneto/O=FLoM Software/OU=${PEER}/CN=${UNIQUE_ID}" || exit $?
	openssl x509 -x509toreq -in ${PEER}_${CA}_req.pem -signkey ${PEER}_${CA}_key.pem -out tmp.pem || exit $?
	openssl ca -batch -config flom_openssl.conf -policy policy_anything -passin pass:flom${CA} -out ${PEER}_${CA}_cert.pem -infiles tmp.pem || exit $?
	rm tmp.pem || exit $?
}

CA=CA1
create_ca || exit $?
PEER=peer1
create_cert || exit $?
PEER=peer2
create_cert || exit $?
# wrong UNIQUE_ID
TRUE_UNIQUE_ID=${UNIQUE_ID}
UNIQUE_ID=00112233445566778899aabbccddeeff
PEER=peer3
create_cert || exit $?
UNIQUE_ID=${TRUE_UNIQUE_ID}

cd ..

CA=CA2
create_ca || exit $?
PEER=peer1
create_cert || exit $?
PEER=peer2
create_cert || exit $?

cd ..

echo "Certification authorities and certificates completed!"
