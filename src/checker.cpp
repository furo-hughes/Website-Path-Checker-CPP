#include <chrono>
#include <utility>

#include "wpc/checker.hpp"
#include "wpc/http_client.hpp"

namespace wpc {

namespace {

std::string resolve_redirect(const std::string& source,
                             const std::optional<std::string>& location) {
  if (!location || location->empty() || location->find("://") != std::string::npos) {
    return location.value_or("");
  }

  const auto authority_end = source.find('/', source.find("://") + 3);
  if (location->front() == '/') return source.substr(0, authority_end) + *location;

  return source.substr(0, source.find_last_of('/') + 1) + *location;
}

}

WebsitePathChecker::WebsitePathChecker(ApplicationConfig config) : config_(std::move(config)) {}

CheckTask WebsitePathChecker::create_task(const std::string& path) const {
  const auto first_path_character = path.find_first_not_of('/');
  const auto normalized_path =
      first_path_character == std::string::npos ? "" : path.substr(first_path_character);

  return {normalized_path, config_.base_url + "/" + normalized_path};
}

CheckResult WebsitePathChecker::check(const CheckTask& task) const {
  const auto start = std::chrono::steady_clock::now();
  auto method = RequestMethod::Head;
  const auto elapsed = [&] {
    return std::chrono::duration<double, std::milli>(
               std::chrono::steady_clock::now() - start)
        .count();
  };

  try {
    HttpClient client;
    auto response = client.perform(
        {task.url, method, config_.timeout_seconds, config_.user_agent,
         config_.verify_tls, false, std::nullopt});

    if (response.status_code == 405) {
      method = RequestMethod::Get;
      response = client.perform(
          {task.url, method, config_.timeout_seconds, config_.user_agent,
           config_.verify_tls, false, std::nullopt});
    }

    const auto category = categorize_status(response.status_code);
    const auto redirect_target = category == ResultCategory::Redirection
                                     ? std::optional(resolve_redirect(task.url, response.location))
                                     : std::nullopt;

    return {task, method, category, response.status_code, redirect_target, elapsed(), std::nullopt};
  } catch (const HttpTimeoutError&) {
    return {task, method, ResultCategory::NetworkError, {}, {}, elapsed(),
            "Request timed out after " + std::to_string(config_.timeout_seconds) + " seconds."};
  } catch (const HttpError& error) {
    return {task, method, ResultCategory::NetworkError, {}, {}, elapsed(), error.what()};
  }
}

}
