#!/usr/bin/perl
#
# File: transactional.pl
# By  : Kevin L. Esteb
# Date: 15-Nov-2016
#
# a port of transactional.c
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

# step 3a: set a different (non default) resource name to lock

if (($rc = Flom::handle_set_resource_name($handle, '_S_transact[1]')) != Flom::RC_OK) {

    printf("Flom::handle_set_resource_name() returned %d, %s\n", $rc, Flom::strerror($rc));
    goto fini;

}

# step 3b: set a different (non default) resource idle lifespan 

if (($rc = Flom::handle_set_resource_idle_lifespan($handle, 60000)) != Flom::RC_OK) {

    printf("Flom::handle_set_resource_idle_lifespan() returned %d, %s\n", $rc, Flom::strerror($rc));
    goto fini;

}
    
# step 4: lock acquisition 

if (($rc = Flom::handle_lock($handle)) != Flom::RC_OK) {

    printf("Flom::handle_lock() returned %d, %s\n", $rc, Flom::strerror($rc));
    goto fini;

} elsif ($rc = Flom::handle_get_locked_element($handle)) {

    printf("Flom::_handle_get_locked_element(): '%s' (first lock)\n",
        Flom::handle_get_locked_element($handle));

}    

# step 5: lock release 

if (($rc = Flom::handle_unlock_rollback($handle)) != Flom::RC_OK) {

    printf("Flom::handle_unlock_rollback() returned %d, '%s'\n", $rc, Flom::strerror($rc));
    goto fini;

}

# step 6: lock acquisition 

if (($rc = Flom::handle_lock($handle)) != Flom::RC_OK) {

    printf("Flom::handle_lock() returned %d, %s\n", $rc, Flom::strerror($rc));
    goto fini;

} elsif ($rc = Flom::handle_get_locked_element($handle)) {
    
    printf("Flom::_handle_get_locked_element(): '%s' (second lock)\n",
        Flom::handle_get_locked_element($handle));

}    

# step 7: lock release 

if (($rc = Flom::handle_unlock($handle)) != Flom::RC_OK) {

    printf("Flom::handle_unlock() returned %d, '%s'\n", $rc, Flom::strerror($rc));
    goto fini;

}

# step 8: lock acquisition 

if (($rc = Flom::handle_lock($handle)) != Flom::RC_OK) {

    printf("Flom::handle_lock() returned %d, %s\n", $rc, Flom::strerror($rc));
    goto fini;

} elsif ($rc = Flom::handle_get_locked_element($handle)) {

    printf("Flom::_handle_get_locked_element(): '%s' (third lock)\n",
        Flom::handle_get_locked_element($handle));

}

# step 9: sleep 5 seconds to allow program killing

printf("The program is waiting 5 seconds: kill it with the [control]+[c] "
    "keystroke and restart it to verify resource rollback...\n");
sleep(5);

# step 10: lock release 

if (($rc = Flom::handle_unlock($handle)) != Flom::RC_OK) {

    printf("Flom::handle_unlock() returned %d, '%s'\n", $rc, Flom::strerror($rc));
    goto fini;

}

# step 11: delete the handle 

fini:
  Flom::handle_delete($handle);

