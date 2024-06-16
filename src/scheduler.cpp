#include "scheduler.h"

#include "log.h"

namespace myserver {
static mylog::Logger::ptr g_logger = MYLOG_LOG_NAME("system");

Scheduler::Scheduler(size_t threads, bool use_caller, const std::string& name) {}

Scheduler::~Scheduler() {}

Scheduler* Scheduler::GetThis() { return nullptr; }

Fiber* Scheduler::GetMainFiber() { return nullptr; }

void Scheduler::start() {}

void Scheduler::stop() {}
}  // namespace myserver