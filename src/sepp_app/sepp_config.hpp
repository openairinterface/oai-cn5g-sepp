/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#pragma once

#include "config.hpp"
#include "logger.hpp"
#include <stdexcept>
#include <string>
#include <yaml-cpp/yaml.h>

namespace oai::config::sepp {

enum class sepp_role_e { C_SEPP, P_SEPP };

class sepp_config : public oai::config::config {
private:
  string_config_value m_role;
  sepp_role_e m_sepp_role_enum{sepp_role_e::C_SEPP};

  option_config_value m_disable_tls;
  string_config_value m_server_private_key;
  string_config_value m_server_cert;
  string_config_value m_client_cacert;

public:
  string_config_value m_security;
  unsigned int instance = 0;
  explicit sepp_config(const std::string &config_path, bool log_stdout,
                       bool log_rot_file);

  static bool get_api_list(nlohmann::json &api_list);
  bool init() override;

  void read_from_file(const std::string &file_path);

  // Role Getters & Helpers
  [[nodiscard]] const std::string &get_role() const;
  [[nodiscard]] sepp_role_e get_role_enum() const;
  [[nodiscard]] bool is_c_sepp() const;
  [[nodiscard]] bool is_p_sepp() const;

  // TLS & Security Getters
  [[nodiscard]] const std::string &get_server_private_key() const;
  [[nodiscard]] const std::string &get_server_cert() const;
  [[nodiscard]] const std::string &get_client_cacert() const;
  [[nodiscard]] const std::string &get_security() const;
  [[nodiscard]] bool is_tls_enabled() const;
  [[nodiscard]] bool is_tls_disabled() const;
};

} // namespace oai::config::sepp