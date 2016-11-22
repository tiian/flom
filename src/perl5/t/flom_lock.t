#!perl
use strict;
use warnings FATAL => 'all';

use Flom;
use Test::More;

unless ( $ENV{RELEASE_TESTING} ) {

    plan( skip_all => "Author tests not required for installation" );

} else {

    plan tests => 6;

}

# starting up flom - making some assumptions here

system('pkill flom');
system('flom -d -1 -- true');

my $handle;

ok( $handle = Flom::flom_handle_t->new );
ok( $handle->isa('Flom::flom_handle_t') );

ok( Flom::handle_init($handle) == Flom::RC_OK );
ok( Flom::handle_lock($handle) == Flom::RC_OK );
ok( Flom::handle_unlock($handle) == Flom::RC_OK );
ok( Flom::handle_clean($handle) == Flom::RC_OK );

system('pkill flom');

