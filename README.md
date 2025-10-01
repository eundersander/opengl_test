
# egl_test.cpp

`gcc egl_test.cpp -o egl_test -lEGL`


# render_perf_benchmark
```
cd render_perf_benchmark
make
./run_with_metrics.sh
```

# cuda_benchmark
```
cd cuda_benchmark
nvcc -O3 cuda_matmul.cu -o cuda_matmul