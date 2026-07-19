#pragma once
#include <vector>
#include "wpc/checker.hpp"
#include "wpc/console_reporter.hpp"
namespace wpc { class CheckRunner { public: CheckRunner(const WebsitePathChecker& checker, int workers) : checker_(checker), workers_(workers) {} [[nodiscard]] ResultSummary run(const std::vector<std::string>& paths) const; private: const WebsitePathChecker& checker_; int workers_; }; }
