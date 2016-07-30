
#ifndef WEBCLIENT_H_
#define WEBCLIENT_H_

#include <vector>
#include <string>
#include <functional>

#define USE_OPENSSL 1

class WebContext;
class WebClient;

#define CT_APPLICATION_X_WWW_FORM_URLENCODED "application/x-www-form-urlencoded"
#define CT_MULTIPART_FORM_DATA "multipart/form-data"

class WebClientHandler {
protected:
	void abort(std::string const &message = std::string());
public:
	virtual ~WebClientHandler()
	{
	}
	virtual void checkHeader(WebClient * /*wc*/)
	{
	}
	virtual void checkContent(char const * /*ptr*/, size_t /*len*/)
	{
	}
};

class WebClient {
public:
	class URL {
	private:
		std::string scheme_;
		std::string host_;
		int port_ = 0;
		std::string path_;
	public:
		URL(char const *str);
		std::string const &scheme() const { return scheme_; }
		std::string const &host() const { return host_; }
		int port() const { return port_; }
		std::string const &path() const { return path_; }
		bool isssl() const;
	};

	class Error {
	private:
		std::string msg_;
	public:
		Error()
		{
		}
		Error(std::string const &message)
			: msg_(message)
		{
		}
		virtual ~Error()
		{
		}
		std::string message() const
		{
			return msg_;
		}
	};
	struct Response {
		int code = 0;
		struct Version {
			unsigned int hi = 0;
			unsigned int lo = 0;
		} version;
		std::vector<std::string> header;
		std::vector<char> content;
	};
	struct Post {
		std::string content_type;
		std::string boundary;
		std::vector<char> data;
	};
	struct RequestOption {
		WebClientHandler *handler = nullptr;
		bool keep_alive = true;
	};
private:
	struct Private;
	Private *pv;
	void clear_error();
	static int get_port(URL const *uri, char const *scheme, char const *protocol);
	void set_default_header(URL const &uri, Post const *post, const RequestOption &opt);
	std::string make_http_request(URL const &uri, Post const *post);
	void parse_http_header(char const *begin, char const *end, std::vector<std::string> *header);
	void parse_http_header(char const *begin, char const *end, Response *out);
	bool http_get(URL const &uri, Post const *post, RequestOption const &opt, std::vector<char> *out);
	bool https_get(URL const &uri, Post const *post, RequestOption const &opt, std::vector<char> *out);
	void get(URL const &uri, Post const *post, Response *out, WebClientHandler *handler);
	static void parse_header(std::vector<std::string> const *header, WebClient::Response *res);
	static std::string header_value(std::vector<std::string> const *header, const std::string &name);
	void append(const char *ptr, size_t len, std::vector<char> *out, WebClientHandler *handler);
	void on_end_header(const std::vector<char> *vec, WebClientHandler *handler);
	void receive_(const RequestOption &opt, std::function<int (char *, int)>, std::vector<char> *out);
	void output_debug_string(const char *str);
	void output_debug_strings(const std::vector<std::string> &vec);
	static void cleanup();
public:
	static void initialize();
	WebClient(WebContext *webcx);
	~WebClient();
	WebClient(WebClient const &) = delete;
	void operator = (WebClient const &) = delete;

	Error const &error() const;
	int get(URL const &uri, WebClientHandler *handler = nullptr);
	int post(URL const &uri, Post const *post, WebClientHandler *handler = nullptr);
	void close();
	void add_header(std::string const &text);
	Response const &response() const;
	std::string header_value(std::string const &name) const;
	std::string content_type() const;
	size_t content_length() const;
	const char *content_data() const;
	static void make_application_www_form_urlencoded(const char *__data__, size_t size, WebClient::Post *out);
	static void make_multipart_form_data(const char *__data__, size_t size, WebClient::Post *out);
};

class WebContext {
	friend class WebClient;
private:
	struct Private;
	Private *pv;
	WebContext(WebContext const &r);
	void operator = (WebContext const &r);
public:
	WebContext();
	~WebContext();

	void set_keep_alive_enabled(bool f);

	bool load_cacert(char const *path);
};

#endif
