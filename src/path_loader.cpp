#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_set>

#include "wpc/http_client.hpp"
#include "wpc/path_loader.hpp"

namespace wpc {

namespace {

std::string trim(std::string_view value) {
  const auto first = value.find_first_not_of(" \t\r\n");
  if (first == std::string_view::npos) return {};

  const auto last = value.find_last_not_of(" \t\r\n");
  return std::string(value.substr(first, last - first + 1));
}

std::vector<std::string> split_lines(const std::string& text) {
  std::vector<std::string> lines;
  std::istringstream input(text);

  for (std::string line; std::getline(input, line);) {
    lines.push_back(std::move(line));
  }

  return lines;
}

}

bool is_remote_source(std::string_view source) {
  const auto scheme_position = source.find("://");
  if (scheme_position == std::string_view::npos) return false;

  const auto scheme = source.substr(0, scheme_position);
  return scheme == "http" || scheme == "https" || scheme == "HTTP" || scheme == "HTTPS";
}

std::string normalize_path(std::string_view value) {
  auto path = trim(value);
  const auto first_path_character = path.find_first_not_of('/');
  return first_path_character == std::string::npos ? "" : path.substr(first_path_character);
}

std::vector<std::string> parse_paths(const std::vector<std::string>& lines) {
  std::vector<std::string> paths;
  std::unordered_set<std::string> seen_paths;

  for (const auto& raw_line : lines) {
    const auto line = trim(raw_line);
    if (line.empty() || line.front() == '#') continue;

    auto path = normalize_path(line);
    if (seen_paths.insert(path).second) paths.push_back(std::move(path));
  }

  return paths;
}

std::vector<std::string> load_paths(const std::string& source,
                                    const std::string& user_agent,
                                    double timeout_seconds,
                                    bool verify_tls,
                                    std::size_t max_size_bytes) {
  std::string content;

  if (is_remote_source(source)) {
    try {
      const auto response = HttpClient{}.perform(
          {source, RequestMethod::Get, timeout_seconds, user_agent, verify_tls, true,
           max_size_bytes});

      if (response.status_code < 200 || response.status_code >= 300) {
        throw PathListError("Remote path-list server returned HTTP " +
                            std::to_string(response.status_code) + ": " + source);
      }

      content = response.body;
    } catch (const HttpBodyTooLargeError&) {
      throw PathListError("Remote path list exceeds the maximum allowed size of " +
                          std::to_string(max_size_bytes) + " bytes.");
    } catch (const HttpError& error) {
      throw PathListError("Could not download the remote path list: " +
                          std::string(error.what()));
    }
  } else {
    const std::filesystem::path file_path(source);

    if (!std::filesystem::exists(file_path)) {
      throw PathListError("Path-list file does not exist: " +
                          std::filesystem::absolute(file_path).string());
    }
    if (!std::filesystem::is_regular_file(file_path)) {
      throw PathListError("Path-list source is not a regular file: " +
                          std::filesystem::absolute(file_path).string());
    }

    std::ifstream input(file_path, std::ios::binary);
    if (!input) {
      throw PathListError("Could not read path-list file: " +
                          std::filesystem::absolute(file_path).string());
    }

    content.assign(std::istreambuf_iterator<char>(input), {});
  }

  if (content.size() >= 3 && static_cast<unsigned char>(content[0]) == 0xEF &&
      static_cast<unsigned char>(content[1]) == 0xBB &&
      static_cast<unsigned char>(content[2]) == 0xBF) {
    content.erase(0, 3);
  }

  return parse_paths(split_lines(content));
}

}
