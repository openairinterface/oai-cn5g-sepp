# SPDX-License-Identifier: LicenseRef-CSSL-1.0

include_directories(${SRC_TOP_DIR}/sepp)
include_directories(${SRC_TOP_DIR}/api-server/api)
include_directories(${SRC_TOP_DIR}/api-server/impl)
include_directories(${SRC_TOP_DIR}/api-server/)
include_directories(${SRC_TOP_DIR}/${MOUNTED_COMMON}/3gpp)
include_directories(${SRC_TOP_DIR}/${MOUNTED_COMMON}/common)
include_directories(${SRC_TOP_DIR}/${MOUNTED_COMMON}/logger)
include_directories(${SRC_TOP_DIR}/${MOUNTED_COMMON}/model/sepp)
include_directories(${SRC_TOP_DIR}/${MOUNTED_COMMON}/utils)
## Common modules
include(${SRC_TOP_DIR}/${MOUNTED_COMMON}/http/http.cmake)

include(${SRC_TOP_DIR}/${MOUNTED_COMMON}/config/config.cmake)

add_library (SEPP STATIC
  sepp_app.cpp
  sepp_config.cpp
  sepp_profile.cpp
  sepp_nrf.cpp
  sepp_event.cpp
  task_manager.cpp
  sepp_telescopic_fqdn_mapping.cpp
  sepp_n32c_handshake.cpp
  sepp_n32f_forward.cpp
  sepp_JOSE.cpp
  )
