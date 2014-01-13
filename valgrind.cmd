#!/bin/sh
# use this trivial script to check memory behavior
export G_SLICE=always-malloc
#valgrind --leak-check=full src/flom -r VALRES1 -t /tmp/valgrind-daemon.trace -T /tmp/valgrind-command.trace -- ls >/tmp/stdout 2>/tmp/stderr
valgrind --leak-check=full src/flom -- sleep 1 >/tmp/stdout 2>/tmp/stderr
