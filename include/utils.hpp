/**
 * @file utils.hpp
 * @author Ziad Ahmed
 * @brief Utility functions for system info and timing.
 */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

class Utils {
public:
    static void log_system_info();
    static std::string get_cpu_info();
    static std::string get_gpu_info();
    static std::string get_os_info();
};

#endif
