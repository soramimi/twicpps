
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#pragma warning(disable:4996)
#else
#include <iconv.h>
#include <errno.h>
#include <unistd.h>
#define O_BINARY 0
#endif

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "oauth.h"
#include "webclient.h"
#include "tweet.h"
#include "charvec.h"
#include "urlencode.h"

static std::string make_authorization_string(char const *begin, char const *end)
{
	std::vector<char> vec;
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
	return to_stdstr(vec);
}

std::string make_boundary(char const *begin, char const *end)
{
	std::string boundary = "-";
	size_t pos = 0;
	while (begin + pos + boundary.size() < end) {
		if (memcmp(begin + pos, boundary.c_str(), boundary.size()) == 0) {
			int i = pos + boundary.size();
			int c = begin[i] & 0xff;
			if (isdigit(c)) {
				c = (c -'0' + 1) % 10;
			} else {
				c = 0;
			}
			boundary += c + '0';
		}
		pos++;
	}
	boundary += '-';
	return boundary;
}

bool TwitterClient::request(std::string const &url, const RequestOption &opt, std::string *reply)
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
	if (opt.method == RequestOption::GET) {
		client.get(uri);
	} else if (opt.method == RequestOption::POST) {
		WebClient::Post postdata;
		if (opt.upload_path.empty()) {
			WebClient::make_application_www_form_urlencoded(opt.post_begin, opt.post_end, &postdata);
		} else {
			std::vector<char> filedata;
			int fd = open(opt.upload_path.c_str(), O_RDONLY | O_BINARY);
			if (fd >= 0) {
				struct stat st;
				if (fstat(fd, &st) == 0 && st.st_size > 0) {
					filedata.resize(st.st_size);
					read(fd, &filedata[0], st.st_size);
				}
				::close(fd);
			}
			if (filedata.empty()) return false;

			std::string s = make_authorization_string(opt.post_begin, opt.post_end);
			client.add_header("Authorization: OAuth " + s);

			std::string boundary;
			{
				char const *begin = &filedata[0];
				char const *end = begin + filedata.size();
				boundary = make_boundary(begin, end);
			}
			std::vector<WebClient::Part> parts;
			{
				WebClient::Part part(&filedata[0], filedata.size());
				{
					WebClient::ContentDisposition cd;
					cd.type = "form-data";
					cd.name = "media";
					part.set_content_disposition(cd);
				}
				parts.push_back(part);
			}
			WebClient::make_multipart_form_data(parts, &postdata, boundary);
		}
		client.post(uri, &postdata);
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

#ifdef _WIN32

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
			if (n == (size_t)-1 && errno != E2BIG) {
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

#include "json.h"


bool TwitterClient::tweet(std::string message, std::vector<std::string> const *media_ids)
{
	if (message.empty()) {
		return false;
	}

	// convert message to utf-8
#ifdef _WIN32
	message = sjis_to_utf8(message);
#else
	//message = conv("utf-8", "euc-jp", message);
#endif

	std::string url = "https://api.twitter.com/1.1/statuses/update.json";

	url += "?status=";
	url += url_encode(message);
	if (media_ids && !media_ids->empty()) {
		std::string ids;
		for (std::string const &media_id : *media_ids) {
			if (!media_id.empty()) {
				if (!ids.empty()) {
					ids += ',';
				}
				ids += media_id;
			}
		}
		if (!ids.empty()) {
			url += "&media_ids=";
			url += ids;
		}
	}

	oauth::Request oauth_req = oauth::sign(url.c_str(), oauth::POST, keys());
	std::string res;
	RequestOption opt;
	char const *p = oauth_req.post.c_str();
	opt.set_post_data(p, p + oauth_req.post.size());
	return request(oauth_req.url, opt, &res);
}

std::string TwitterClient::upload(std::string const &path)
{
	std::string media_id;

	std::string url = "https://upload.twitter.com/1.1/media/upload.json";

	oauth::Request oauth_req = oauth::sign(url.c_str(), oauth::POST, keys());
	std::string res;
	RequestOption opt;
	char const *p = oauth_req.post.c_str();
	opt.set_post_data(p, p + oauth_req.post.size());
	opt.set_upload_path(path);
	bool ok = request(oauth_req.url, opt, &res);
	if (ok && !res.empty()) {
		JSON json;
		json.parse(res);
		media_id = json.get("/media_id").value;
	}
	return media_id;
}

