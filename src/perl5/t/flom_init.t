#!perl -T
use strict;
use warnings FATAL => 'all';

use Flom;
use Test::More tests => 2;

my $handle;

ok( $handle = Flom::flom_handle_t->new );
ok( $handle->isa('Flom::flom_handle_t') );

