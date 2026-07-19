#pragma once

#include "wpc/config.hpp"
#include "wpc/models.hpp"

namespace wpc {
class WebsitePathChecker {
public:
    explicit WebsitePathChecker(ApplicationConfig config) : config_(std::move(config)) {}
    [[nodiscard]] CheckTask create_task(const std::string& path) const;
    [[nodiscard]] CheckResult check(const CheckTask& task) const;
private: ApplicationConfig config_;
};
} // namespace wpc
