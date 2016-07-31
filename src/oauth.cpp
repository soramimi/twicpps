
#include "oauth.h"
#include <time.h>
#include <algorithm>
#include "urlencode.h"
#include "sha1.h"
#include "base64.h"
#include "charvec.h"

void oauth::hmac_sha1(uint8_t const *key, size_t keylen, uint8_t const *in, size_t inlen, uint8_t *resbuf)
{
	SHA1Context inner;
	SHA1Context outer;
	uint8_t tmpkey[20];
	uint8_t digest[20];
	uint8_t block[64];

	const int IPAD = 0x36;
	const int OPAD = 0x5c;

	if (keylen > 64) {
		struct SHA1Context keyhash;
		SHA1Reset(&keyhash);
		SHA1Input(&keyhash, (uint8_t const *)key, keylen);
		SHA1Result(&keyhash, (uint8_t *)tmpkey);
		key = tmpkey;
		keylen = 20;
	}

	for (size_t i = 0; i < sizeof(block); i++) {
		block[i] = IPAD ^ (i < keylen ? key[i] : 0);
	}
	SHA1Reset(&inner);
	SHA1Input(&inner, (uint8_t const *)block, 64);
	SHA1Input(&inner, (uint8_t const *)in, inlen);
	SHA1Result(&inner, digest);

	for (size_t i = 0; i < sizeof(block); i++) {
		block[i] = OPAD ^ (i < keylen ? key[i] : 0);
	}
	SHA1Reset(&outer);
	SHA1Input(&outer, (uint8_t const *)block, 64);
	SHA1Input(&outer, (uint8_t const *)digest, 20);
	SHA1Result(&outer, (uint8_t *)resbuf);
}

std::string oauth::sign_hmac_sha1(std::string const &m, std::string const &k)
{
	uint8_t result[20];
	hmac_sha1((uint8_t const *)k.c_str(), k.size(), (uint8_t const *)m.c_str(), m.size(), result);
	std::vector<char> vec;
	base64_encode((char const *)result, 20, &vec);
	return to_stdstr(&vec);
}


std::string oauth::encode_base64(const unsigned char *src, int size)
{
	std::vector<char> vec;
	base64_encode((char const *)src, size, &vec);
	return to_stdstr(&vec);

}

std::string oauth::decode_base64(const char *src)
{
	std::vector<char> vec;
	base64_decode(src, &vec);
	return to_stdstr(&vec);
}

std::string oauth::url_escape(const char *string)
{
	return url_encode(string);
}

std::string oauth::url_unescape(const char *string)
{
	return url_decode(string);
}

std::string oauth::build_url(std::vector<std::string> const &argv, int start, std::string const &sep)
{
	std::string query;
	for (size_t i = start; i < argv.size(); i++) {
		std::string s = argv[i];
		if (i > 0) {
			char const *p = s.c_str();
			char const *e = strchr(p, '=');
			if (e) {
				std::string name(p, e);
				std::string value = e + 1;
				s = name + '=' + url_encode(value);
			} else {
				s += '=';
			}
		}
		if (!query.empty()) {
			query += sep;
		}
		query += s;
	}
	return query;
}

void oauth::sign_process(std::vector<std::string> *args, http_method_t http_method, const Keys &keys)
{
	auto to_s = [](int v)->std::string{
		char tmp[100];
		sprintf(tmp, "%d", v);
		return tmp;
	};

	{
		auto nonce = [](){
			static const char *chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
			const unsigned int max = 26 + 26 + 10 + 1; //strlen(chars);
			char tmp[50];
			int i, len;

			srand((unsigned int)time(0));
			len = 15 + rand() % 16;
			for (i = 0; i < len; i++) {
				tmp[i] = chars[rand() % max];
			}
			return std::string(tmp, i);
		};

		auto is_key_contains = [](std::vector<std::string> const &argv, std::string const &key)
		{
			size_t keylen = key.size();
			for (std::string const &s : argv) {
				if (strncmp(s.c_str(), key.c_str(), keylen) == 0 && s[keylen] == '=') {
					return true;
				}
			}
			return false;
		};

		std::string oauth_nonce = "oauth_nonce";
		if (!is_key_contains(*args, oauth_nonce)) {
			oauth_nonce += '=';
			oauth_nonce += nonce();
			args->push_back(oauth_nonce);
		}

		std::string oauth_timestamp = "oauth_timestamp";
		if (!is_key_contains(*args, oauth_timestamp)) {
			oauth_timestamp += '=';
			oauth_timestamp += to_s((int)time(nullptr));
			args->push_back(oauth_timestamp);
		}

		if (!keys.accesstoken.empty()) {
			std::string oauth_token = "oauth_token";
			oauth_token += '=';
			oauth_token += keys.accesstoken;
			args->push_back(oauth_token);
		}

		std::string oauth_consumer_key = "oauth_consumer_key";
		oauth_consumer_key += '=';
		oauth_consumer_key += keys.consumer_key;
		args->push_back(oauth_consumer_key);

		std::string oauth_signature_method = "oauth_signature_method";
		oauth_signature_method += '=';
		oauth_signature_method += "HMAC-SHA1";
		args->push_back(oauth_signature_method);

		std::string oauth_version = "oauth_version";
		if (!is_key_contains(*args, oauth_version)) {
			oauth_version += '=';
			oauth_version += "1.0";
			args->push_back(oauth_version);
		}

	}
	std::sort(args->begin() + 1, args->end());


	auto Combine = [](std::initializer_list<std::string> list){
		std::string text;
		for (std::string const &s : list) {
			if (!s.empty()) {
				if (!text.empty()) {
					text += '&';
				}
				text += url_encode(s);
			}
		}
		return text;
	};

	std::string query = oauth::build_url(*args, 1, "&");

	std::string httpmethod;
	if (http_method == http_method_t::GET) {
		httpmethod = "GET";
	} else if (http_method == http_method_t::POST) {
		httpmethod = "POST";
	}
	std::string m = Combine({httpmethod, (*args)[0], query});
	std::string k = Combine({std::string(keys.consumer_sec), std::string(keys.accesstoken_sec)});

	std::string oauth_signature = "oauth_signature";
	oauth_signature += '=';
	oauth_signature += sign_hmac_sha1(m, k);

	args->push_back(oauth_signature);

	for (std::string const &s : *args) {
		puts(s.c_str());
	}
}

oauth::Request oauth::sign(const char *url, http_method_t http_method, const Keys &keys)
{
	auto split_url = [](const char *url, std::vector<std::string> *out){
		out->clear();
		char const *left = url;
		char const *ptr = left;
		while (1) {
			int c = *ptr & 0xff;
			if (c == '&' || c == '?' || c == 0) {
				if (left < ptr) {
					out->push_back(std::string(left, ptr));
				}
				if (c == 0) break;
				ptr++;
				left = ptr;
			} else {
				ptr++;
			}
		}
		for (size_t i = 1; i < out->size(); i++) {
			std::string *p = &out->at(i);
			*p = url_decode(*p);
		}
	};

	std::vector<std::string> argv;
	split_url(url, &argv);

	sign_process(&argv, http_method, keys);

	if (http_method == POST) {
		Request req;
		req.post = oauth::build_url(argv, 1, "&");
		req.url = argv.at(0);
		return req;
	} else {
		Request req;
		req.url = oauth::build_url(argv, 0, "&");
		return req;
	}
}

