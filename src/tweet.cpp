
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
#include "charvec.h"
#include "urlencode.h"

static bool http_request(std::string const &url, std::string const *post, std::string *reply, bool upload)
{
	if (reply) {
		reply->clear();
	}
	WebContext webcx;
#if 0
	webcx.load_cacert("C:\\develop\\twicpps\\cacert.pem");
#endif
	WebClient client(&webcx);
	WebClient::URL uri(url.c_str());
	if (post) {
		WebClient::Post postdata;
		if (upload) {
			auto make_authorization_from_post_data = [](std::string const &post){
				std::vector<char> vec;
				{
					char const *begin = post.c_str();
					char const *end = begin + post.size();
					char const *ptr = begin;
					char const *left = begin;
					while (1) {
						int c = -1;
						if (ptr < end) {
							c = *ptr & 0xff;
						}
						if (c == '&' || c == -1) {
							if (left < ptr) {
								if (!vec.empty()) {
									vec.push_back(',');
								}
								std::string s(left, ptr);
								print(&vec, s);
							}
							if (c == -1) break;
							ptr++;
							left = ptr;
						} else {
							ptr++;
						}
					}
				}
				return to_stdstr(&vec);
			};
			std::string s = make_authorization_from_post_data(*post);

			client.add_header("Authorization: OAuth " + s);
			WebClient::make_multipart_form_data(post->c_str(), post->size(), &postdata);

		} else {
			WebClient::make_application_www_form_urlencoded(post->c_str(), post->size(), &postdata);
		}
		client.post(uri, &postdata);
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
#if 1
	if (message.empty()) {
		return false;
	}

	// convert message to utf-8
#ifdef WIN32
	message = sjis_to_utf8(message);
#else
	//message = conv("utf-8", "euc-jp", message);
#endif

	std::string url = "https://api.twitter.com/1.1/statuses/update.json";

	url += "?status=";
	url += url_encode(message);

	oauth::Request request = oauth::sign(url.c_str(), oauth::POST, keys());
	std::string res;
	bool ok = http_request(request.url, &request.post, &res, false);
	puts(res.c_str());
#else

	std::string url = "https://upload.twitter.com/1.1/media/upload.json";

	std::string post;
	oauth::Request request = oauth::sign(url.c_str(), oauth::POST, keys());
	std::string res;
	bool ok = http_request(request.url, &request.post, &res, true);
	puts(res.c_str());
#endif

	return ok;
}

