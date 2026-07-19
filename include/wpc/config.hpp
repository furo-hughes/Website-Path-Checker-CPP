#pragma once

#include <cstddef>
#include <string>

namespace wpc {
inline constexpr int kDefaultWorkers = 10;
inline constexpr double kDefaultTimeoutSeconds = 5.0;
inline constexpr const char* kDefaultListSource = "subdomains.txt";

struct ApplicationConfig {
    std::string base_url;
    std::string list_source{kDefaultListSource};
    int max_workers{kDefaultWorkers};
    double timeout_seconds{kDefaultTimeoutSeconds};
    std::string user_agent{"WebsitePathChecker/1.0"};
    bool verify_tls{true};
    double remote_list_timeout_seconds{10.0};
    std::size_t max_remote_list_size_bytes{5U * 1024U * 1024U};
    void normalize_and_validate();
};
} // namespace wpc
