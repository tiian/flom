#!/bin/sh
# use this trivial script to check memory behavior
export G_SLICE=always-malloc
#valgrind --suppressions=valgrind.supp --leak-check=full src/flom -a 192.168.1.4 -r VALRES1 -t /tmp/valgrind-daemon.trace -T /tmp/valgrind-command.trace -- ls >/tmp/stdout 2>/tmp/stderr
#valgrind --suppressions=valgrind.supp --leak-check=full src/flom -a 192.168.1.4 -A 224.0.0.1 -t /tmp/valgrind-daemon.trace -T /tmp/valgrind-command.trace -- ls >/tmp/stdout 2>/tmp/stderr
#valgrind --suppressions=valgrind.supp --leak-check=full src/flom -- sleep 1 >/tmp/stdout 2>/tmp/stderr
#valgrind --suppressions=valgrind.supp --leak-check=full src/flom -r foo[3] -q 2 -- sleep 1 >/tmp/stdout 2>/tmp/stderr
#valgrind --suppressions=valgrind.supp --leak-check=full src/flom -r red.green.blue -- sleep 1 >/tmp/stdout 2>/tmp/stderr
valgrind --suppressions=valgrind.supp --leak-check=full src/flom -r /red/green/blue -- sleep 1 >/tmp/stdout 2>/tmp/stderr
#valgrind --suppressions=valgrind.supp --leak-check=full src/flom -e n -r /bar/foo -o 500 -- true >/tmp/stdout 2>/tmp/stderr
#valgrind --suppressions=valgrind.supp --leak-check=full src/flom -e n -r /bar/foo/goofy -o 500 -- true >>/tmp/stdout 2>>/tmp/stderr

