#include <exception>
#include <future>

#include "wpc/runner.hpp"

namespace wpc {

namespace {

CheckResult resolve_future(std::future<CheckResult>& future) {
  try {
    return future.get();
  } catch (const std::exception& error) {
    return {{}, RequestMethod::Head, ResultCategory::NetworkError, {}, {}, {},
            std::string("Unexpected worker failure: ") + error.what()};
  }
}

void process_result(ResultSummary& summary, const CheckResult& result) {
  summary.add(result);
  ConsoleReporter::print_result(result);
}

}

CheckRunner::CheckRunner(const WebsitePathChecker& checker, int workers)
    : checker_(checker), workers_(workers) {}

ResultSummary CheckRunner::run(const std::vector<std::string>& paths) const {
  ResultSummary summary;
  std::vector<std::future<CheckResult>> active_tasks;

  for (const auto& path : paths) {
    const auto task = checker_.create_task(path);
    active_tasks.push_back(std::async(std::launch::async,
                                      [this, task] { return checker_.check(task); }));

    if (active_tasks.size() == static_cast<std::size_t>(workers_)) {
      const auto result = resolve_future(active_tasks.front());
      active_tasks.erase(active_tasks.begin());
      process_result(summary, result);
    }
  }

  for (auto& task : active_tasks) {
    process_result(summary, resolve_future(task));
  }

  return summary;
}

}
