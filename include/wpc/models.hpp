#pragma once

#include <optional>
#include <string>

namespace wpc {

enum class RequestMethod { Head, Get };
enum class ResultCategory { Success, Redirection, ClientError, ServerError, NetworkError, Unknown };

struct CheckTask { std::string path; std::string url; };

struct CheckResult {
    CheckTask task;
    RequestMethod method{RequestMethod::Head};
    ResultCategory category{ResultCategory::Unknown};
    std::optional<long> status_code;
    std::optional<std::string> redirect_target;
    std::optional<double> response_time_ms;
    std::optional<std::string> error_message;
    [[nodiscard]] bool received_http_response() const { return status_code.has_value(); }
    [[nodiscard]] bool is_successful() const { return category == ResultCategory::Success; }
    [[nodiscard]] bool is_redirection() const { return category == ResultCategory::Redirection; }
    [[nodiscard]] bool has_network_error() const { return category == ResultCategory::NetworkError; }
};

[[nodiscard]] inline const char* to_string(RequestMethod method) { return method == RequestMethod::Head ? "HEAD" : "GET"; }
[[nodiscard]] inline ResultCategory categorize_status(long code) {
    if (code >= 200 && code < 300) return ResultCategory::Success;
    if (code >= 300 && code < 400) return ResultCategory::Redirection;
    if (code >= 400 && code < 500) return ResultCategory::ClientError;
    if (code >= 500 && code < 600) return ResultCategory::ServerError;
    return ResultCategory::Unknown;
}
} // namespace wpc
