
#ifndef __TWEET_H
#define __TWEET_H

#include <string>

class TwitterClient {
private:
	struct Data {
		std::string c_key;
		std::string c_sec;
		std::string t_key;
		std::string t_sec;
	} data;
	char const *c_key() const { return data.c_key.c_str(); }
	char const *c_sec() const { return data.c_sec.c_str(); }
	char const *t_key() const { return data.t_key.c_str(); }
	char const *t_sec() const { return data.t_sec.c_str(); }
public:
	TwitterClient()
	{
	}
	TwitterClient(std::string const &c_key, std::string const &c_sec, std::string const &t_key, std::string const &t_sec)
	{
		data.c_key = c_key;
		data.c_sec = c_sec;
		data.t_key = t_key;
		data.t_sec = t_sec;
	}
	bool tweet(std::string message);
};

#endif
