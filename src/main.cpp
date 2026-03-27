/** @file main.cpp
 ##* @author [Mahmoud Atef]
 ##* @brief CLI entry point for HPC Numerical Integrator.
*/

#include "parser.hpp"
#include "integrator.hpp"
#include "utils.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <getopt.h>

void print_help() {
    std::cout << "HPC Numerical Integrator\n"
              << "Usage: ./hpc_integrator [options]\n\n"
              << "Options:\n"
              << "  --function <str>    Mathematical expression to integrate (e.g., \"x^2 + sin(x)\")\n"
              << "  --a <float>         Lower integration limit\n"
              << "  --b <float>         Upper integration limit\n"
              << "  --n <int>           Number of intervals\n"
              << "  --method <str>      Integration method: riemann, simpson\n"
              << "  --impl <str>        Implementation: serial, omp, cuda\n"
              << "  --threads <int>     Number of OpenMP threads (default: 1)\n"
              << "  --block-size <int>  CUDA block size (default: 256)\n"
              << "  --grid-size <int>   CUDA grid size (default: 1024)\n"
              << "  --repeats <int>     Number of experiments to run for averaging (default: 1)\n"
              << "  --output <path>     Path for CSV/plot output (optional)\n"
              << "  --help              Show this help message\n";
}

int main(int argc, char** argv) {
    std::string function_str = "x^2";
    double a = 0, b = 1;
    int n = 1000000;
    std::string method = "riemann";
    std::string impl = "serial";
    int threads = 1;
    int block_size = 256;
    int grid_size = 1024;
    int repeats = 1;
    std::string output_path = "";

    static struct option long_options[] = {
        {"function", required_argument, 0, 'f'},
        {"a", required_argument, 0, 'a'},
        {"b", required_argument, 0, 'b'},
        {"n", required_argument, 0, 'n'},
        {"method", required_argument, 0, 'm'},
        {"impl", required_argument, 0, 'i'},
        {"threads", required_argument, 0, 't'},
        {"block-size", required_argument, 0, 's'},
        {"grid-size", required_argument, 0, 'g'},
        {"repeats", required_argument, 0, 'r'},
        {"output", required_argument, 0, 'o'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt_index = 0;
    int c;
    while ((c = getopt_long(argc, argv, "f:a:b:n:m:i:t:s:g:r:o:h", long_options, &opt_index)) != -1) {
        switch (c) {
            case 'f': function_str = optarg; break;
            case 'a': a = std::stod(optarg); break;
            case 'b': b = std::stod(optarg); break;
            case 'n': n = std::stoi(optarg); break;
            case 'm': method = optarg; break;
            case 'i': impl = optarg; break;
            case 't': threads = std::stoi(optarg); break;
            case 's': block_size = std::stoi(optarg); break;
            case 'g': grid_size = std::stoi(optarg); break;
            case 'r': repeats = std::stoi(optarg); break;
            case 'o': output_path = optarg; break;
            case 'h': print_help(); return 0;
            default: break;
        }
    }

    if (method == "simpson" && n % 2 != 0) {
        std::cerr << "Error: Simpson's rule requires an even number of intervals (n)." << std::endl;
        return 1;
    }

    // Parse expression
    auto tokens = Parser::tokenize(function_str);
    auto rpn = Parser::shuntingYard(tokens);
    auto device_rpn = Parser::toDeviceTokens(rpn);

    Utils::log_system_info();

    double total_time = 0;
    double total_mem_time = 0;
    IntegrationResult res;

    for (int i = 0; i < repeats; ++i) {
        if (impl == "serial") res = Integrator::integrate_serial(device_rpn, a, b, n, method);
        else if (impl == "omp") res = Integrator::integrate_omp(device_rpn, a, b, n, method, threads);
        else if (impl == "cuda") res = Integrator::integrate_cuda(device_rpn, a, b, n, method, block_size, grid_size);
        else {
            std::cerr << "Unknown implementation: " << impl << std::endl;
            return 1;
        }
        total_time += res.time_ms;
        total_mem_time += res.mem_transfer_time_ms;
    }

    double avg_time = total_time / repeats;
    double avg_mem_time = total_mem_time / repeats;

    std::cout << std::fixed << std::setprecision(10);
    std::cout << "--- Integration Results ---" << std::endl;
    std::cout << "Function:   " << function_str << std::endl;
    std::cout << "Interval:   [" << a << ", " << b << "]" << std::endl;
    std::cout << "Method:     " << method << std::endl;
    std::cout << "Impl:       " << impl << std::endl;
    std::cout << "Result:     " << res.value << std::endl;
    std::cout << "Avg Time:   " << avg_time << " ms" << std::endl;
    if (impl == "cuda") {
        std::cout << "Avg Mem:    " << avg_mem_time << " ms" << std::endl;
        std::cout << "Compute %:  " << (avg_time / (avg_time + avg_mem_time)) * 100 << "%" << std::endl;
    }
    std::cout << "---------------------------" << std::endl;

    return 0;
}
