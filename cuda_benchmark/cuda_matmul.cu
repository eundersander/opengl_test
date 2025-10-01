#include <cuda_runtime.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>

#define CHECK_CUDA(call) \
    { cudaError_t err = call; \
      if (err != cudaSuccess) { \
          fprintf(stderr, "CUDA error at %s:%d: %s\n", \
                  __FILE__, __LINE__, cudaGetErrorString(err)); \
          exit(1); \
      } \
    }

#define TILE 32   // tile size; 32×32 is a good default on Ampere/Lovelace

__global__ void matmul_tiled(const float *A, const float *B, float *C, int N) {
    __shared__ float As[TILE][TILE];
    __shared__ float Bs[TILE][TILE];

    int row = blockIdx.y * TILE + threadIdx.y;
    int col = blockIdx.x * TILE + threadIdx.x;

    float val = 0.0f;

    // Loop over tiles
    for (int t = 0; t < (N + TILE - 1) / TILE; t++) {
        // Load a tile of A and B into shared memory
        int aRow = row;
        int aCol = t * TILE + threadIdx.x;
        int bRow = t * TILE + threadIdx.y;
        int bCol = col;

        As[threadIdx.y][threadIdx.x] = (aRow < N && aCol < N) ? A[aRow * N + aCol] : 0.0f;
        Bs[threadIdx.y][threadIdx.x] = (bRow < N && bCol < N) ? B[bRow * N + bCol] : 0.0f;

        __syncthreads();

        // Multiply the two tiles together
        for (int k = 0; k < TILE; k++) {
            val += As[threadIdx.y][k] * Bs[k][threadIdx.x];
        }

        __syncthreads();
    }

    // Write back result
    if (row < N && col < N) {
        C[row * N + col] = val;
    }
}

int main(int argc, char **argv) {
    int N = (argc > 1) ? atoi(argv[1]) : 4096;   // bigger than before
    int iters = (argc > 2) ? atoi(argv[2]) : 10;

    size_t bytes = (size_t)N * N * sizeof(float);
    float *hA = (float*)malloc(bytes);
    float *hB = (float*)malloc(bytes);
    float *hC = (float*)malloc(bytes);

    for (size_t i = 0; i < (size_t)N * N; i++) {
        hA[i] = (float)rand() / RAND_MAX;
        hB[i] = (float)rand() / RAND_MAX;
    }

    float *dA, *dB, *dC;
    CHECK_CUDA(cudaMalloc(&dA, bytes));
    CHECK_CUDA(cudaMalloc(&dB, bytes));
    CHECK_CUDA(cudaMalloc(&dC, bytes));

    CHECK_CUDA(cudaMemcpy(dA, hA, bytes, cudaMemcpyHostToDevice));
    CHECK_CUDA(cudaMemcpy(dB, hB, bytes, cudaMemcpyHostToDevice));

    dim3 threads(TILE, TILE);
    dim3 blocks((N + TILE - 1) / TILE, (N + TILE - 1) / TILE);

    printf("Running %d iterations of %dx%d tiled matmul (TILE=%d)...\n", iters, N, N, TILE);

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iters; i++) {
        matmul_tiled<<<blocks, threads>>>(dA, dB, dC, N);
        CHECK_CUDA(cudaDeviceSynchronize());
    }

    auto end = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(end - start).count();

    printf("Total time: %.2f ms\n", ms);
    printf("Average per iteration: %.2f ms\n", ms / iters);

    CHECK_CUDA(cudaMemcpy(hC, dC, bytes, cudaMemcpyDeviceToHost));

    // FLOP/s = 2*N^3 ops per multiply
    double gflops = (2.0 * (double)N * N * N * iters) / (ms / 1000.0) / 1e9;
    printf("Approx GFLOP/s: %.2f\n", gflops);

    cudaFree(dA); cudaFree(dB); cudaFree(dC);
    free(hA); free(hB); free(hC);
    return 0;
}
