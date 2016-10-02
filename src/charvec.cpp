#include "charvec.h"
#include <string.h>
#include <stdlib.h>

void print(std::vector<char> *out, char c)
{
	out->push_back(c);
}

void print(std::vector<char> *out, char const *begin, char const *end)
{
	out->insert(out->end(), begin, end);
}

void print(std::vector<char> *out, char const *ptr, size_t len)
{
	print(out, ptr, ptr + len);
}

void print(std::vector<char> *out, char const *s)
{
	print(out, s, s + strlen(s));
}

void print(std::vector<char> *out, std::string const &s)
{
	print(out, s.c_str(), s.size());
}

std::string to_stdstr(std::vector<char> const &vec)
{
	if (!vec.empty()) {
		char const *begin = &vec[0];
		char const *end = begin + vec.size();
		return std::string(begin, end);
	}
	return std::string();
}
