#!/usr/bin/perl
#
# File: basic.pl
# By  : Kevin L. Esteb
# Date: 15-Nov-2016
#
# a port of basic_dynamic.c
#
# usage:
#    flom -d -1 -- true
#    export FLOM_TRACE_MASK=0x80000
#    ./basic.pl
#

use strict;
use warnings;

use Flom;

my $rc;
my $handle;

# step 2: new handle creation

unless ($handle = Flom::handle_new()) {

    printf("Flom::handle_new() returned %p\n", $handle);
    exit 1;

}

# step 3: lock acquisition

if (($rc = Flom::handle_lock($handle)) != Flom::RC_OK) {

    printf("Flom::handle_lock() returned %d, %s\n", $rc, Flom::strerror($rc));
    goto fini;

}

# step 4: execute the code that needs lock protection

sleep(10);

# step 5: lock release

if (($rc = Flom::handle_unlock($handle)) != Flom::RC_OK) {

    printf("Flom::handle_unlock() returned %d, %s\n", $rc, Flom::strerror($rc));
    goto fini;

}

# step 6: delete the lock

fini:
  Flom::handle_delete($handle);

