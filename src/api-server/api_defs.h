/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#pragma once
#include <string>

namespace oai::sepp::api {
class sepp_forward_API {
public:
  static inline const std::string API_NAME = "n32f-forward";
  static inline const std::string API_BASE = "/" + API_NAME + "/";
  static inline const std::string CREATE_ROUTE = "/n32f-forward";

  static std::string get_route();
};

class sepp_handshake_API {
public:
  static inline const std::string API_NAME = "n32c-handshake";
  static inline const std::string API_BASE = "/" + API_NAME + "/";
  static inline const std::string CREATE_ROUTE = "/n32c-handshake";

  static std::string get_route();
};

class sepp_telescoic_API {
public:
  static inline const std::string API_NAME = "nsepp-telescopic";
  static inline const std::string API_BASE = "/" + API_NAME + "/";
  static inline const std::string CREATE_ROUTE = "/nsepp-telescopic";

  static std::string get_route();
};

} // namespace oai::sepp::api