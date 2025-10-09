#!/bin/bash
set -euo pipefail

GPU_IDX=${1:-0}
OUTDIR="bench_with_metrics_$(date +%Y%m%d_%H%M%S)_gpu${GPU_IDX}"
mkdir -p "$OUTDIR"

echo "[*] Starting benchmark + telemetry on GPU $GPU_IDX. Results -> $OUTDIR"

#####################################
# Start monitoring
#####################################
echo "[*] Launching nvidia-smi dmon on GPU $GPU_IDX..."
nvidia-smi dmon -i $GPU_IDX -s pucvmt -d 1 -o DT > "$OUTDIR/nvidia_dmon.log" &
DMON_PID=$!

# Snapshot before
nvidia-smi -i $GPU_IDX -q -d CLOCK,POWER,PERFORMANCE,UTILIZATION > "$OUTDIR/nvidia_smi_start.log"

#####################################
# Idle period before benchmark
#####################################
IDLE_BEFORE=5   # seconds
echo "[*] Waiting $IDLE_BEFORE s to record idle GPU baseline..."
sleep $IDLE_BEFORE

#####################################
# Run benchmark
#####################################
./run_render_perf_benchmarks.sh | tee "$OUTDIR/benchmark.log"

#####################################
# Idle period after benchmark
#####################################
IDLE_AFTER=5   # seconds
echo "[*] Waiting $IDLE_AFTER s to capture post-benchmark idle period..."
sleep $IDLE_AFTER

#####################################
# Stop monitoring
#####################################
kill $DMON_PID 2>/dev/null || true
wait $DMON_PID 2>/dev/null || true

# Snapshot after
nvidia-smi -i $GPU_IDX -q -d CLOCK,POWER,PERFORMANCE,UTILIZATION > "$OUTDIR/nvidia_smi_end.log"

echo "[*] Benchmark and metrics complete."

#####################################
# Summarize results
#####################################
echo
echo "=== Benchmark summary (from $OUTDIR/benchmark.log) ==="
grep -E "Avg frame|Throughput|Tex fetch rate" "$OUTDIR/benchmark.log" || true

echo
echo "=== Clocks summary (from start snapshot) ==="
grep -A2 "Clocks" "$OUTDIR/nvidia_smi_start.log" || true

echo
echo "=== dmon raw samples (GPU $GPU_IDX) ==="
echo "[*] Full log is in $OUTDIR/nvidia_dmon.log"
echo "[*] Showing first 10 and last 10 lines:"
(head -n 15 "$OUTDIR/nvidia_dmon.log"; echo "..."; tail -n 10 "$OUTDIR/nvidia_dmon.log") || true

echo
echo "[*] Detailed logs are in $OUTDIR/"
