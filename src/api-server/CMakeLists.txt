# SPDX-License-Identifier: LicenseRef-CSSL-1.0

set(SEPP_API_SERVER_DIR "${SRC_TOP_DIR}/api-server")

include_directories(${SEPP_API_SERVER_DIR}/api)
include_directories(${SEPP_API_SERVER_DIR}/impl)
include_directories(${SEPP_API_SERVER_DIR}/)
include_directories(${SRC_TOP_DIR}/sepp_app)
include_directories(${SRC_TOP_DIR}/${MOUNTED_COMMON}/3gpp)
include_directories(${SRC_TOP_DIR}/${MOUNTED_COMMON}/config)
include_directories(${SRC_TOP_DIR}/${MOUNTED_COMMON}/common)
include_directories(${SRC_TOP_DIR}/${MOUNTED_COMMON}/logger)
include_directories(${SRC_TOP_DIR}/${MOUNTED_COMMON}/model)
include_directories(${SRC_TOP_DIR}/${MOUNTED_COMMON}/utils)
include_directories(${SRC_TOP_DIR}/${MOUNTED_COMMON}/utils/bstr)

include(${SRC_TOP_DIR}/${MOUNTED_COMMON}/model/sepp_model.cmake)

file(GLOB SEPP_API_SERVER_src_files
    ${SEPP_API_SERVER_DIR}/sepp-api-server.cpp
    ${SEPP_API_SERVER_DIR}/sepp-http2-sbi-server.cpp
    ${SEPP_API_SERVER_DIR}/sepp-http2-nbi-server.cpp
    ${SEPP_API_SERVER_DIR}/api/*.cpp
    ${SEPP_API_SERVER_DIR}/impl/*.cpp
)

add_library(SEPP_API STATIC
           ${SEPP_API_SERVER_src_files})
