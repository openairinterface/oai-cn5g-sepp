/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "sepp_config.hpp"

#include "if.hpp"
#include "logger.hpp"
#include "sbi_helper.hpp"
#include <nlohmann/json.hpp>

using namespace oai::config::sepp;
using namespace oai::config;
using namespace oai::common::sbi;

//------------------------------------------------------------------------------
sepp_config::sepp_config(const std::string &config_path, bool log_stdout,
                         bool log_rot_file)
    : config(config_path, SEPP_CONFIG_NAME, log_stdout, log_rot_file),
      m_role("role", "c-sepp"), m_security("security", "PRINS"),
      m_disable_tls("disable_tls", false),
      m_server_private_key("private_key", ""), m_server_cert("cert", ""),
      m_client_cacert("cacert", "") {
  m_used_config_values = {LOG_LEVEL_CONFIG_NAME, REGISTER_NF_CONFIG_NAME,
                          NF_LIST_CONFIG_NAME,   SEPP_CONFIG_NAME,
                          NF_CONFIG_HTTP_NAME,   ROAMING_CONFIG_NAME};
  m_used_sbi_values = {SEPP_CONFIG_NAME, NRF_CONFIG_NAME};

  auto sepp =
      std::make_shared<nf>(SEPP_CONFIG_NAME, "oai-sepp",
                           sbi_interface("SBI", "oai-sepp", 80, "v1", "eth0"),
                           nbi_interface("NBI", "oai-sepp", 80, "v1", "eth1"));

  auto nrf =
      std::make_shared<nf>(NRF_CONFIG_NAME, "oai-nrf",
                           sbi_interface("SBI", "oai-nrf", 80, "v1", "eth0"));

  add_nf(SEPP_CONFIG_NAME, sepp);
  add_nf(NRF_CONFIG_NAME, nrf);
  read_from_file(config_path);
}

//------------------------------------------------------------------------------
void sepp_config::read_from_file(const std::string &file_path) {
  Logger::sepp_app().debug("Reading SEPP configuration from file: %s",
                           file_path.c_str());
  config::read_from_file(file_path);

  try {
    YAML::Node root_node = YAML::LoadFile(file_path);

    if (root_node[SEPP_CONFIG_NAME]) {
      const YAML::Node &sepp_node = root_node[SEPP_CONFIG_NAME];

      // Parse role
      if (sepp_node["role"]) {
        m_role.from_yaml(sepp_node["role"]);
        std::string role_str = m_role.get_value();
        if (role_str == "c-sepp") {
          m_sepp_role_enum = sepp_role_e::C_SEPP;
        } else if (role_str == "p-sepp") {
          m_sepp_role_enum = sepp_role_e::P_SEPP;
        } else {
          throw std::invalid_argument(
              "Invalid SEPP role specified: '" + role_str +
              "'. Valid values are 'c-sepp' or 'p-sepp'.");
        }
      }

      // Parse security (PRINS or TLS)
      if (sepp_node["security"]) {
        m_security.from_yaml(sepp_node["security"]);
      }

      // Parse disable_tls (yes / no)
      if (sepp_node["disable_tls"]) {
        m_disable_tls.from_yaml(sepp_node["disable_tls"]);
      }

      // Parse TLS certificates if TLS is not disabled
      if (is_tls_enabled()) {
        if (sepp_node["tls"]) {
          const YAML::Node &tls_node = sepp_node["tls"];

          if (tls_node["server"]) {
            const YAML::Node &server_node = tls_node["server"];
            if (server_node["private_key"]) {
              m_server_private_key.from_yaml(server_node["private_key"]);
            }
            if (server_node["cert"]) {
              m_server_cert.from_yaml(server_node["cert"]);
            }
          }

          if (tls_node["client"]) {
            const YAML::Node &client_node = tls_node["client"];
            if (client_node["cacert"]) {
              m_client_cacert.from_yaml(client_node["cacert"]);
            }
          }
        }

        if (m_server_cert.get_value().empty() ||
            m_server_private_key.get_value().empty()) {
          Logger::sepp_app().warn(
              "TLS is enabled (disable_tls: no), but server certificate "
              "or private key paths are missing in config.yaml");
        }
      }
    }
  } catch (const YAML::Exception &e) {
    logger::logger_registry::get_logger(LOGGER_NAME)
        .error("Failed to parse SEPP YAML configuration: %s", e.what());
    throw std::runtime_error(e.what());
  }
}

//------------------------------------------------------------------------------
const std::string &sepp_config::get_security() const {
  return m_security.get_value();
}

//------------------------------------------------------------------------------
bool sepp_config::is_tls_enabled() const { return !m_disable_tls.get_value(); }

//------------------------------------------------------------------------------
bool sepp_config::is_tls_disabled() const { return m_disable_tls.get_value(); }

//------------------------------------------------------------------------------
bool sepp_config::init() { return config::init(); }

const std::string &sepp_config::get_role() const { return m_role.get_value(); }

sepp_role_e sepp_config::get_role_enum() const { return m_sepp_role_enum; }

bool sepp_config::is_c_sepp() const {
  return m_sepp_role_enum == sepp_role_e::C_SEPP;
}

//------------------------------------------------------------------------------
bool sepp_config::is_p_sepp() const {
  return m_sepp_role_enum == sepp_role_e::P_SEPP;
}

//------------------------------------------------------------------------------
const std::string &sepp_config::get_server_private_key() const {
  return m_server_private_key.get_value();
}

//------------------------------------------------------------------------------
const std::string &sepp_config::get_server_cert() const {
  return m_server_cert.get_value();
}

//------------------------------------------------------------------------------
const std::string &sepp_config::get_client_cacert() const {
  return m_client_cacert.get_value();
}

//------------------------------------------------------------------------------
bool sepp_config::get_api_list(nlohmann::json &api_list) {
  api_list["OAI-SEPP"] = {
      {"Organisation", "Openairinterface Software Aliance"},
      {"Description",
       "OAI-SEPP (Security Edge Protection Proxy) initial Release"},
      {"Version", "1.0.0"},
      {"Supported APIs",
       {
           {{"API", "n32c: Security Capability Negotiation"},
            {"Method", "POST"},
            {"URI Path", "/n32c-handshake/<api_version>/exchange-capability"},
            {"Details", "Initiate the handshake procedure between two SEPPs"}},
           {{"API", "n32c: Parameter Exchange"},
            {"Method", "POST"},
            {"URI Path", "/n32c-handshake/<api_version>/exchange-params"},
            {"Details", "Initiate the handshake procedure between two SEPPs"}},
           {{"API", "n32c: N32-f Error Report"},
            {"Method", "POST"},
            {"URI Path", "/n32c-handshake/<api_version>/n32f-error"},
            {"Details", "Report the error to the remote SEPP"}},
           {{"API", "n32c: N32-f Context Terminate"},
            {"Method", "POST"},
            {"URI Path", "/n32c-handshake/<api_version>/n32f-terminate"},
            {"Details",
             "Terminate the PDU Session context at the remote SEPP"}},
           {{"API", "n32f: Forwarding"},
            {"Method", "POST"},
            {"URI Path", "/n32f-forward/<api_version>/n32f-process"},
            {"Details", "Forward the PDU Session data to the remote SEPP "}},
           {{"API", "n32f: Forwarding"},
            {"Method", "OPTIONS"},
            {"URI Path", "/n32f-forward/<api_version>/n32f-process"},
            {"Details", "Discover communication options supported by next hop "
                        "(IPX or SEPP)"}},
       }}};
  return true;
}