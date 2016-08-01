#ifndef JSON_H
#define JSON_H

#include <string>
#include <vector>

class JSON {
public:
	enum class Type {
		Unknown,
		Null,
		Number,
		String,
		Boolean,
		Array,
		Object,
	};

	class Value {
	public:
		Type type = Type::Unknown;
		std::string name;
		std::string value;
	};
private:
	class Node : public Value {
	public:
		std::vector<Node> children;
	};

	Node node;

	int scan_space(char const *ptr, char const *end);
	int parse_string(char const *begin, char const *end, std::string *out);
	int parse_name(char const *begin, char const *end, Node *node);
	int parse_value(char const *begin, char const *end, Node *node);
	int parse_array(char const *begin, char const *end, std::vector<Node> *children);
public:
	void parse(char const *begin, char const *end);

	Value get(std::string const &path) const;
};

#endif // JSON_H
