# Copyright (c) 2014, Ford Motor Company
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
#
# Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following
# disclaimer in the documentation and/or other materials provided with the
# distribution.
#
# Neither the name of the Ford Motor Company nor the names of its contributors
# may be used to endorse or promote products derived from this software
# without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.


set(FORMATTERS_SRC_DIR  ${COMPONENTS_DIR}/formatters/src)

include_directories (
  include/
  ${COMPONENTS_DIR}/protocol_handler/include/
  ${COMPONENTS_DIR}/utils/include/
  ${JSONCPP_INCLUDE_DIRECTORY}
  ${MESSAGE_BROKER_INCLUDE_DIRECTORY}
  ${COMPONENTS_DIR}/smart_objects/include
)

set (SOURCES
    ${FORMATTERS_SRC_DIR}/CSmartFactory.cpp
)

set (FORMATTER_SOURCES
   ${FORMATTERS_SRC_DIR}/CFormatterJsonBase.cpp
   ${FORMATTERS_SRC_DIR}/CFormatterJsonSDLRPCv1.cpp
   ${FORMATTERS_SRC_DIR}/CFormatterJsonSDLRPCv2.cpp
   ${FORMATTERS_SRC_DIR}/formatter_json_rpc.cc
   ${FORMATTERS_SRC_DIR}/meta_formatter.cc
   ${FORMATTERS_SRC_DIR}/generic_json_formatter.cc
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set (HEADERS  ${COMPONENTS_DIR}/formatters/include/formatters/formatter_json_rpc.h
                ${COMPONENTS_DIR}/formatters/include/formatters/generic_json_formatter.h
                ${COMPONENTS_DIR}/formatters/include/formatters/meta_formatter.h
                ${COMPONENTS_DIR}/formatters/include/formatters/CFormatterJsonBase.hpp
                ${COMPONENTS_DIR}/formatters/include/formatters/CFormatterJsonSDLRPCv1.hpp
                ${COMPONENTS_DIR}/formatters/include/formatters/CFormatterJsonSDLRPCv2.hpp
                ${COMPONENTS_DIR}/formatters/include/formatters/CSmartFactory.hpp)
  source_group("Header Files" FILES ${HEADERS} )
  source_group("Source Files" FILES ${SOURCES} ${FORMATTER_SOURCES})

  add_library("formatters" ${HEADERS} ${SOURCES} ${FORMATTER_SOURCES})
else()
  add_library("formatters" ${SOURCES}
          ${FORMATTER_SOURCES}
  )
endif()

if(BUILD_TESTS)
  add_subdirectory(test)
endif()
