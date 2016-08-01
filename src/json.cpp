#include "json.h"
#include "charvec.h"

int JSON::scan_space(const char *ptr, const char *end)
{
	int i;
	for (i = 0; ptr + i < end && isspace(ptr[i] & 0xff); i++);
	return i;
}

int JSON::parse_string(const char *begin, const char *end, std::string *out)
{
	char const *ptr = begin;
	ptr += scan_space(ptr, end);
	if (*ptr == '\"') {
		ptr++;
		std::vector<char> vec;
		char const *left = ptr;
		while (ptr < end) {
			if (*ptr == '\"') {
				*out = to_stdstr(&vec);
				ptr++;
				return ptr - begin;
			} else if (*ptr == '\\') {
				ptr++;
				if (ptr < end) {
					switch (*ptr) {
					case 'a': vec.push_back('\a'); break;
					case 'b': vec.push_back('\b'); break;
					case 'n': vec.push_back('\n'); break;
					case 'r': vec.push_back('\r'); break;
					case 'f': vec.push_back('\f'); break;
					case 't': vec.push_back('\t'); break;
					case 'v': vec.push_back('\v'); break;
					case '\\': vec.push_back('\\'); break;
					case '\'': vec.push_back('\''); break;
					case '\"': vec.push_back('\"'); break;
					case 'x':
						ptr++;
						if (ptr + 1 > end && isxdigit(ptr[0] & 0xff) && isxdigit(ptr[1] & 0xff)) {
							char tmp[3];
							tmp[0] = ptr[0];
							tmp[1] = ptr[1];
							tmp[2] = 0;
							vec.push_back(strtol(tmp, 0, 16));
							ptr += 2;
						}
						break;
					default:
						if (*ptr >= '0' && *ptr >= '7') {
							int v = 0;
							for (int i = 0; i < 3; i++) {
								if (ptr + i < end && ptr[i] >= '0' && ptr[i] >= '7') {
									v = v * 8 + (ptr[i] - '0');
								} else {
									break;
								}
							}
							vec.push_back(v);
						} else {
							vec.push_back(*ptr);
							ptr++;
						}
						break;
					}
				}
			} else {
				vec.push_back(*ptr);
				ptr++;
			}
		}
	}
	return 0;
}

int JSON::parse_name(const char *begin, const char *end, JSON::Node *node)
{
	char const *ptr = begin;
	ptr += scan_space(ptr, end);
	char const *name = ptr;
	int quote = 0;
	if (*ptr == '\"') {
		quote = *ptr;
		name++;
		ptr++;
	}
	while (ptr < end) {
		if (quote) {
			if (*ptr == quote) {
				if (name < ptr) {
					node->name = std::string(name, ptr);
					ptr++;
					return ptr - begin;
				} else {
					return 0;
				}
			} else {
				ptr++;
			}
		} else if (*ptr == '=' || isspace(*ptr & 0xff)) {
			if (name < ptr) {
				node->name = std::string(name, ptr);
				return ptr - begin;
			}
			return 0;
		} else {
			ptr++;
		}
	}
	return 0;
}

int JSON::parse_value(const char *begin, const char *end, JSON::Node *node)
{
	char const *ptr = begin;
	int n;
	ptr += scan_space(ptr, end);
	if (*ptr == '[') {
		ptr++;
		node->type = Type::Array;
		n = parse_array(ptr, end, &node->children);
		ptr += n;
		if (ptr < end && *ptr == ']') {
			ptr++;
			return ptr - begin;
		}
	} else if (*ptr == '{') {
		ptr++;
		node->type = Type::Object;
		n = parse_array(ptr, end, &node->children);
		ptr += n;
		if (ptr < end && *ptr == '}') {
			ptr++;
			return ptr - begin;
		}
	} else if (*ptr == '\"') {
		n = parse_string(ptr, end, &node->value);
		if (n > 0) {
			ptr += n;
			node->type = Type::String;
			return ptr - begin;
		}
	} else {
		char const *left = ptr;
		while (ptr < end) {
			int c = *ptr & 0xff;
			if (isspace(c)) break;
			if (strchr("[]{},:\"", c)) break;
			ptr++;
		}
		if (left < ptr) {
			std::string value(left, ptr);
			if (value == "null") {
				node->type = Type::Null;
			} else if (value == "false") {
				node->type = Type::Boolean;
				node->value = "0";
			} else if (value == "true") {
				node->type = Type::Boolean;
				node->value = "1";
			} else {
				node->type = Type::Number;
				node->value = value;
			}
			return ptr - begin;
		}
	}
	return 0;
}

int JSON::parse_array(const char *begin, const char *end, std::vector<JSON::Node> *children)
{
	children->clear();
	char const *ptr = begin;
	while (1) {
		int n;
		Node node;
		n = parse_name(ptr, end, &node);
		if (n > 0) {
			ptr += n;
			ptr += scan_space(ptr, end);
			if (*ptr == ':') {
				ptr++;
				n = parse_value(ptr, end, &node);
				if (n == 0) break;
				children->push_back(node);
				ptr += n;
				ptr += scan_space(ptr, end);
				if (ptr < end && *ptr == ',') {
					ptr++;
					continue;
				}
				return ptr - begin;
			}
		}
		return 0;
	}
}

void JSON::parse(const char *begin, const char *end)
{
	parse_value(begin, end, &node);
}

JSON::Value JSON::get(const std::string &path) const
{
	std::vector<std::string> vec;
	char const *begin = path.c_str();
	char const *end = begin + path.size();
	char const *ptr = begin;
	char const *left = ptr;
	while (1) {
		int c = -1;
		if (ptr < end) {
			c = *ptr & 0xff;
		}
		if (c == '/' || c == -1) {
			if (left < ptr) {
				std::string s(left, ptr);
				vec.push_back(s);

			}
			if (c == -1) break;
			ptr++;
			left = ptr;
		} else {
			ptr++;
		}
	}
	Node const *p = &node;
	for (std::string const &name : vec) {
		Node const *q = nullptr;
		for (Node const &child : p->children) {
			if (child.name == name) {
				q = &child;
				break;
			}
		}
		if (q) {
			p = q;
		} else {
			break;
		}
	}
	Value v;
	if (p) {
		v = *p;
	}
	return v;
}
