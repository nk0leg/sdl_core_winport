/*

 Copyright (c) 2013, Ford Motor Company
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.

 Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following
 disclaimer in the documentation and/or other materials provided with the
 distribution.

 Neither the name of the Ford Motor Company nor the names of its contributors
 may be used to endorse or promote products derived from this software
 without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

#include "application_manager/commands/mobile/set_app_icon_request.h"
#include "application_manager/application_manager_impl.h"
#include "application_manager/application_impl.h"
#include "config_profile/profile.h"
#include "interfaces/MOBILE_API.h"
#include "interfaces/HMI_API.h"
#include "utils/file_system.h"
#include "utils/helpers.h"

namespace application_manager {

namespace commands {

int8_t SetAppIconRequest::is_icons_saving_enabled_ = -1;

SetAppIconRequest::SetAppIconRequest(const MessageSharedPtr& message)
    : CommandRequestImpl(message) {
  if (-1 == is_icons_saving_enabled_) {
    const std::string path = profile::Profile::instance()->app_icons_folder();
    is_icons_saving_enabled_ = file_system::IsWritingAllowed(path) &&
                               file_system::IsReadingAllowed(path);
  }
}

SetAppIconRequest::~SetAppIconRequest() {}

void SetAppIconRequest::Run() {
  LOGGER_AUTO_TRACE(logger_);

  ApplicationSharedPtr app =
      ApplicationManagerImpl::instance()->application(connection_key());

  if (!app) {
    LOGGER_ERROR(logger_, "Application is not registered");
    SendResponse(false, mobile_apis::Result::APPLICATION_NOT_REGISTERED);
    return;
  }

  const std::string& sync_file_name =
      (*message_)[strings::msg_params][strings::sync_file_name].asString();

  std::string full_file_path = file_system::ConcatPath(
      profile::Profile::instance()->app_storage_folder(),
      app->folder_name(),
      sync_file_name);

  if (!file_system::FileExists(full_file_path)) {
    LOGGER_ERROR(logger_, "No such file " << full_file_path);
    SendResponse(false, mobile_apis::Result::INVALID_DATA);
    return;
  }

  if (is_icons_saving_enabled_) {
    CopyToIconStorage(full_file_path);
  }

  smart_objects::SmartObject msg_params =
      smart_objects::SmartObject(smart_objects::SmartType_Map);

  msg_params[strings::app_id] = app->app_id();
  msg_params[strings::sync_file_name] =
      smart_objects::SmartObject(smart_objects::SmartType_Map);

  // Panasonic requres unchanged path value without encoded special characters
  const std::string full_file_path_for_hmi =
      file_system::ConvertPathForURL(full_file_path);

  msg_params[strings::sync_file_name][strings::value] = full_file_path_for_hmi;

  // TODO(VS): research why is image_type hardcoded
  msg_params[strings::sync_file_name][strings::image_type] =
      static_cast<int32_t>(SetAppIconRequest::ImageType::DYNAMIC);

  // for further use in on_event function
  (*message_)[strings::msg_params][strings::sync_file_name] =
      msg_params[strings::sync_file_name];

  SendHMIRequest(hmi_apis::FunctionID::UI_SetAppIcon, &msg_params, true);
}

void SetAppIconRequest::CopyToIconStorage(
    const std::string& path_to_file) const {
  if (!profile::Profile::instance()->enable_protocol_4()) {
    LOGGER_WARN(logger_,
                 "Icon copying skipped, since protocol ver. 4 is not enabled.");
    return;
  }

  std::vector<uint8_t> file_content;
  if (!file_system::ReadBinaryFile(path_to_file, file_content)) {
    LOGGER_ERROR(logger_, "Can't read icon file: " << path_to_file);
    return;
  }

  const std::string icon_storage =
      profile::Profile::instance()->app_icons_folder();
  const uint64_t storage_max_size = static_cast<uint64_t>(
      profile::Profile::instance()->app_icons_folder_max_size());
  const uint64_t file_size = file_system::FileSize(path_to_file);

  if (storage_max_size < file_size) {
    LOGGER_ERROR(logger_,
                  "Icon size (" << file_size << ") is bigger, than "
                                                " icons storage maximum size ("
                                << storage_max_size
                                << ")."
                                   "Copying skipped.");
    return;
  }

  const uint64_t storage_size = file_system::DirectorySize(icon_storage);
  if (storage_max_size < (file_size + storage_size)) {
    const uint32_t icons_amount =
        profile::Profile::instance()->app_icons_amount_to_remove();

    if (!icons_amount) {
      LOGGER_DEBUG(logger_,
                    "No icons will be deleted, since amount icons to remove "
                    "is zero. Icon saving skipped.");
      return;
    }

    while (!IsEnoughSpaceForIcon(file_size)) {
      RemoveOldestIcons(icon_storage, icons_amount);
    }
  }
  ApplicationConstSharedPtr app =
      application_manager::ApplicationManagerImpl::instance()->application(
          connection_key());

  if (!app) {
    LOGGER_ERROR(
        logger_,
        "Can't get application for connection key: " << connection_key());
    return;
  }

  const std::string icon_path =
      file_system::ConcatPath(icon_storage, app->mobile_app_id());
  if (!file_system::CreateFile(icon_path)) {
    LOGGER_ERROR(logger_, "Can't create icon: " << icon_path);
    return;
  }

  if (!file_system::Write(icon_path, file_content)) {
    LOGGER_ERROR(logger_, "Can't write icon: " << icon_path);
    return;
  }

  LOGGER_DEBUG(logger_,
                "Icon was successfully copied from :" << path_to_file << " to "
                                                      << icon_path);

  return;
}

void SetAppIconRequest::RemoveOldestIcons(const std::string& storage,
                                          const uint32_t icons_amount) const {
  const std::vector<std::string> icons_list = file_system::ListFiles(storage);
  std::map<uint64_t, std::string> icon_modification_time;
  std::vector<std::string>::const_iterator it = icons_list.begin();
  for (; it != icons_list.end(); ++it) {
    const std::string file_name = *it;
    const std::string file_path = file_system::ConcatPath(storage, file_name);
    if (!file_system::FileExists(file_path)) {
      continue;
    }
    const uint64_t time = file_system::GetFileModificationTime(file_path);
    icon_modification_time[time] = file_name;
  }

  for (size_t counter = 0; counter < icons_amount; ++counter) {
    if (!icon_modification_time.size()) {
      LOGGER_ERROR(logger_, "No more icons left for deletion.");
      return;
    }
    const std::string file_name = icon_modification_time.begin()->second;
    const std::string file_path = file_system::ConcatPath(storage, file_name);
    if (!file_system::DeleteFile(file_path)) {
      LOGGER_DEBUG(logger_, "Error while deleting icon " << file_path);
    }
    icon_modification_time.erase(icon_modification_time.begin());
    LOGGER_DEBUG(logger_,
                  "Old icon " << file_path << " was deleted successfully.");
  }
}

bool SetAppIconRequest::IsEnoughSpaceForIcon(const uint64_t icon_size) const {
  const std::string icon_storage =
      profile::Profile::instance()->app_icons_folder();
  const uint64_t storage_max_size = static_cast<uint64_t>(
      profile::Profile::instance()->app_icons_folder_max_size());
  const uint64_t storage_size = file_system::DirectorySize(icon_storage);
  return storage_max_size >= (icon_size + storage_size);
}

void SetAppIconRequest::on_event(const event_engine::Event& event) {
  LOGGER_AUTO_TRACE(logger_);
  using namespace helpers;
  const smart_objects::SmartObject& message = event.smart_object();

  switch (event.id()) {
    case hmi_apis::FunctionID::UI_SetAppIcon: {
      mobile_apis::Result::eType result_code =
          static_cast<mobile_apis::Result::eType>(
              message[strings::params][hmi_response::code].asInt());

      const bool result = Compare<mobile_api::Result::eType, EQ, ONE>(
          result_code,
          mobile_api::Result::SUCCESS,
          mobile_api::Result::WARNINGS);

      if (result) {
        ApplicationSharedPtr app =
            ApplicationManagerImpl::instance()->application(connection_key());

        if (!message_.valid() || !app.valid()) {
          LOGGER_ERROR(logger_, "NULL pointer.");
          return;
        }

        const std::string path =
            (*message_)[strings::msg_params][strings::sync_file_name]
                       [strings::value]
                           .asString();
        app->set_app_icon_path(path);

        LOGGER_INFO(logger_,
                     "Icon path was set to '" << app->app_icon_path() << "'");
      }

      SendResponse(result, result_code, NULL, &(message[strings::msg_params]));
      break;
    }
    default: {
      LOGGER_ERROR(logger_, "Received unknown event" << event.id());
      return;
    }
  }
}

}  // namespace commands

}  // namespace application_manager
