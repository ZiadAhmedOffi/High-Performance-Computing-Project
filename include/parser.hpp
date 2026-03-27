/**
 * @file parser.hpp
 * @author Ziad Ahmed
 * @brief Expression parser and evaluator using Shunting-Yard and RPN.
 */

#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>
#include <cmath>

#ifdef __CUDACC__
#define CUDA_HOST_DEVICE __host__ __device__
#else
#define CUDA_HOST_DEVICE
#endif

enum class TokenType {
    NUMBER,
    VARIABLE,
    OPERATOR,
    FUNCTION,
    LPAREN,
    RPAREN
};

struct Token {
    TokenType type;
    double value;       // For NUMBER
    char op;            // For OPERATOR (+, -, *, /, ^)
    std::string func;   // For FUNCTION (sin, cos, etc.)
    int precedence;

    CUDA_HOST_DEVICE Token() : type(TokenType::NUMBER), value(0), op(0), precedence(0) {}
};

// Simplified Token for GPU to avoid std::string
struct DeviceToken {
    TokenType type;
    double value;
    char op;            // +, -, *, /, ^
    int func_id;        // 1: sin, 2: cos, 3: tan, 4: exp, 5: log
    int precedence;

    CUDA_HOST_DEVICE DeviceToken() : type(TokenType::NUMBER), value(0), op(0), func_id(0), precedence(0) {}
};

class Parser {
public:
    static std::vector<Token> tokenize(const std::string& expression);
    static std::vector<Token> shuntingYard(const std::vector<Token>& tokens);
    static std::vector<DeviceToken> toDeviceTokens(const std::vector<Token>& tokens);
    
    CUDA_HOST_DEVICE inline static double evaluate(const DeviceToken* rpn, int size, double x) {
        double stack[32];
        int top = -1;

        for (int i = 0; i < size; ++i) {
            const DeviceToken& t = rpn[i];
            if (t.type == TokenType::NUMBER) {
                stack[++top] = t.value;
            } else if (t.type == TokenType::VARIABLE) {
                stack[++top] = x;
            } else if (t.type == TokenType::OPERATOR) {
                double b = stack[top--];
                double a = stack[top--];
                switch (t.op) {
                    case '+': stack[++top] = a + b; break;
                    case '-': stack[++top] = a - b; break;
                    case '*': stack[++top] = a * b; break;
                    case '/': stack[++top] = a / b; break;
                    case '^': stack[++top] = pow(a, b); break;
                }
            } else if (t.type == TokenType::FUNCTION) {
                double a = stack[top--];
                switch (t.func_id) {
                    case 1: stack[++top] = sin(a); break;
                    case 2: stack[++top] = cos(a); break;
                    case 3: stack[++top] = tan(a); break;
                    case 4: stack[++top] = exp(a); break;
                    case 5: stack[++top] = log(a); break;
                }
            }
        }
        return stack[top];
    }
};

#endif
