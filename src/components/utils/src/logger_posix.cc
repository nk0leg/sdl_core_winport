/*
 * Copyright (c) 2015, Ford Motor Company
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the name of the Ford Motor Company nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include "utils/logger.h"
#include "utils/log_message_loop_thread.h"
#include "config_profile/profile.h"

namespace {
bool is_logs_enabled = false;
logger::LogMessageLoopThread* message_loop_thread = NULL;
}  // namespace

namespace logger {

bool init_logger(const std::string& ini_file_name) {
  log4cxx::PropertyConfigurator::configure(ini_file_name);
  if (!message_loop_thread) {
    message_loop_thread = new LogMessageLoopThread();
  }
  set_logs_enabled(profile::Profile::instance()->logs_enabled());
  return true;
}

void deinit_logger() {
  CREATE_LOGGERPTR_LOCAL(logger_, "Logger");
  LOG4CXX_DEBUG(logger_, "Logger deinitialization");

  set_logs_enabled(false);
  delete message_loop_thread;
}

bool logs_enabled() { return is_logs_enabled; }

void set_logs_enabled(bool state) { is_logs_enabled = state; }

bool push_log(log4cxx::LoggerPtr logger,
              log4cxx::LevelPtr level,
              const std::string& entry,
              log4cxx_time_t time,
              const log4cxx::spi::LocationInfo& location,
              const log4cxx::LogString& thread_name) {
  if (!logs_enabled()) {
    return false;
  }
  if (!logger->isEnabledFor(level)) {
    return false;
  }
  LogMessage message = {logger, level, entry, time, location, thread_name};
  message_loop_thread->PostMessage(message);
  return true;
}

}  // namespace logger
