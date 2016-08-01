#include <stdio.h>
#include <sys/stat.h>

#include "tweet.h"
#include "webclient.h"
#include "oauth.h"
#include "base64.h"

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
	std::vector<std::string> media_paths;
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
				} else if (arg == "-media") {
					if (i < argc) {
						media_paths.push_back(argv[i]);
						i++;
					}
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
			message = base64_decode(message);
		}
	}

	bool ok = true;

	for (std::string const &path : media_paths) {
		struct stat st;
		if (stat(path.c_str(), &st) == 0 && (st.st_mode & S_IFMT) == S_IFREG) {
			// ok
		} else {
			fprintf(stderr, "media file is invalid: %s\n", path.c_str());
		}
	}
	if (!ok) return 1;

	ok = false;

	WebClient::initialize();
	TwitterClient tc(consumer_key, consumer_sec, accesstoken, accesstoken_sec);

	std::vector<std::string> media_ids;
	for (std::string const &path : media_paths) {
		media_ids.push_back(tc.upload(path));
	}

	ok = tc.tweet(message, &media_ids);

	return ok ? 0 : 1;
}

