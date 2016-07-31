
#ifndef __CHARVEC_H
#define __CHARVEC_H

#include <vector>
#include <string>

void print(std::vector<char> *out, char c);
void print(std::vector<char> *out, char const *begin, char const *end);
void print(std::vector<char> *out, char const *ptr, size_t len);
void print(std::vector<char> *out, char const *s);
void print(std::vector<char> *out, std::string const &s);
std::string to_stdstr(std::vector<char> const *vec);

#endif
