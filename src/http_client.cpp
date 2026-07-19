#include <cctype>
#include <memory>
#include <mutex>
#include <string>

#include <curl/curl.h>

#include "wpc/http_client.hpp"

namespace wpc {

namespace {

std::once_flag curl_initialization_flag;

void ensure_curl_initialized() {
  std::call_once(curl_initialization_flag, [] {
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
      throw HttpError("Could not initialize libcurl.");
    }
  });
}

struct TransferData {
  HttpResponse response;
  std::optional<std::size_t> maximum_body_size;
  bool body_too_large{false};
};

size_t write_body(char* bytes, size_t element_size, size_t element_count, void* context) {
  auto& data = *static_cast<TransferData*>(context);
  const auto byte_count = element_size * element_count;

  if (data.maximum_body_size && data.response.body.size() + byte_count > *data.maximum_body_size) {
    data.body_too_large = true;
    return 0;
  }

  data.response.body.append(bytes, byte_count);
  return byte_count;
}

size_t read_header(char* bytes, size_t element_size, size_t element_count, void* context) {
  auto& data = *static_cast<TransferData*>(context);
  const std::string header_line(bytes, element_size * element_count);
  const auto separator_position = header_line.find(':');

  if (separator_position == std::string::npos) return element_size * element_count;

  auto name = header_line.substr(0, separator_position);
  for (auto& character : name) {
    character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
  }

  if (name != "location") return element_size * element_count;

  const auto value = header_line.substr(separator_position + 1);
  const auto first = value.find_first_not_of(" \t");
  const auto last = value.find_last_not_of(" \t\r\n");
  data.response.location = first == std::string::npos ? "" : value.substr(first, last - first + 1);

  return element_size * element_count;
}

[[noreturn]] void throw_http_error(CURLcode code, CURL* handle) {
  char* effective_url = nullptr;
  curl_easy_getinfo(handle, CURLINFO_EFFECTIVE_URL, &effective_url);

  const auto url_suffix = effective_url == nullptr ? "" : " for " + std::string(effective_url);
  const auto details = ": " + std::string(curl_easy_strerror(code));

  if (code == CURLE_OPERATION_TIMEDOUT) {
    throw HttpTimeoutError("Request timed out" + url_suffix + ".");
  }
  if (code == CURLE_PEER_FAILED_VERIFICATION || code == CURLE_SSL_CONNECT_ERROR) {
    throw HttpTlsError("TLS certificate validation failed" + url_suffix + details);
  }
  if (code == CURLE_COULDNT_CONNECT || code == CURLE_COULDNT_RESOLVE_HOST) {
    throw HttpConnectionError("Connection failed" + url_suffix + details);
  }

  throw HttpError("HTTP request failed" + url_suffix + details);
}

}

HttpResponse HttpClient::perform(const HttpRequest& request) const {
  ensure_curl_initialized();

  CURL* raw_handle = curl_easy_init();
  if (raw_handle == nullptr) {
    throw HttpError("Could not create HTTP request handle.");
  }

  const std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> handle(raw_handle,
                                                                     &curl_easy_cleanup);
  TransferData data{{}, request.maximum_body_size};

  curl_easy_setopt(raw_handle, CURLOPT_URL, request.url.c_str());
  curl_easy_setopt(raw_handle, CURLOPT_USERAGENT, request.user_agent.c_str());
  curl_easy_setopt(raw_handle, CURLOPT_TIMEOUT_MS,
                   static_cast<long>(request.timeout_seconds * 1000.0));
  curl_easy_setopt(raw_handle, CURLOPT_FOLLOWLOCATION, request.follow_redirects ? 1L : 0L);
  curl_easy_setopt(raw_handle, CURLOPT_MAXREDIRS, 10L);
  curl_easy_setopt(raw_handle, CURLOPT_NOSIGNAL, 1L);
  curl_easy_setopt(raw_handle, CURLOPT_SSL_VERIFYPEER, request.verify_tls ? 1L : 0L);
  curl_easy_setopt(raw_handle, CURLOPT_SSL_VERIFYHOST, request.verify_tls ? 2L : 0L);
  curl_easy_setopt(raw_handle, CURLOPT_NOBODY, request.method == RequestMethod::Head ? 1L : 0L);
  curl_easy_setopt(raw_handle, CURLOPT_WRITEFUNCTION, write_body);
  curl_easy_setopt(raw_handle, CURLOPT_WRITEDATA, &data);
  curl_easy_setopt(raw_handle, CURLOPT_HEADERFUNCTION, read_header);
  curl_easy_setopt(raw_handle, CURLOPT_HEADERDATA, &data);

  const auto result = curl_easy_perform(raw_handle);
  if (data.body_too_large) {
    throw HttpBodyTooLargeError("Response body exceeds the maximum allowed size.");
  }
  if (result != CURLE_OK) throw_http_error(result, raw_handle);

  curl_easy_getinfo(raw_handle, CURLINFO_RESPONSE_CODE, &data.response.status_code);
  return data.response;
}

}
