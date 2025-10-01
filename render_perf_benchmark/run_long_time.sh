#!/bin/bash
set -e

BIN=./render_perf_benchmark_gl

run_test() {
    local name=$1
    shift
    echo "=== $name ==="
    $BIN "$@"
}

# ALU bound
run_test "alu" \
  --geom fullscreen \
  --res 2048x2048 \
  --verts 1000000 \
  --frag-ops 65536 \
  --frag-fetches 0 \
  --frames 10000

