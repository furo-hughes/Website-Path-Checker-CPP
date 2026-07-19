#pragma once

#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>

#include "wpc/models.hpp"

namespace wpc {
struct HttpRequest {
    std::string url;
    RequestMethod method{RequestMethod::Head};
    double timeout_seconds{5.0};
    std::string user_agent;
    bool verify_tls{true};
    bool follow_redirects{false};
    std::optional<std::size_t> maximum_body_size;
};
struct HttpResponse {
    long status_code{0};
    std::optional<std::string> location;
    std::string body;
};
class HttpError : public std::runtime_error { public: using std::runtime_error::runtime_error; };
class HttpTimeoutError : public HttpError { public: using HttpError::HttpError; };
class HttpTlsError : public HttpError { public: using HttpError::HttpError; };
class HttpConnectionError : public HttpError { public: using HttpError::HttpError; };
class HttpBodyTooLargeError : public HttpError { public: using HttpError::HttpError; };
class HttpClient { public: HttpResponse perform(const HttpRequest& request) const; };
} // namespace wpc
