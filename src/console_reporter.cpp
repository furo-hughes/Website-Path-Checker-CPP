#include <iomanip>
#include <iostream>

#include "wpc/console_reporter.hpp"

namespace wpc {

namespace {

constexpr const char* kReset = "\033[0m";
constexpr const char* kGreen = "\033[92m";
constexpr const char* kYellow = "\033[93m";
constexpr const char* kRed = "\033[91m";
constexpr const char* kCyan = "\033[96m";
constexpr const char* kMagenta = "\033[95m";

const char* category_color(ResultCategory category) {
  switch (category) {
    case ResultCategory::Success:
      return kGreen;
    case ResultCategory::Redirection:
      return kCyan;
    case ResultCategory::ClientError:
      return kYellow;
    case ResultCategory::ServerError:
    case ResultCategory::NetworkError:
      return kRed;
    case ResultCategory::Unknown:
      return kMagenta;
  }

  return kMagenta;
}

}

void ResultSummary::add(const CheckResult& result) {
  ++total;

  switch (result.category) {
    case ResultCategory::Success:
      ++successful;
      break;
    case ResultCategory::Redirection:
      ++redirections;
      break;
    case ResultCategory::ClientError:
      ++client_errors;
      break;
    case ResultCategory::ServerError:
      ++server_errors;
      break;
    case ResultCategory::NetworkError:
      ++network_errors;
      break;
    case ResultCategory::Unknown:
      ++unknown;
      break;
  }
}

void ConsoleReporter::print_authorization_notice() {
  const std::string border(80, '=');
  std::cout << kYellow << border << '\n'
            << "AUTHORIZED USE ONLY\n"
            << "Only use this program against systems for which you have explicit permission "
               "to perform automated checks.\n"
            << border << kReset << "\n\n";
}

void ConsoleReporter::print_start_information(const std::string& base_url,
                                              std::size_t path_count,
                                              int worker_count,
                                              double timeout_seconds) {
  std::cout << "Website Path Checker\n"
            << "Base URL:       " << base_url << '\n'
            << "Unique paths:   " << path_count << '\n'
            << "Workers:        " << worker_count << '\n'
            << "Timeout:        " << timeout_seconds << " seconds\n"
            << std::string(80, '-') << '\n';
}

void ConsoleReporter::print_result(const CheckResult& result) {
  std::cout << category_color(result.category);

  if (result.has_network_error()) {
    std::cout << "[NETWORK ERROR]";
  } else {
    std::cout << '[' << result.status_code.value_or(0) << ']';
  }

  std::cout << kReset << ' ' << std::setw(4) << std::left << to_string(result.method) << ' '
            << result.task.url << " (" << std::fixed << std::setprecision(0)
            << result.response_time_ms.value_or(0.0) << " ms)";

  if (result.has_network_error()) {
    std::cout << " - " << result.error_message.value_or("Unknown network error");
  } else if (result.is_redirection()) {
    std::cout << " -> " << kCyan
              << result.redirect_target.value_or("No Location header") << kReset;
  }

  std::cout << '\n';
}

void ConsoleReporter::print_summary(const ResultSummary& summary) {
  std::cout << std::string(80, '-') << "\n"
            << "Summary\n"
            << "Checked:          " << summary.total << '\n'
            << kGreen << "Successful:       " << summary.successful << '\n'
            << kCyan << "Redirections:     " << summary.redirections << '\n'
            << kYellow << "Client errors:    " << summary.client_errors << '\n'
            << kRed << "Server errors:    " << summary.server_errors << '\n'
            << "Network errors:   " << summary.network_errors << kReset << '\n';

  if (summary.unknown > 0) {
    std::cout << kMagenta << "Unknown:          " << summary.unknown << kReset << '\n';
  }
}

void ConsoleReporter::print_error(const std::string& message) {
  std::cerr << kRed << "Error: " << kReset << message << '\n';
}

void ConsoleReporter::print_warning(const std::string& message) {
  std::cout << kYellow << "Warning: " << kReset << message << '\n';
}

void ConsoleReporter::print_information(const std::string& message) {
  std::cout << kCyan << "Info: " << kReset << message << '\n';
}

}
