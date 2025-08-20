// Simple test program using C++20 modules
import calculator;
import logger;

#include <iostream>
#include <string>

int main() {
  Logger::info("Starting module test program");

  int a = 10, b = 5;

  Logger::debug("Testing calculator module");

  int sum = Calculator::add(a, b);
  int product = Calculator::multiply(a, b);
  double quotient =
      Calculator::divide(static_cast<double>(a), static_cast<double>(b));

  Logger::info("Addition: " + std::to_string(a) + " + " + std::to_string(b) +
               " = " + std::to_string(sum));
  Logger::info("Multiplication: " + std::to_string(a) + " * " +
               std::to_string(b) + " = " + std::to_string(product));
  Logger::info("Division: " + std::to_string(a) + " / " + std::to_string(b) +
               " = " + std::to_string(quotient));

  Logger::info("Module test completed successfully!");

  return 0;
}
