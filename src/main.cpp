#include <stdio.h>

#include "tweet.h"
#include "webclient.h"
#include "oauth.h"

//#include "../keys/mykey.h"
//#include "../keys/soramimi_jp.h"
#include "../keys/soramimi_jp_bot.h"

static void str_append(std::string *to, std::string const &s)
{
	if (!to->empty()) {
		*to += ' ';
	}
	*to += s;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		return 1;
	}

	std::string message;
	{
		bool f_stdin = false;
		bool f_base64 = false;
		bool f_separator = false;
		int i = 1;
		while (i < argc) {
			std::string arg = argv[i];
			i++;
			if (f_separator) {
				str_append(&message, arg);
			} else if (arg[0] == '-') {
				if (arg == "--") {
					f_separator = true;
				} else if (arg == "-stdin") {
					f_stdin = true;
				} else if (arg == "-base64") {
					f_base64 = true;
				} else {
					fprintf(stderr, "unknown option: %s\n", arg.c_str());
				}
			} else {
				if (f_base64) {
					message += arg;
				} else {
					str_append(&message, arg);
				}
			}
		}
		if (f_stdin) {
			char tmp[1000];
			int n = fread(tmp, 1, sizeof(tmp), stdin);
			message = std::string(tmp, n);
		}
		if (f_base64) {
			message = oauth_decode_base64(message.c_str());
		}
	}

	WebClient::initialize();
	TwitterClient tc(consumer_key, consumer_sec, accesstoken, accesstoken_sec);
	bool ok = tc.tweet(message);

	return ok ? 0 : 1;
}

