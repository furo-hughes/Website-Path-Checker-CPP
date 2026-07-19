#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <stdexcept>
#include <vector>

namespace wpc {
class PathListError : public std::runtime_error { public: using std::runtime_error::runtime_error; };
[[nodiscard]] bool is_remote_source(std::string_view source);
[[nodiscard]] std::string normalize_path(std::string_view value);
[[nodiscard]] std::vector<std::string> parse_paths(const std::vector<std::string>& lines);
[[nodiscard]] std::vector<std::string> load_paths(const std::string& source, const std::string& user_agent,
    double timeout_seconds, bool verify_tls, std::size_t max_size_bytes);
} // namespace wpc
