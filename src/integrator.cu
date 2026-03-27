/**
 * @file integrator.cu
 * @author Yassin Ahmad 
 * @brief CUDA, OpenMP, and Serial implementations of numerical integration.
 */

#include "integrator.hpp"
#include <chrono>
#include <omp.h>
#include <cuda_runtime.h>
#include <iostream>

// CUDA Kernel for Riemann Sum
__global__ void riemann_kernel(const DeviceToken* rpn, int rpn_size, double a, double dx, int n, double* block_results) {
    extern __shared__ double sdata[];
    int tid = threadIdx.x;
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    
    double sum = 0;
    // Each thread can handle multiple intervals if grid is smaller than n
    for (; i < n; i += blockDim.x * gridDim.x) {
        double x = a + (i + 0.5) * dx;
        sum += Parser::evaluate(rpn, rpn_size, x);
    }
    
    sdata[tid] = sum;
    __syncthreads();
    
    // Block-level reduction
    for (unsigned int s = blockDim.x / 2; s > 0; s >>= 1) {
        if (tid < s) {
            sdata[tid] += sdata[tid + s];
        }
        __syncthreads();
    }
    
    if (tid == 0) {
        atomicAdd(block_results, sdata[0]);
    }
}

// CUDA Kernel for Simpson Sum
__global__ void simpson_kernel(const DeviceToken* rpn, int rpn_size, double a, double dx, int n, double* block_results) {
    extern __shared__ double sdata[];
    int tid = threadIdx.x;
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    
    double sum = 0;
    // Each thread can handle multiple intervals
    for (; i <= n; i += blockDim.x * gridDim.x) {
        double x = a + i * dx;
        double fx = Parser::evaluate(rpn, rpn_size, x);
        if (i == 0 || i == n) sum += fx;
        else if (i % 2 == 1) sum += 4 * fx;
        else sum += 2 * fx;
    }
    
    sdata[tid] = sum;
    __syncthreads();
    
    for (unsigned int s = blockDim.x / 2; s > 0; s >>= 1) {
        if (tid < s) {
            sdata[tid] += sdata[tid + s];
        }
        __syncthreads();
    }
    
    if (tid == 0) {
        atomicAdd(block_results, sdata[0]);
    }
}

IntegrationResult Integrator::integrate_serial(const std::vector<DeviceToken>& rpn, double a, double b, int n, const std::string& method) {
    auto start = std::chrono::high_resolution_clock::now();
    double sum = 0;
    double dx = (b - a) / n;
    
    if (method == "riemann") {
        for (int i = 0; i < n; ++i) {
            double x = a + (i + 0.5) * dx;
            sum += Parser::evaluate(rpn.data(), rpn.size(), x);
        }
        sum *= dx;
    } else if (method == "simpson") {
        sum = Parser::evaluate(rpn.data(), rpn.size(), a) + Parser::evaluate(rpn.data(), rpn.size(), b);
        for (int i = 1; i < n; i++) {
            double x = a + i * dx;
            if (i % 2 == 1) sum += 4 * Parser::evaluate(rpn.data(), rpn.size(), x);
            else sum += 2 * Parser::evaluate(rpn.data(), rpn.size(), x);
        }
        sum *= (dx / 3.0);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> diff = end - start;
    
    return {sum, diff.count(), 0};
}

IntegrationResult Integrator::integrate_omp(const std::vector<DeviceToken>& rpn, double a, double b, int n, const std::string& method, int threads) {
    auto start = std::chrono::high_resolution_clock::now();
    double sum = 0;
    double dx = (b - a) / n;
    
    omp_set_num_threads(threads);
    
    if (method == "riemann") {
        #pragma omp parallel for reduction(+:sum)
        for (int i = 0; i < n; ++i) {
            double x = a + (i + 0.5) * dx;
            sum += Parser::evaluate(rpn.data(), rpn.size(), x);
        }
        sum *= dx;
    } else if (method == "simpson") {
        sum = Parser::evaluate(rpn.data(), rpn.size(), a) + Parser::evaluate(rpn.data(), rpn.size(), b);
        #pragma omp parallel for reduction(+:sum)
        for (int i = 1; i < n; i++) {
            double x = a + i * dx;
            if (i % 2 == 1) sum += 4 * Parser::evaluate(rpn.data(), rpn.size(), x);
            else sum += 2 * Parser::evaluate(rpn.data(), rpn.size(), x);
        }
        sum *= (dx / 3.0);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> diff = end - start;
    
    return {sum, diff.count(), 0};
}

IntegrationResult Integrator::integrate_cuda(const std::vector<DeviceToken>& rpn, double a, double b, int n, const std::string& method, int block_size, int grid_size) {
    auto start_total = std::chrono::high_resolution_clock::now();
    
    DeviceToken* d_rpn;
    double* d_result;
    double h_result = 0;
    double dx = (b - a) / n;
    
    auto start_mem = std::chrono::high_resolution_clock::now();
    cudaMalloc(&d_rpn, rpn.size() * sizeof(DeviceToken));
    cudaMalloc(&d_result, sizeof(double));
    cudaMemcpy(d_rpn, rpn.data(), rpn.size() * sizeof(DeviceToken), cudaMemcpyHostToDevice);
    cudaMemset(d_result, 0, sizeof(double));
    auto end_mem = std::chrono::high_resolution_clock::now();
    
    auto start_compute = std::chrono::high_resolution_clock::now();
    if (method == "riemann") {
        riemann_kernel<<<grid_size, block_size, block_size * sizeof(double)>>>(d_rpn, rpn.size(), a, dx, n, d_result);
    } else {
        simpson_kernel<<<grid_size, block_size, block_size * sizeof(double)>>>(d_rpn, rpn.size(), a, dx, n, d_result);
    }
    cudaDeviceSynchronize();
    auto end_compute = std::chrono::high_resolution_clock::now();
    
    auto start_mem2 = std::chrono::high_resolution_clock::now();
    cudaMemcpy(&h_result, d_result, sizeof(double), cudaMemcpyDeviceToHost);
    auto end_mem2 = std::chrono::high_resolution_clock::now();
    
    if (method == "riemann") h_result *= dx;
    else h_result *= (dx / 3.0);
    
    cudaFree(d_rpn);
    cudaFree(d_result);
    
    auto end_total = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> diff_total = end_total - start_total;
    std::chrono::duration<double, std::milli> diff_mem = (end_mem - start_mem) + (end_mem2 - start_mem2);
    std::chrono::duration<double, std::milli> diff_compute = end_compute - start_compute;
    
    return {h_result, diff_compute.count(), diff_mem.count()};
}
