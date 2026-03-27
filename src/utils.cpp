/**
 * @file utils.cpp
 * @author Ziad Ahmed
 * @brief Implementation of utility functions for system info.
 */

#include "utils.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/utsname.h>

void Utils::log_system_info() {
    std::cout << "--- System Information ---" << std::endl;
    std::cout << "OS: " << get_os_info() << std::endl;
    std::cout << "CPU: " << get_cpu_info() << std::endl;
    std::cout << "GPU: " << get_gpu_info() << std::endl;
    std::cout << "--------------------------" << std::endl;
}

std::string Utils::get_cpu_info() {
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.substr(0, 10) == "model name") {
            return line.substr(line.find(":") + 2);
        }
    }
    return "Unknown CPU";
}

std::string Utils::get_gpu_info() {
#ifdef __CUDACC__
    int deviceCount;
    cudaGetDeviceCount(&deviceCount);
    if (deviceCount == 0) return "No CUDA GPU detected";
    
    cudaDeviceProp prop;
    cudaGetDeviceProperties(&prop, 0);
    return std::string(prop.name) + " (Compute " + std::to_string(prop.major) + "." + std::to_string(prop.minor) + ")";
#else
    return "CUDA not available at compile time";
#endif
}

std::string Utils::get_os_info() {
    struct utsname buffer;
    if (uname(&buffer) != 0) return "Unknown OS";
    return std::string(buffer.sysname) + " " + std::string(buffer.release) + " " + std::string(buffer.machine);
}
