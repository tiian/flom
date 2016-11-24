#!/usr/bin/perl
#
#

use strict;
use warnings;

use Flom;

my $handle;
my $username = $ENV{'USER'};

$handle = Flom::flom_handle_t->new;
Flom::handle_init($handle);

printf("get_socket_name:            %s\n", Flom::handle_get_socket_name($handle));
printf("get_trace_name:             %s\n", Flom::handle_get_trace_filename($handle) || '');
printf("get_resource_name:          %s\n", Flom::handle_get_resource_name($handle));
printf("get_resource_create:        %s\n", Flom::handle_get_resource_create($handle));
printf("get_resource_timeout:       %s\n", Flom::handle_get_resource_timeout($handle));
printf("get_resouce_quantity:       %s\n", Flom::handle_get_resource_quantity($handle));
printf("get_lock_mode:              %s\n", Flom::handle_get_lock_mode($handle));
printf("get_resource_idle_lifespan: %s\n", Flom::handle_get_resource_idle_lifespan($handle));
printf("get_unicast_address:        %s\n", Flom::handle_get_unicast_address($handle) || '');
printf("get_multicast_address:      %s\n", Flom::handle_get_multicast_address($handle) || '');
printf("get_unicast_port:           %s\n", Flom::handle_get_unicast_port($handle));
printf("get_multicast_port:         %s\n", Flom::handle_get_multicast_port($handle));
printf("get_discovery_attempts:     %s\n", Flom::handle_get_discovery_attempts($handle));
printf("get_discovery_ttl:          %s\n", Flom::handle_get_discovery_ttl($handle));
printf("get_tls_check_peer_id:      %s\n", Flom::handle_get_tls_check_peer_id($handle));

