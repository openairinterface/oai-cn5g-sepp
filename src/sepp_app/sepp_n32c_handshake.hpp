#ifndef FILE_SEPP_N32C_HANDSHAKE_HPP_SEEN
#define FILE_SEPP_N32C_HANDSHAKE_HPP_SEEN

#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#include "N32fContextInfo.h"
#include "SecNegotiateReqData.h"
#include "SecParamExchReqData.h"
#include "SecParamExchRspData.h"

namespace oai::sepp::app {

class sepp_n32c_handshake {
  friend class sepp_app;

public:
  sepp_n32c_handshake() = default;
  explicit sepp_n32c_handshake(std::string sender_fqdn)
      : m_sender_fqdn(std::move(sender_fqdn)) {}
  ~sepp_n32c_handshake() = default;

  void set_sender_fqdn(std::string fqdn) noexcept {
    m_sender_fqdn = std::move(fqdn);
  }
  [[nodiscard]] const std::string &get_sender_fqdn() const noexcept {
    return m_sender_fqdn;
  }
  [[nodiscard]] const std::string &get_peer_sepp_fqdn() const noexcept {
    return m_peer_sepp_fqdn;
  }
  [[nodiscard]] const std::string &get_n32f_context_id() const noexcept {
    return m_n32c_context_id;
  }
  void set_n32f_context_id(std::string context_id) noexcept {
    m_n32c_context_id = std::move(context_id);
  }
  [[nodiscard]] bool is_peer_target_api_root_supported() const noexcept {
    return m_peer_target_api_root_supported;
  }
  void set_peer_target_api_root_supported(bool supported) noexcept {
    m_peer_target_api_root_supported = supported;
  }

  /* 3GPP TS 29.573: Capability Exchange */
  bool handle_exchange_capability_request(
      const nlohmann::json &req_data, nlohmann::json &resp_data,
      std::unordered_map<std::string, std::string> &resp_headers);

  bool create_exchange_capability_req(nlohmann::json &req_data);
  bool create_exchange_capability_req(
      oai::_3gpp::model::SecNegotiateReqData &req_data);

  bool handle_exchange_capability_response(const nlohmann::json &resp_data);

  [[nodiscard]] const std::string &
  get_selected_sec_capability() const noexcept {
    return m_selected_sec_capability;
  }

  /* 3GPP TS 29.573: Parameter Exchange (/exchange-params) */
  bool
  create_exchange_params_req(oai::_3gpp::model::SecParamExchReqData &req_obj);
  bool create_exchange_params_req(nlohmann::json &req_data);

  bool handle_exchange_params(
      const nlohmann::json &req_data, nlohmann::json &resp_data,
      std::unordered_map<std::string, std::string> &resp_headers);

  bool handle_exchange_params_response(const nlohmann::json &resp_data);

  /* 3GPP TS 29.573: N32-f Context Terminate (/n32f-terminate) */
  bool create_n32f_terminate_req(oai::_3gpp::model::N32fContextInfo &req_obj);
  bool create_n32f_terminate_req(nlohmann::json &req_data);

  bool handle_n32f_terminate_req(
      const nlohmann::json &req_data, nlohmann::json &resp_data,
      std::unordered_map<std::string, std::string> &resp_headers);

  bool handle_n32f_terminate_response(const nlohmann::json &resp_data);

  /* 3GPP TS 29.573: /n32f-error */
  bool
  handle_n32f_error(const nlohmann::json &req_data, nlohmann::json &resp_data,
                    std::unordered_map<std::string, std::string> &resp_headers);

private:
  bool validate_handshake_request(const nlohmann::json &req_data) const;

  std::string m_n32c_context_id;
  std::string m_peer_sepp_fqdn;
  std::string m_sender_fqdn{"sepp.5gc.mnc22.mcc208.3gppnetwork.org"};
  bool m_peer_target_api_root_supported{false};

  std::string m_selected_sec_capability{"PRINS"};
  std::vector<std::string> m_allowed_n32_purposes;

  std::string m_selected_jwe_cipher{"A128GCM"};
  std::string m_selected_jws_cipher{"ES256"};
};

} // namespace oai::sepp::app

#endif /* FILE_SEPP_N32C_HANDSHAKE_HPP_SEEN */