#!/bin/bash
set -e

BIN=./render_perf_benchmark
OUT=results.csv

echo "test,resolution,verts,frag_ops,frag_fetches,texels,frames,avg_ms,fps,mverts_per_sec,gtexels_per_sec" > $OUT

run_test() {
    local name=$1; shift
    local args="$@"
    echo "=== $name ==="

    # Run benchmark, echo raw output to terminal, capture to tmp
    $BIN $args | tee tmp.out

    avg_ms=$(grep "Avg frame" tmp.out | awk '{print $3}')
    fps=$(grep "Avg frame" tmp.out | awk -F'[()]' '{print $2}' | awk '{print $1}')
    mverts=$(grep "Mverts/sec" tmp.out | awk '{print $2}')
    gtexels=$(grep "Tex fetch rate" tmp.out | awk '{print $3}')

    res=$(echo $args | sed -n 's/.*--res \([0-9]*x[0-9]*\).*/\1/p')
    verts=$(echo $args | sed -n 's/.*--verts \([0-9]*\).*/\1/p')
    frag_ops=$(echo $args | sed -n 's/.*--frag-ops \([0-9]*\).*/\1/p')
    frag_fetches=$(echo $args | sed -n 's/.*--frag-fetches \([0-9]*\).*/\1/p')
    texels=$(echo $args | sed -n 's/.*--texels \([0-9]*\).*/\1/p')
    frames=$(echo $args | sed -n 's/.*--frames \([0-9]*\).*/\1/p')

    echo "$name,$res,$verts,$frag_ops,$frag_fetches,$texels,$frames,$avg_ms,$fps,$mverts,$gtexels" >> $OUT
}

run_test "vertex"  --res 4096x4096 --verts 5000000 --frag-ops 0    --frag-fetches 0  --frames 200
run_test "alu"     --res 2048x2048 --verts 2000000 --frag-ops 2000 --frag-fetches 0  --frames 100
run_test "texture" --res 4096x4096 --verts 5000000 --frag-ops 0    --frag-fetches 64 --texels 1024000000 --frames 50
run_test "mixed"   --res 2048x2048 --verts 2000000 --frag-ops 500  --frag-fetches 16 --texels 512000000  --frames 100

echo "Results saved to $OUT"
rm -f tmp.out
