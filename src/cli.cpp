#include <iostream>
#include <string>
#include <vector>

#include "wpc/checker.hpp"
#include "wpc/cli.hpp"
#include "wpc/config.hpp"
#include "wpc/console_reporter.hpp"
#include "wpc/path_loader.hpp"
#include "wpc/runner.hpp"

namespace wpc {

namespace {

constexpr int kExitSuccess = 0;
constexpr int kExitListError = 1;
constexpr int kExitArgumentError = 2;
constexpr int kExitNetworkError = 3;

void print_usage() {
  std::cout << "Usage: website-path-checker -url <HTTP_URL> [-list <FILE_OR_URL>] "
               "[--workers <1-100>] [--timeout <SECONDS>] [--no-tls-verification]\n";
}

bool read_option_value(int& index, int argument_count, char* arguments[], std::string& value) {
  if (++index >= argument_count) return false;

  value = arguments[index];
  return true;
}

bool parse_integer(const std::string& text, int& value) {
  try {
    std::size_t parsed_characters = 0;
    value = std::stoi(text, &parsed_characters);
    return parsed_characters == text.size();
  } catch (const std::exception&) {
    return false;
  }
}

bool parse_double(const std::string& text, double& value) {
  try {
    std::size_t parsed_characters = 0;
    value = std::stod(text, &parsed_characters);
    return parsed_characters == text.size();
  } catch (const std::exception&) {
    return false;
  }
}

}

int run_cli(int argc, char* argv[]) {
  ConsoleReporter::print_authorization_notice();

  ApplicationConfig config;
  bool has_url = false;

  for (int index = 1; index < argc; ++index) {
    const std::string argument = argv[index];
    const auto separator_position = argument.find('=');
    const auto option = argument.substr(0, separator_position);

    if (option == "-h" || option == "--help") {
      print_usage();
      return kExitSuccess;
    }

    if (option == "--no-tls-verification") {
      if (separator_position != std::string::npos) {
        ConsoleReporter::print_error("The --no-tls-verification option does not accept a value.");
        return kExitArgumentError;
      }

      config.verify_tls = false;
      continue;
    }

    std::string value;
    if (separator_position == std::string::npos) {
      if (!read_option_value(index, argc, argv, value)) {
        ConsoleReporter::print_error("Missing value for " + option);
        return kExitArgumentError;
      }
    } else {
      value = argument.substr(separator_position + 1);
    }

    if (option == "-url") {
      config.base_url = value;
      has_url = true;
    } else if (option == "-list") {
      config.list_source = value;
    } else if (option == "--workers") {
      if (!parse_integer(value, config.max_workers)) {
        ConsoleReporter::print_error("--workers must be an integer.");
        return kExitArgumentError;
      }
    } else if (option == "--timeout") {
      if (!parse_double(value, config.timeout_seconds)) {
        ConsoleReporter::print_error("--timeout must be a number.");
        return kExitArgumentError;
      }
    } else {
      ConsoleReporter::print_error("Unknown argument: " + option);
      print_usage();
      return kExitArgumentError;
    }
  }

  if (!has_url) {
    ConsoleReporter::print_error("The -url argument is required.");
    print_usage();
    return kExitArgumentError;
  }

  try {
    config.normalize_and_validate();
  } catch (const std::exception& error) {
    ConsoleReporter::print_error(error.what());
    return kExitArgumentError;
  }

  if (!config.verify_tls) {
    ConsoleReporter::print_warning("TLS certificate verification is disabled.");
  }

  std::vector<std::string> paths;
  try {
    paths = load_paths(config.list_source,
                       config.user_agent,
                       config.remote_list_timeout_seconds,
                       config.verify_tls,
                       config.max_remote_list_size_bytes);
  } catch (const PathListError& error) {
    ConsoleReporter::print_error(error.what());
    return kExitListError;
  }

  if (paths.empty()) {
    ConsoleReporter::print_warning("The selected path list does not contain any valid entries.");
    return kExitSuccess;
  }

  ConsoleReporter::print_start_information(config.base_url,
                                           paths.size(),
                                           config.max_workers,
                                           config.timeout_seconds);

  const auto source_type = is_remote_source(config.list_source) ? "remote URL" : "local file";
  ConsoleReporter::print_information("Path-list source (" + std::string(source_type) + "): " +
                                     config.list_source);

  WebsitePathChecker checker(config);
  const auto summary = CheckRunner(checker, config.max_workers).run(paths);
  ConsoleReporter::print_summary(summary);

  return summary.network_errors == 0 ? kExitSuccess : kExitNetworkError;
}

}
