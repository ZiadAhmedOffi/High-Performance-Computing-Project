/**
 * @file parser.cpp
 * @author Ziad Ahmed
 * @brief Implementation of expression parser using Shunting-Yard and RPN.
 */

#include "parser.hpp"
#include <iostream>
#include <stack>
#include <cmath>
#include <algorithm>
#include <cctype>

std::vector<Token> Parser::tokenize(const std::string& expression) {
    std::vector<Token> tokens;
    size_t i = 0;
    while (i < expression.length()) {
        char c = expression[i];
        if (std::isspace(c)) {
            i++;
            continue;
        }

        if (std::isdigit(c) || c == '.') {
            Token t;
            t.type = TokenType::NUMBER;
            size_t end;
            t.value = std::stod(expression.substr(i), &end);
            i += end;
            tokens.push_back(t);
            continue;
        }

        if (c == 'x') {
            Token t;
            t.type = TokenType::VARIABLE;
            tokens.push_back(t);
            i++;
            continue;
        }

        if (c == '+' || c == '-' || c == '*' || c == '/' || c == '^') {
            Token t;
            t.type = TokenType::OPERATOR;
            t.op = c;
            if (c == '+' || c == '-') t.precedence = 1;
            else if (c == '*' || c == '/') t.precedence = 2;
            else if (c == '^') t.precedence = 3;
            tokens.push_back(t);
            i++;
            continue;
        }

        if (c == '(') {
            Token t;
            t.type = TokenType::LPAREN;
            tokens.push_back(t);
            i++;
            continue;
        }

        if (c == ')') {
            Token t;
            t.type = TokenType::RPAREN;
            tokens.push_back(t);
            i++;
            continue;
        }

        if (std::isalpha(c)) {
            size_t start = i;
            while (i < expression.length() && std::isalpha(expression[i])) i++;
            std::string func = expression.substr(start, i - start);
            Token t;
            t.type = TokenType::FUNCTION;
            t.func = func;
            tokens.push_back(t);
            continue;
        }
        i++;
    }
    return tokens;
}

std::vector<Token> Parser::shuntingYard(const std::vector<Token>& tokens) {
    std::vector<Token> output;
    std::stack<Token> ops;

    for (const auto& token : tokens) {
        if (token.type == TokenType::NUMBER || token.type == TokenType::VARIABLE) {
            output.push_back(token);
        } else if (token.type == TokenType::FUNCTION) {
            ops.push(token);
        } else if (token.type == TokenType::OPERATOR) {
            while (!ops.empty() && ops.top().type == TokenType::OPERATOR &&
                   ((token.op != '^' && ops.top().precedence >= token.precedence) ||
                    (token.op == '^' && ops.top().precedence > token.precedence))) {
                output.push_back(ops.top());
                ops.pop();
            }
            ops.push(token);
        } else if (token.type == TokenType::LPAREN) {
            ops.push(token);
        } else if (token.type == TokenType::RPAREN) {
            while (!ops.empty() && ops.top().type != TokenType::LPAREN) {
                output.push_back(ops.top());
                ops.pop();
            }
            if (!ops.empty()) ops.pop(); // Remove LPAREN
            if (!ops.empty() && ops.top().type == TokenType::FUNCTION) {
                output.push_back(ops.top());
                ops.pop();
            }
        }
    }

    while (!ops.empty()) {
        output.push_back(ops.top());
        ops.pop();
    }
    return output;
}

std::vector<DeviceToken> Parser::toDeviceTokens(const std::vector<Token>& tokens) {
    std::vector<DeviceToken> d_tokens;
    for (const auto& t : tokens) {
        DeviceToken dt;
        dt.type = t.type;
        dt.value = t.value;
        dt.op = t.op;
        dt.precedence = t.precedence;
        if (t.type == TokenType::FUNCTION) {
            if (t.func == "sin") dt.func_id = 1;
            else if (t.func == "cos") dt.func_id = 2;
            else if (t.func == "tan") dt.func_id = 3;
            else if (t.func == "exp") dt.func_id = 4;
            else if (t.func == "log") dt.func_id = 5;
        }
        d_tokens.push_back(dt);
    }
    return d_tokens;
}
