﻿/*
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
#if defined(OS_WINDOWS)

#include "utils/lock.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <cstring>
#include "utils/logger.h"

namespace sync_primitives {

CREATE_LOGGERPTR_GLOBAL(logger_, "Utils")

Lock::Lock()
#ifndef NDEBUG
    : lock_taken_(0)
    , is_mutex_recursive_(false)
#endif  // NDEBUG
{
  Init(false);
}

Lock::Lock(bool is_recursive)
#ifndef NDEBUG
    : lock_taken_(0)
    , is_mutex_recursive_(is_recursive)
#endif  // NDEBUG
{
  Init(is_recursive);
}

Lock::~Lock() {
#ifndef NDEBUG
  if (lock_taken_ > 0) {
    LOG4CXX_ERROR(logger_, "Destroying non-released mutex " << &mutex_);
  }
#endif
  DeleteCriticalSection(&mutex_);
}

void Lock::Acquire() {
  EnterCriticalSection(&mutex_);
  AssertFreeAndMarkTaken();
}

void Lock::Release() {
  AssertTakenAndMarkFree();
  LeaveCriticalSection(&mutex_);
}

bool Lock::Try() {
  if (TryEnterCriticalSection(&mutex_)) {
#ifndef NDEBUG
    lock_taken_++;
#endif
    return true;
  }
  return false;
}

#ifndef NDEBUG
void Lock::AssertFreeAndMarkTaken() {
  if ((lock_taken_ > 0) && !is_mutex_recursive_) {
    LOG4CXX_ERROR(logger_, "Locking already taken not recursive mutex");
    NOTREACHED();
  }
  lock_taken_++;
}
void Lock::AssertTakenAndMarkFree() {
  if (lock_taken_ == 0) {
    LOG4CXX_ERROR(logger_, "Unlocking a mutex that is not taken");
    NOTREACHED();
  }
  lock_taken_--;
}
#endif

void Lock::Init(bool is_recursive) { InitializeCriticalSection(&mutex_); }

}  // namespace sync_primitives

#endif  // OS_WINDOWS
