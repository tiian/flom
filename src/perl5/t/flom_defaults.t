#!perl -T
use strict;
use warnings FATAL => 'all';

use Flom;
use Test::More;

unless ( $ENV{RELEASE_TESTING} ) {

    plan( skip_all => "Author tests not required for installation" );

} else {

    plan tests => 18;

}

my $handle;
my $username = $ENV{'USER'};

ok( $handle = Flom::flom_handle_t->new );
ok( $handle->isa('Flom::flom_handle_t') );
ok( Flom::handle_init($handle) == Flom::RC_OK );

# check defaults

is( Flom::handle_get_socket_name($handle), "/tmp/flom-$username" );
is( Flom::handle_get_trace_filename($handle), undef );
is( Flom::handle_get_resource_name($handle), '_RESOURCE' );
is( Flom::handle_get_resource_create($handle), 1 );
is( Flom::handle_get_resource_timeout($handle), -1 );
is( Flom::handle_get_resource_quantity($handle), 1 );
is( Flom::handle_get_lock_mode($handle), 5 );
is( Flom::handle_get_resource_idle_lifespan($handle), 0 );
is( Flom::handle_get_unicast_address($handle), undef );
is( Flom::handle_get_multicast_address($handle), undef );
is( Flom::handle_get_unicast_port($handle), 28015 );
is( Flom::handle_get_multicast_port($handle), 28015 );
is( Flom::handle_get_discovery_attempts($handle), 2 );
is( Flom::handle_get_discovery_ttl($handle), 1 );
is( Flom::handle_get_tls_check_peer_id($handle), 0 );

