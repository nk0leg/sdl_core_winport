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
#ifndef SRC_COMPONENTS_UTILS_INCLUDE_UTILS_LOG_MESSAGE_LOOP_THREAD_H_
#define SRC_COMPONENTS_UTILS_INCLUDE_UTILS_LOG_MESSAGE_LOOP_THREAD_H_

#include <string>
#include <queue>
#include <cstdint>

#if defined(LOG4CXX_LOGGER)
#include <log4cxx/logger.h>
#elif defined(WIN_NATIVE)
#include "utils/winhdr.h"
#elif defined(QT_PORT)
#include <QDateTime>
#else
#error Unsupported case for logging includes
#endif

#include "utils/macro.h"
#include "utils/threads/message_loop_thread.h"

namespace logger {

#if defined(OS_POSIX)
typedef struct {
  log4cxx::LoggerPtr logger;
  log4cxx::LevelPtr level;
  log4cxx_time_t time;
  std::string entry;
  log4cxx::spi::LocationInfo location;
  log4cxx::LogString thread_name;
} LogMessage;
#elif defined(WIN_NATIVE)
typedef struct {
  std::string logger;
  LogLevel level;
  SYSTEMTIME time;
  std::string entry;
  unsigned long line_number;
  const char* file_name;
  const char* function_name;
  uint32_t thread_id;
  FILE* output_file;
} LogMessage;
#elif defined(QT_PORT)
struct LogMessage {
  std::string logger;
  LogLevel level;
  QDateTime time;
  std::string entry;
  unsigned long line_number;
  const char* file_name;
  const char* function_name;
  uint32_t thread_id;
};
#else
#error Unsupported case for the LogMessage
#endif

typedef std::queue<LogMessage> LogMessageQueue;
typedef threads::MessageLoopThread<LogMessageQueue>
    LogMessageLoopThreadTemplate;

class LogMessageHandler : public LogMessageLoopThreadTemplate::Handler {
 public:
  virtual void Handle(const LogMessage message) OVERRIDE;
};

class LogMessageLoopThread : public LogMessageLoopThreadTemplate {
 public:
  LogMessageLoopThread()
      : LogMessageLoopThreadTemplate("Logger", new LogMessageHandler()) {}

  ~LogMessageLoopThread() {}

 private:
  DISALLOW_COPY_AND_ASSIGN(LogMessageLoopThread);
};

}  // namespace logger

#endif  // SRC_COMPONENTS_UTILS_INCLUDE_UTILS_LOG_MESSAGE_LOOP_THREAD_H_
