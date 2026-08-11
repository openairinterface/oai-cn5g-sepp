/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "api_defs.h"

#include "sepp_config.hpp"

extern std::unique_ptr<oai::config::sepp::sepp_config> sepp_cfg;

namespace oai::sepp::api {

std::string sepp_forward_API::get_route() {
  return API_BASE + sepp_cfg->local().get_sbi().get_api_version() +
         sepp_API::CREATE_ROUTE;
}

std::string sepp_handshake_API::get_route() {
  return API_BASE + sepp_cfg->local().get_sbi().get_api_version() +
         sepp_handshake_API::CREATE_ROUTE;
}

std::string sepp_telescoic_API::get_route() {
  return API_BASE + sepp_cfg->local().get_sbi().get_api_version() +
         sepp_telescoic_API::CREATE_ROUTE;
}
} // namespace oai::sepp::api
