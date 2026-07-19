#pragma once

#include <cstddef>
#include <string>
#include "wpc/models.hpp"
namespace wpc {
struct ResultSummary { int total{}; int successful{}; int redirections{}; int client_errors{}; int server_errors{}; int network_errors{}; int unknown{}; void add(const CheckResult& result); };
class ConsoleReporter {
public:
    static void print_authorization_notice(); static void print_start_information(const std::string&, std::size_t, int, double);
    static void print_result(const CheckResult&); static void print_summary(const ResultSummary&);
    static void print_error(const std::string&); static void print_warning(const std::string&); static void print_information(const std::string&);
};
} // namespace wpc
