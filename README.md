
# egl_test.cpp

`gcc egl_test.cpp -o egl_test -lEGL`


#

```
g++ render_perf_benchmark.cpp glad/src/gl.c -o render_perf_benchmark \
    -Iinclude -lEGL -lGL -ldl -lpthread

# verify you see a circle rendered
./render_perf_benchmark --dump render_perf_benchmark.ppm

# vertex throughput test
./render_perf_benchmark --verts 10000000 --frag-ops 0 --frag-fetches 0

# fragment ALU test
./render_perf_benchmark --verts 1000000 --frag-ops 200 --frag-fetches 0

# texture bandwidth test
./render_perf_benchmark --verts 1000000 --frag-ops 0 --frag-fetches 16 --texels 512000000

# mixed / balanced load
./render_perf_benchmark --verts 2000000 --frag-ops 50 --frag-fetches 4 --texels 128000000

# debug build
g++ -g -O0 -DDEBUG render_perf_benchmark.cpp glad/src/gl.c \
    -Iinclude -lEGL -lGL -ldl -lpthread -o render_perf_benchmark
```