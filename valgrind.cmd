#!/bin/sh
# use this trivial script to check memory behavior
export G_SLICE=always-malloc
valgrind --leak-check=full src/flom --trace-file=/tmp/trace -- ls >/tmp/stdout 2>/tmp/stderr
