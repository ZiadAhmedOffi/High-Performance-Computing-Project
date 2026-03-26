/** 
 * @file integrator.hpp
 * @author Yassin Ahmad 
 * @brief Interface for numerical integration (Serial, OpenMP, CUDA).
 */

 #ifndef INTEGRATOR_HPP
#define INTEGRATOR_HPP

#include "parser.hpp"
#include <string>

struct IntegrationResult {
    double value;
    double time_ms;
    double mem_transfer_time_ms; // For CUDA
};

class Integrator {
public:
    static IntegrationResult integrate_serial(const std::vector<DeviceToken>& rpn, double a, double b, int n, const std::string& method);
    static IntegrationResult integrate_omp(const std::vector<DeviceToken>& rpn, double a, double b, int n, const std::string& method, int threads);
    static IntegrationResult integrate_cuda(const std::vector<DeviceToken>& rpn, double a, double b, int n, const std::string& method, int block_size, int grid_size);
};

#endif
