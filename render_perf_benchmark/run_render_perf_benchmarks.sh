#!/bin/bash
set -e

BIN=./render_perf_benchmark_gl
OUT="results_$(basename "$BIN").csv"

echo "test,resolution,verts,frag_ops,frag_fetches,texels,frames,avg_ms,fps,mverts_per_sec,gtexels_per_sec,gflops,gbps" > "$OUT"

run_test() {
    local name=$1; shift
    local args="$@"
    echo "=== $name ==="

    # Run benchmark, echo raw output, capture to tmp
    $BIN $args | tee tmp.out

    # Parse numbers directly from C++ benchmark output
    avg_ms=$(grep "Avg frame" tmp.out | awk '{print $3}')
    fps=$(grep "Avg frame" tmp.out | awk -F'[()]' '{print $2}' | awk '{print $1}')
    mverts=$(grep "Vertex throughput" tmp.out | awk '{print $3}')
    gtexels=$(grep "Texture fetch rate" tmp.out | awk '{print $4}')
    gflops=$(grep "GFLOP/s" tmp.out | awk '{print $2}')
    gbps=$(grep "GB/s" tmp.out | awk '{print $2}')

    # Extract args for logging
    res=$(echo $args | sed -n 's/.*--res \([0-9]*x[0-9]*\).*/\1/p')
    verts=$(echo $args | sed -n 's/.*--verts \([0-9]*\).*/\1/p')
    frag_ops=$(echo $args | sed -n 's/.*--frag-ops \([0-9]*\).*/\1/p')
    frag_fetches=$(echo $args | sed -n 's/.*--frag-fetches \([0-9]*\).*/\1/p')
    texels=$(echo $args | sed -n 's/.*--texels \([0-9]*\).*/\1/p')
    frames=$(echo $args | sed -n 's/.*--frames \([0-9]*\).*/\1/p')

    echo "$name,$res,$verts,$frag_ops,$frag_fetches,$texels,$frames,$avg_ms,$fps,$mverts,$gtexels,$gflops,$gbps" >> "$OUT"
}

# Vertex throughput
run_test "vertex" \
  --geom sphere \
  --res 1024x1024 \
  --verts 50000000 \
  --frag-ops 0 \
  --frag-fetches 0 \
  --frames 260

# ALU bound
run_test "alu" \
  --geom fullscreen \
  --res 2048x2048 \
  --verts 1000000 \
  --frag-ops 65536 \
  --frag-fetches 0 \
  --frames 128

# Texture bound
run_test "texture" \
  --geom fullscreen \
  --res 4096x4096 \
  --verts 1000000 \
  --frag-ops 0 \
  --frag-fetches 16 \
  --texels 256000000 \
  --frames 340

# # Mixed
# run_test "mixed" \
#   --geom fullscreen \
#   --res 2048x2048 \
#   --verts 2000000 \
#   --frag-ops 64 \
#   --frag-fetches 8 \
#   --texels 128000000 \
#   --frames 17000

echo "Results saved to $OUT"
rm -f tmp.out
