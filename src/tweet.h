
#ifndef TWEET_H_
#define TWEET_H_

#include <string>
#include "oauth.h"

class TwitterClient {
private:
	struct Data {
		oauth::Keys keys;
	} data;
	oauth::Keys const &keys() const
	{
		return data.keys;
	}
	struct RequestOption {
		enum Method {
			GET,
			POST,
		};
		Method method = GET;
		char const *post_begin = nullptr;
		char const *post_end = nullptr;
		std::string upload_path;
		void set_post_data(char const *begin, char const *end)
		{
			post_begin = begin;
			post_end = end;
			method = POST;
		}
		void set_upload_path(std::string const &path)
		{
			upload_path = path;
		}
	};
	bool request(const std::string &url, RequestOption const &opt, std::string *reply);
public:
	TwitterClient()
	{
	}
	TwitterClient(std::string const &consumer_key, std::string const &consumer_sec, std::string const &accesstoken, std::string const &accesstoken_sec)
	{
		data.keys.consumer_key = consumer_key;
		data.keys.consumer_sec = consumer_sec;
		data.keys.accesstoken = accesstoken;
		data.keys.accesstoken_sec = accesstoken_sec;
	}
	bool tweet(std::string message, const std::vector<std::string> *media_ids = nullptr);
	std::string upload(const std::string &path);
};

#endif
