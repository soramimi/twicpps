
#ifndef __TWEET_H
#define __TWEET_H

#include <string>
#include "oauth.h"

class TwitterClient {
private:
	struct Data {
		std::string consumer_key;
		std::string consumer_sec;
		std::string accesstoken;
		std::string accesstoken_sec;
	} data;
	char const *consumer_key() const { return data.consumer_key.c_str(); }
	char const *consumer_sec() const { return data.consumer_sec.c_str(); }
	char const *accesstoken() const { return data.accesstoken.c_str(); }
	char const *accesstoken_sec() const { return data.accesstoken_sec.c_str(); }
	oauth::Keys keys() const
	{
		return oauth::Keys(consumer_key(), consumer_sec(), accesstoken(), accesstoken_sec());
	}
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
	bool tweet(std::string message, const std::vector<std::string> *media_ids = nullptr);
	std::string upload(const std::string &path);
};

#endif
