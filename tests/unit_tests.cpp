#include <cassert>
#include <iostream>

#include "wpc/config.hpp"
#include "wpc/models.hpp"
#include "wpc/path_loader.hpp"

int main() {
  using namespace wpc;
  ApplicationConfig config{"  https://example.com/  ", "  paths.txt  "};
  config.normalize_and_validate();
  assert(config.base_url == "https://example.com");
  assert(config.list_source == "paths.txt");
  assert(normalize_path(" ///admin/users ") == "admin/users");
  const auto paths =
      parse_paths({"# comment", " /admin ", "admin", "//login", "", "/"});
  assert((paths == std::vector<std::string>{"admin", "login", ""}));
  assert(categorize_status(200) == ResultCategory::Success);
  assert(categorize_status(301) == ResultCategory::Redirection);
  assert(categorize_status(404) == ResultCategory::ClientError);
  assert(categorize_status(500) == ResultCategory::ServerError);
  bool rejected = false;
  try {
    ApplicationConfig{"ftp://example.com"}.normalize_and_validate();
  } catch (const std::invalid_argument&) {
    rejected = true;
  }
  assert(rejected);
  std::cout << "All unit tests passed.\n";
}
