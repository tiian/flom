#!/bin/sh
# use this trivial script to check memory behavior
export G_SLICE=always-malloc
valgrind --leak-check=full src/flom -- ls >/dev/null 2>/tmp/foo
