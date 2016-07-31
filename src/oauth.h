
#ifndef OAUTH_H_
#define OAUTH_H_

#include <vector>
#include <string>
#include <stdint.h>

std::string sign_hmac_sha1(std::string const &m, std::string const &k);

class oauth {
public:
	class Keys {
	public:
		std::string consumer_key;
		std::string consumer_sec;
		std::string accesstoken;
		std::string accesstoken_sec;

		Keys()
		{
		}

		Keys(std::string consumer_key, std::string consumer_sec, std::string accesstoken, std::string accesstoken_sec)
			: consumer_key(consumer_key)
			, consumer_sec(consumer_sec)
			, accesstoken(accesstoken)
			, accesstoken_sec(accesstoken_sec)
		{
		}
	};
	enum http_method_t {
		GET,
		POST,
	};
	struct Request {
		std::string url;
		std::string post;
	};
	static Request sign(const char *url, http_method_t http_method, Keys const &keys);
private:
	static std::string encode_base64(const unsigned char *src, int size);
	static std::string decode_base64(const char *src);
	static std::string url_escape(const char *string);
	static std::string url_unescape(const char *string);
	static void sign_process(std::vector<std::string> *args, http_method_t http_method, const Keys &keys);
	static std::string build_url(const std::vector<std::string> &argv, int start, const std::string &sep);
	static std::string sign_hmac_sha1(const std::string &m, const std::string &k);
	static void hmac_sha1(const uint8_t *key, size_t keylen, const uint8_t *in, size_t inlen, uint8_t *resbuf);
};


#endif
