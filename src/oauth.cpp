
#include "oauth.h"
#include "urlencode.h"
#include "sha1.h"
#include "base64.h"
#include "charvec.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <algorithm>

void oauth::hmac_sha1(uint8_t const *key, size_t keylen, uint8_t const *in, size_t inlen, uint8_t *out)
{
	SHA1Context sha1;
	uint8_t tmp[20];

	uint8_t ibuf[64];
	uint8_t obuf[64];

	for (size_t i = 0; i < 64; i++) {
		uint8_t c = i < keylen ? key[i] : 0;
		ibuf[i] = c ^ 0x36;
		obuf[i] = c ^ 0x5c;
	}

	SHA1Reset(&sha1);
	SHA1Input(&sha1, ibuf, 64);
	SHA1Input(&sha1, in, inlen);
	SHA1Result(&sha1, tmp);

	SHA1Reset(&sha1);
	SHA1Input(&sha1, obuf, 64);
	SHA1Input(&sha1, tmp, 20);
	SHA1Result(&sha1, out);
}

std::string oauth::sign_hmac_sha1(std::string const &m, std::string const &k)
{
	uint8_t key[20];
	uint8_t result[20];

	SHA1Context sha1;
	SHA1Reset(&sha1);
	SHA1Input(&sha1, (uint8_t const *)k.c_str(), k.size());
	SHA1Result(&sha1, key);

	hmac_sha1(key, 20, (uint8_t const *)m.c_str(), m.size(), result);
	std::vector<char> vec;
	base64_encode((char const *)result, 20, &vec);
	return to_stdstr(vec);
}


std::string oauth::encode_base64(const unsigned char *src, int size)
{
	std::vector<char> vec;
	base64_encode((char const *)src, size, &vec);
	return to_stdstr(vec);

}

std::string oauth::decode_base64(const char *src)
{
	std::vector<char> vec;
	base64_decode(src, &vec);
	return to_stdstr(vec);
}

std::string oauth::url_escape(const char *string)
{
	return url_encode(string);
}

std::string oauth::url_unescape(const char *string)
{
	return url_decode(string);
}

void oauth::split_url(const char *url, std::vector<std::string> *out)
{
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
}

std::string oauth::build_url(std::vector<std::string> const &vec, int start)
{
	const char sep = '&';
	std::string query;
	for (size_t i = start; i < vec.size(); i++) {
		std::string s = vec[i];
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

void oauth::process_(std::vector<std::string> *vec, http_method_t http_method, const Keys &keys)
{
	auto to_s = [](int v)->std::string{
		char tmp[100];
		sprintf(tmp, "%d", v);
		return tmp;
	};

	{
		auto nonce = [](){
			static const char *chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
			const unsigned int max = 26 + 26 + 10 + 1;
			char tmp[50];
			srand((unsigned int)time(0));
			int len = 15 + rand() % 16;
			for (int i = 0; i < len; i++) {
				tmp[i] = chars[rand() % max];
			}
			return std::string(tmp, len);
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
		if (!is_key_contains(*vec, oauth_nonce)) {
			oauth_nonce += '=';
			oauth_nonce += nonce();
			vec->push_back(oauth_nonce);
		}

		std::string oauth_timestamp = "oauth_timestamp";
		if (!is_key_contains(*vec, oauth_timestamp)) {
			oauth_timestamp += '=';
			oauth_timestamp += to_s((int)time(nullptr));
			vec->push_back(oauth_timestamp);
		}

		if (!keys.accesstoken.empty()) {
			std::string oauth_token = "oauth_token";
			oauth_token += '=';
			oauth_token += keys.accesstoken;
			vec->push_back(oauth_token);
		}

		std::string oauth_consumer_key = "oauth_consumer_key";
		oauth_consumer_key += '=';
		oauth_consumer_key += keys.consumer_key;
		vec->push_back(oauth_consumer_key);

		std::string oauth_signature_method = "oauth_signature_method";
		oauth_signature_method += '=';
		oauth_signature_method += "HMAC-SHA1";
		vec->push_back(oauth_signature_method);

		std::string oauth_version = "oauth_version";
		if (!is_key_contains(*vec, oauth_version)) {
			oauth_version += '=';
			oauth_version += "1.0";
			vec->push_back(oauth_version);
		}

	}
	std::sort(vec->begin() + 1, vec->end());

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

	std::string query = oauth::build_url(*vec, 1);

	std::string httpmethod;
	if (http_method == http_method_t::GET) {
		httpmethod = "GET";
	} else if (http_method == http_method_t::POST) {
		httpmethod = "POST";
	}
	std::string m = Combine({httpmethod, (*vec)[0], query});
	std::string k = Combine({std::string(keys.consumer_sec), std::string(keys.accesstoken_sec)});

	std::string oauth_signature = "oauth_signature";
	oauth_signature += '=';
	oauth_signature += sign_hmac_sha1(m, k);

	vec->push_back(oauth_signature);
}

oauth::Request oauth::sign(const char *url, http_method_t http_method, const Keys &keys)
{
	std::vector<std::string> vec;
	split_url(url, &vec);

	process_(&vec, http_method, keys);

	if (http_method == POST) {
		Request req;
		req.post = oauth::build_url(vec, 1);
		req.url = vec.at(0);
		return req;
	} else {
		Request req;
		req.url = oauth::build_url(vec, 0);
		return req;
	}
}

