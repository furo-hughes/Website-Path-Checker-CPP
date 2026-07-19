#include <stdexcept>

#include "wpc/config.hpp"

namespace wpc {

namespace {

std::string trim(std::string text) {
  const auto first = text.find_first_not_of(" \t\r\n");
  if (first == std::string::npos) return {};

  const auto last = text.find_last_not_of(" \t\r\n");
  return text.substr(first, last - first + 1);
}

}

void ApplicationConfig::normalize_and_validate() {
  base_url = trim(base_url);
  list_source = trim(list_source);

  while (!base_url.empty() && base_url.back() == '/') base_url.pop_back();

  const auto scheme_end = base_url.find("://");
  if (scheme_end == std::string::npos ||
      (base_url.substr(0, scheme_end) != "http" &&
       base_url.substr(0, scheme_end) != "https")) {
    throw std::invalid_argument("The URL provided through -url must use HTTP or HTTPS.");
  }

  const auto authority_start = scheme_end + 3;
  const auto path_start = base_url.find_first_of("/?#", authority_start);
  const auto authority = base_url.substr(authority_start, path_start - authority_start);

  if (authority.empty()) {
    throw std::invalid_argument("The URL provided through -url must contain a valid hostname.");
  }
  if (authority.find('@') != std::string::npos) {
    throw std::invalid_argument("The base URL must not contain embedded credentials.");
  }
  if (base_url.find('?') != std::string::npos) {
    throw std::invalid_argument("The base URL must not contain a query string.");
  }
  if (base_url.find('#') != std::string::npos) {
    throw std::invalid_argument("The base URL must not contain a fragment.");
  }
  if (list_source.empty()) {
    throw std::invalid_argument("The list source must not be empty.");
  }
  if (max_workers < 1 || max_workers > 100) {
    throw std::invalid_argument("The worker count must be between 1 and 100.");
  }
  if (timeout_seconds <= 0) {
    throw std::invalid_argument("The request timeout must be greater than zero.");
  }
  if (remote_list_timeout_seconds <= 0) {
    throw std::invalid_argument("The remote list timeout must be greater than zero.");
  }
  if (max_remote_list_size_bytes == 0) {
    throw std::invalid_argument("The maximum remote list size must be greater than zero.");
  }
}

}
