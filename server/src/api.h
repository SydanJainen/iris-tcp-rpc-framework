#ifndef IRIS_API_H
#define IRIS_API_H

#include <string>

#define API_EXPORT

API_EXPORT int add(int a, int b);
API_EXPORT std::string reverse(std::string s);
API_EXPORT double multiply(double a, double b);
API_EXPORT int fibonacci(int n);
API_EXPORT std::string to_base(int number, int base);
API_EXPORT std::string tfidf(std::string term);

void helper();

#endif
