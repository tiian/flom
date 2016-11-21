#!/usr/bin/perl
#
# File: advance.pl
# By  : Kevin L. Esteb
# Date: 15-Nov-2016
#
# a port of advance_dynamic.c
#
# usage:
#    flom -s /tmp/my_socket_name -d -1 -- true
#    export FLOM_TRACE_MASK=0x80000
#    ./advance.pl
#

use strict;
use warnings;

use Flom;

my $rc;
my $handle;

# step 2: new handle creation

unless ($handle = Flom::handle_new) {

    printf("Flom::handle_new() returned %p\n", $handle);
    exit 1;

}

# step 3a: set a different AF_UNIX/PF_LOCAL socket to connect to FLoM

if (($rc = Flom::handle_set_socket_name($handle, '/tmp/my_socket_name')) != Flom::RC_OK) {

    printf("flom_handle_set_socket_name() returned %s, %s\n", $rc, Flom::strerror($rc));
    goto fini;

}

# step 3b: set a different (non default) resource name to lock

if (($rc = Flom::handle_set_resource_name($handle, 'Red.Blue.Green')) != Flom::RC_OK) {
    
    printf("Flom::handle_set_resource_name() returned %s, %s\n", $rc, Flom::strerror($rc));
    goto fini;
    
}

# step 4: lock acquistion

if (($rc = Flom::handle_lock($handle)) != Flom::RC_OK) {
    
    printf("Flom::handle_lock() returned %s, %s\n", $rc, Flom::strerror($rc));
    goto fini;

} elsif ($rc = Flom::handle_get_locked_element($handle)) {

    printf("Flom::handle_get_locked_element(): %s\n", 
        Flom::handle_get_locked_element($handle));

}

# step 5: execute the code that needs lock protection

sleep(10);

# step 6: lock release

if (($rc = Flom::handle_unlock($handle)) != Flom::RC_OK) {
    
    printf("Flom::handle_unlock() returned %s, %s\n", $rc, Flom::strerror($rc));
    goto fini;

}

fini:
  Flom::handle_delete($handle);

