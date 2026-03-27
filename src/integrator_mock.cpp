/**
 * @file integrator_mock.cpp
 * @author Yassin Ahmad
 * @brief Serial and OpenMP implementations of numerical integration. Mock CUDA in case CUDA toolkit does not exist.
 */

#include "integrator.hpp"
#include <chrono>
#include <omp.h>
#include <iostream>

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
    std::cout << "Warning: System does not have CUDA. Running mock CUDA (Serial implementation)." << std::endl;
    std::cout << "Block size: " << block_size << ", Grid size: " << grid_size << " (ignored in mock)" << std::endl;
    return integrate_serial(rpn, a, b, n, method);
}
