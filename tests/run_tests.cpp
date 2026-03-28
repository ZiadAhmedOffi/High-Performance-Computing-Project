/**
 * @file run_tests.cpp
 * @author Ahmed Osama Ibrahim
 * @brief Unit tests for the HPC Integrator and Parser.
 */

#include "parser.hpp"
#include "integrator.hpp"
#include <iostream>
#include <cmath>
#include <cassert>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void test_parser() {
    std::cout << "Running Parser Tests..." << std::endl;
    auto tokens = Parser::tokenize("x^2 + 2*x + 1");
    auto rpn = Parser::shuntingYard(tokens);
    auto device_rpn = Parser::toDeviceTokens(rpn);

    double result = Parser::evaluate(device_rpn.data(), (int)device_rpn.size(), 2.0);
    // 2^2 + 2*2 + 1 = 9
    assert(std::abs(result - 9.0) < 1e-9);
    std::cout << "  Parser Evaluation: PASSED" << std::endl;
}

void test_integrals() {
    std::cout << "Running Integration Logic Tests..." << std::endl;
    
    // Test 1: Integral of x^2 from 0 to 1 = 1/3
    auto tokens = Parser::tokenize("x^2");
    auto rpn = Parser::shuntingYard(tokens);
    auto d_rpn = Parser::toDeviceTokens(rpn);
    
    auto res = Integrator::integrate_serial(d_rpn, 0, 1, 1000000, "riemann");
    assert(std::abs(res.value - 0.3333333333) < 1e-6);
    std::cout << "  Integral x^2 [0,1]: PASSED" << std::endl;

    // Test 2: Integral of sin(x) from 0 to PI = 2
    tokens = Parser::tokenize("sin(x)");
    rpn = Parser::shuntingYard(tokens);
    d_rpn = Parser::toDeviceTokens(rpn);
    res = Integrator::integrate_serial(d_rpn, 0, M_PI, 1000000, "riemann");
    assert(std::abs(res.value - 2.0) < 1e-6);
    std::cout << "  Integral sin(x) [0,PI]: PASSED" << std::endl;
}

int main() {
    try {
        test_parser();
        test_integrals();
        std::cout << "\nALL TESTS PASSED!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
