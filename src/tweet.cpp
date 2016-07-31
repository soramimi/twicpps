
#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#else
#include <iconv.h>
#include <errno.h>
#endif

#include <stdio.h>
#include <string.h>

#include "oauth.h"
#include "webclient.h"
#include "tweet.h"

static bool http_request(const char *u, const char *p, std::string *reply = 0)
{
	if (reply) {
		reply->clear();
	}
	WebContext webcx;
#if 0
	webcx.load_cacert("C:\\develop\\twicpps\\cacert.pem");
#endif
	WebClient client(&webcx);
	WebClient::URL uri(u);
	if (p) {
		WebClient::Post post;
		WebClient::make_application_www_form_urlencoded(p, strlen(p), &post);
		client.post(uri, &post);
	} else {
		client.get(uri);
	}
	WebClient::Response const &res = client.response();
	if (res.header.size() > 0) {
		int a, b, c;
		sscanf(res.header[0].c_str(), "HTTP/%u.%u %u", &a, &b, &c);
		if (c == 200) {
			if (reply && !res.content.empty()) {
				char const *p = (char const *)&res.content[0];
				*reply = std::string(p, p + res.content.size());
			}
			return true;
		}
	}
	return false;
}


#ifdef WIN32

std::string sjis_to_utf8(std::string const &message)
{
	int n;
	wchar_t ucs2[1000];
	char utf8[1000];
	n = MultiByteToWideChar(CP_ACP, 0, message.c_str(), message.size(), ucs2, 1000);
	n = WideCharToMultiByte(CP_UTF8, 0, ucs2, n, utf8, 1000, 0, 0);
	return std::string(utf8, n);
}

#else

std::string conv(char const *dstenc, char const *srcenc, std::string const &src)
{
	std::vector<char> out;
	iconv_t cd = iconv_open(dstenc, srcenc);
	if (cd != (iconv_t)-1) {
		char tmp[65536];
		size_t space = sizeof(tmp);
		char *inbuf = (char *)src.c_str();
		size_t inleft = src.size();
		size_t n;
		while (inleft > 0) {
			char *outbuf = tmp;
			size_t outleft = space;
			n = iconv(cd, &inbuf, &inleft, &outbuf, &outleft);
			if (n == -1 && errno != E2BIG) {
				break;
			}
			n = space - outleft;
			out.insert(out.end(), tmp, tmp + n);
		}
		iconv_close(cd);
	}
	if (out.empty()) {
		return std::string();
	}
	return std::string(&out[0], out.size());
}

#endif



bool TwitterClient::tweet(std::string message)
{
	if (message.empty()) {
		return false;
	}

	// convert message to utf-8
#ifdef WIN32
	message = sjis_to_utf8(message);
#else
	//message = conv("utf-8", "euc-jp", message);
#endif

//	std::string uri = "http://api.twitter.com/statuses/update.xml";
//	std::string uri = "http://api.twitter.com/1/statuses/update.xml"; // 2012-11-15
//	std::string uri = "http://api.twitter.com/1.1/statuses/update.json"; // 2013-02-26
	std::string uri = "https://api.twitter.com/1.1/statuses/update.json"; // 2014-01-19

	uri += "?status=";
	uri += oauth_url_escape(message.c_str());

	std::string post;
	std::string request = oauth_sign_url2(uri.c_str(), &post, OA_HMAC, 0, c_key(), c_sec(), t_key(), t_sec());
	std::string res;
	bool ok = http_request(request.c_str(), post.c_str(), &res);

	return ok;
}

