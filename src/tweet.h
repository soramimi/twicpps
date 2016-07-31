
#ifndef __TWEET_H
#define __TWEET_H

#include <string>

class TwitterClient {
private:
	struct Data {
		std::string consumer_key;
		std::string consumer_sec;
		std::string accesstoken;
		std::string accesstoken_sec;
	} data;
	char const *c_key() const { return data.consumer_key.c_str(); }
	char const *c_sec() const { return data.consumer_sec.c_str(); }
	char const *t_key() const { return data.accesstoken.c_str(); }
	char const *t_sec() const { return data.accesstoken_sec.c_str(); }
public:
	TwitterClient()
	{
	}
	TwitterClient(std::string const &consumer_key, std::string const &consumer_sec, std::string const &accesstoken, std::string const &accesstoken_sec)
	{
		data.consumer_key = consumer_key;
		data.consumer_sec = consumer_sec;
		data.accesstoken = accesstoken;
		data.accesstoken_sec = accesstoken_sec;
	}
	bool tweet(std::string message);
};

#endif
