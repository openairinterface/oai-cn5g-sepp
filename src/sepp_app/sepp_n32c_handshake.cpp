/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */
#include "sepp_n32c_handshake.hpp"

#include <algorithm>
#include <sstream>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "N32fContextInfo.h"
#include "SecNegotiateReqData.h"
#include "SecParamExchReqData.h"
#include "SecParamExchRspData.h"
#include "logger.hpp"
#include "sepp_config.hpp"

using namespace oai::sepp::app;
using namespace oai::config::sepp;
using namespace oai::_3gpp::model;

extern std::unique_ptr<sepp_config> sepp_cfg;

namespace {
constexpr const char *SBI_TARGET_API_ROOT_HEADER =
    "3gpp-Sbi-Target-ApiRoot-Supported";
constexpr const char *HEADER_VALUE_YES = "true";
constexpr const char *CAP_TLS = "TLS";
constexpr const char *CAP_PRINS = "PRINS";
} // namespace

//------------------------------------------------------------------------------
bool sepp_n32c_handshake::validate_handshake_request(
    const nlohmann::json &req_data) const {
  if (req_data.is_null() || !req_data.is_object()) {
    Logger::sepp_app().warn("N32-c handshake request JSON is empty or invalid");
    return false;
  }
  return true;
}

//------------------------------------------------------------------------------
bool sepp_n32c_handshake::handle_exchange_capability_request(
    const nlohmann::json &req_data, nlohmann::json &resp_data,
    std::unordered_map<std::string, std::string> &resp_headers) {
  Logger::sepp_app().info("Processing N32-c capability exchange request");

  if (!validate_handshake_request(req_data)) {
    return false;
  }

  SecNegotiateReqData req_obj;
  try {
    from_json(req_data, req_obj);
  } catch (const std::exception &e) {
    Logger::sepp_app().error("Failed to parse SecNegotiateReqData: %s",
                             e.what());
    return false;
  }

  std::stringstream err_msg;
  if (!req_obj.validate(err_msg)) {
    Logger::sepp_app().warn("SecNegotiateReqData validation failed: %s",
                            err_msg.str().c_str());
    return false;
  }

  if (req_obj.r3GppSbiTargetApiRootSupportedIsSet()) {
    m_peer_target_api_root_supported =
        req_obj.isR3GppSbiTargetApiRootSupported();
    Logger::sepp_app().info("Peer indicated 3GppSbiTargetApiRootSupported: %s",
                            m_peer_target_api_root_supported ? "true"
                                                             : "false");
  }

  // 3GPP TS 29.573: Indicate support for target api root
  resp_headers[SBI_TARGET_API_ROOT_HEADER] = HEADER_VALUE_YES;

  m_peer_sepp_fqdn = req_obj.getSender();
  Logger::sepp_app().info(
      "Received capability exchange request from peer SEPP: %s",
      m_peer_sepp_fqdn.c_str());

  std::vector<std::string> supported_caps;
  if (req_data.contains("supportedSecCapabilityList") &&
      req_data["supportedSecCapabilityList"].is_array()) {
    supported_caps =
        req_data["supportedSecCapabilityList"].get<std::vector<std::string>>();
  }

  std::string selected_cap = CAP_PRINS;
  if (!supported_caps.empty()) {
    if (std::find(supported_caps.begin(), supported_caps.end(), CAP_TLS) !=
        supported_caps.end()) {
      selected_cap = CAP_TLS;
    } else if (std::find(supported_caps.begin(), supported_caps.end(),
                         CAP_PRINS) != supported_caps.end()) {
      selected_cap = CAP_PRINS;
    } else {
      selected_cap = supported_caps.front();
    }
  }

  if (selected_cap != m_selected_sec_capability) {
    Logger::sepp_app().error(
        "Selected security capability '%s' differs from configured '%s'. "
        "Using configured value.",
        selected_cap.c_str(), m_selected_sec_capability.c_str());
  }

  // Build SecNegotiateRspData payload according to 3GPP TS 29.573. "sender"
  // is of type Fqdn: the FQDN of this SEPP, as in the requests it sends.
  resp_data["sender"] = m_sender_fqdn;
  resp_data["selectedSecCapability"] = m_selected_sec_capability;
  resp_data["3GppSbiTargetApiRootSupported"] = true;

  nlohmann::json usage_purpose_item;
  usage_purpose_item["usagePurpose"] = "ROAMING";
  resp_data["allowedUsagePurpose"] =
      nlohmann::json::array({std::move(usage_purpose_item)});

  Logger::sepp_app().info(
      "Successfully processed N32-c capability exchange "
      "(Selected Security: %s, Purpose: ROAMING, TargetApiRoot: true)",
      m_selected_sec_capability.c_str());
  return true;
}

//------------------------------------------------------------------------------
bool sepp_n32c_handshake::create_exchange_capability_req(
    SecNegotiateReqData &req_obj) {
  Logger::sepp_app().info(
      "Creating N32-c capability exchange request via model");

  // TS 29.573 SecNegotiateReqData: "sender" is the FQDN of this SEPP
  req_obj.setSender(m_sender_fqdn);
  req_obj.setR3GppSbiTargetApiRootSupported(true);

  nlohmann::json purpose_json = {{"usagePurpose", "ROAMING"}};
  IntendedN32Purpose purpose;
  from_json(purpose_json, purpose);
  req_obj.setIntendedUsagePurpose({std::move(purpose)});

  return true;
}

//------------------------------------------------------------------------------
bool sepp_n32c_handshake::create_exchange_capability_req(
    nlohmann::json &req_data) {
  SecNegotiateReqData req_obj;
  if (!create_exchange_capability_req(req_obj)) {
    return false;
  }
  to_json(req_data, req_obj);

  req_data["supportedSecCapabilityList"] =
      nlohmann::json::array({m_selected_sec_capability});
  return true;
}

//------------------------------------------------------------------------------
bool sepp_n32c_handshake::handle_exchange_capability_response(
    const nlohmann::json &resp_data) {
  Logger::sepp_app().info("Processing N32-c capability exchange response");

  if (!validate_handshake_request(resp_data)) {
    return false;
  }

  if (auto it = resp_data.find("sender");
      it != resp_data.end() && it->is_string()) {
    m_peer_sepp_fqdn = it->get<std::string>();
    Logger::sepp_app().info("Capability exchange response from peer SEPP: %s",
                            m_peer_sepp_fqdn.c_str());
  }

  if (resp_data.contains("3GppSbiTargetApiRootSupported")) {
    m_peer_target_api_root_supported =
        resp_data["3GppSbiTargetApiRootSupported"].get<bool>();
    Logger::sepp_app().info(
        "Peer SEPP confirmed 3GppSbiTargetApiRootSupported: %s",
        m_peer_target_api_root_supported ? "true" : "false");
  }

  if (auto it = resp_data.find("selectedSecCapability");
      it != resp_data.end() && it->is_string()) {
    const std::string selected_cap = it->get<std::string>();
    if (selected_cap != m_selected_sec_capability) {
      Logger::sepp_app().error(
          "Selected security capability '%s' differs from configured '%s'. "
          "Using configured value.",
          selected_cap.c_str(), m_selected_sec_capability.c_str());
    } else {
      Logger::sepp_app().info("Selected Security Capability: %s",
                              selected_cap.c_str());
    }
  } else {
    Logger::sepp_app().warn("Response missing selectedSecCapability");
    return false;
  }

  m_allowed_n32_purposes.clear();
  if (auto it = resp_data.find("allowedUsagePurpose");
      it != resp_data.end() && it->is_array()) {
    m_allowed_n32_purposes.reserve(it->size());
    for (const auto &item : *it) {
      if (item.is_object()) {
        if (auto p_it = item.find("usagePurpose");
            p_it != item.end() && p_it->is_string()) {
          m_allowed_n32_purposes.emplace_back(p_it->get<std::string>());
        }
      }
    }
  }

  return true;
}

//------------------------------------------------------------------------------
bool sepp_n32c_handshake::create_exchange_params_req(
    SecParamExchReqData &req_obj) {
  Logger::sepp_app().info("Creating SecParamExchReqData via OpenAPI model");

  if (m_n32c_context_id.empty()) {
    boost::uuids::random_generator gen;
    m_n32c_context_id = boost::uuids::to_string(gen());
  }

  req_obj.setN32fContextId(m_n32c_context_id);
  // TS 29.573 SecParamExchReqData: "sender" is the FQDN of this SEPP
  req_obj.setSender(m_sender_fqdn);
  req_obj.setJweCipherSuiteList({"A128GCM", "A256GCM"});
  req_obj.setJwsCipherSuiteList({"ES256", "RS256"});

  ProtectionPolicy protection_policy;
  protection_policy.setApiIeMappingList({});
  req_obj.setProtectionPolicyInfo(protection_policy);

  return true;
}

//------------------------------------------------------------------------------
bool sepp_n32c_handshake::create_exchange_params_req(nlohmann::json &req_data) {
  SecParamExchReqData req_obj;
  if (!create_exchange_params_req(req_obj)) {
    return false;
  }

  to_json(req_data, req_obj);
  return true;
}

//------------------------------------------------------------------------------
bool sepp_n32c_handshake::handle_exchange_params(
    const nlohmann::json &req_data, nlohmann::json &resp_data,
    std::unordered_map<std::string, std::string> &resp_headers) {
  Logger::sepp_app().info(
      "Handling N32-c parameter exchange request using model");

  if (!validate_handshake_request(req_data)) {
    return false;
  }

  SecParamExchReqData req_obj;
  try {
    from_json(req_data, req_obj);
  } catch (const std::exception &e) {
    Logger::sepp_app().error("Failed to parse SecParamExchReqData: %s",
                             e.what());
    return false;
  }

  std::stringstream err_ss;
  if (!req_obj.validate(err_ss)) {
    Logger::sepp_app().warn("SecParamExchReqData validation warning: %s",
                            err_ss.str().c_str());
  }

  resp_headers[SBI_TARGET_API_ROOT_HEADER] = HEADER_VALUE_YES;

  if (!req_obj.getN32fContextId().empty()) {
    m_n32c_context_id = req_obj.getN32fContextId();
  } else if (m_n32c_context_id.empty()) {
    boost::uuids::random_generator gen;
    m_n32c_context_id = boost::uuids::to_string(gen());
  }

  SecParamExchRspData resp_obj;
  resp_obj.setN32fContextId(m_n32c_context_id);
  // TS 29.573 SecParamExchRspData: "sender" is the FQDN of this SEPP
  resp_obj.setSender(m_sender_fqdn);

  std::string selected_jwe = "A128GCM";
  if (req_obj.jweCipherSuiteListIsSet() &&
      !req_obj.getJweCipherSuiteList().empty()) {
    selected_jwe = req_obj.getJweCipherSuiteList().front();
  }
  resp_obj.setSelectedJweCipherSuite(selected_jwe);
  m_selected_jwe_cipher = selected_jwe;

  std::string selected_jws = "ES256";
  if (req_obj.jwsCipherSuiteListIsSet() &&
      !req_obj.getJwsCipherSuiteList().empty()) {
    selected_jws = req_obj.getJwsCipherSuiteList().front();
  }
  resp_obj.setSelectedJwsCipherSuite(selected_jws);
  m_selected_jws_cipher = selected_jws;

  if (req_obj.protectionPolicyInfoIsSet()) {
    resp_obj.setSelProtectionPolicyInfo(req_obj.getProtectionPolicyInfo());
  } else {
    ProtectionPolicy sel_policy;
    sel_policy.setApiIeMappingList({});
    resp_obj.setSelProtectionPolicyInfo(sel_policy);
  }

  to_json(resp_data, resp_obj);

  Logger::sepp_app().info(
      "Processed N32-c parameter exchange. N32F Context ID: %s, JWE: "
      "%s, JWS: %s",
      m_n32c_context_id.c_str(), selected_jwe.c_str(), selected_jws.c_str());
  return true;
}

//------------------------------------------------------------------------------
bool sepp_n32c_handshake::handle_exchange_params_response(
    const nlohmann::json &resp_data) {
  Logger::sepp_app().info(
      "Processing N32-c parameter exchange response using model");

  if (!validate_handshake_request(resp_data)) {
    return false;
  }

  SecParamExchRspData rsp_obj;
  try {
    from_json(resp_data, rsp_obj);
  } catch (const std::exception &e) {
    Logger::sepp_app().error("Failed to parse SecParamExchRspData: %s",
                             e.what());
    return false;
  }

  std::stringstream err_ss;
  if (!rsp_obj.validate(err_ss)) {
    Logger::sepp_app().warn("SecParamExchRspData validation warning: %s",
                            err_ss.str().c_str());
  }

  if (!rsp_obj.getN32fContextId().empty()) {
    m_n32c_context_id = rsp_obj.getN32fContextId();
    Logger::sepp_app().info("Context ID established: %s",
                            m_n32c_context_id.c_str());
  }

  if (rsp_obj.selectedJweCipherSuiteIsSet()) {
    m_selected_jwe_cipher = rsp_obj.getSelectedJweCipherSuite();
    Logger::sepp_app().info("Negotiated JWE cipher: %s",
                            m_selected_jwe_cipher.c_str());
  }

  if (rsp_obj.selectedJwsCipherSuiteIsSet()) {
    m_selected_jws_cipher = rsp_obj.getSelectedJwsCipherSuite();
    Logger::sepp_app().info("Negotiated JWS cipher: %s",
                            m_selected_jws_cipher.c_str());
  }

  return true;
}
//------------------------------------------------------------------------------
bool sepp_n32c_handshake::create_n32f_terminate_req(N32fContextInfo &req_obj) {
  if (m_n32c_context_id.empty()) {
    Logger::sepp_app().warn(
        "Cannot create N32-f terminate request: N32-f Context ID is empty");
    return false;
  }

  Logger::sepp_app().info(
      "Creating N32-f Context Terminate request for context: %s",
      m_n32c_context_id.c_str());

  req_obj.setN32fContextId(m_n32c_context_id);
  return true;
}

//------------------------------------------------------------------------------
bool sepp_n32c_handshake::create_n32f_terminate_req(nlohmann::json &req_data) {
  N32fContextInfo req_obj;
  if (!create_n32f_terminate_req(req_obj)) {
    return false;
  }

  to_json(req_data, req_obj);
  return true;
}

//------------------------------------------------------------------------------
bool sepp_n32c_handshake::handle_n32f_terminate_req(
    const nlohmann::json &req_data, nlohmann::json &resp_data,
    std::unordered_map<std::string, std::string> &resp_headers) {
  Logger::sepp_app().info("Processing N32-f context termination request");

  if (!validate_handshake_request(req_data)) {
    return false;
  }

  N32fContextInfo term_req;
  try {
    from_json(req_data, term_req);
  } catch (const std::exception &e) {
    Logger::sepp_app().error(
        "Failed to deserialize N32fContextInfo for termination: %s", e.what());
    return false;
  }

  std::stringstream err_ss;
  if (!term_req.validate(err_ss)) {
    Logger::sepp_app().warn("N32fContextInfo validation warn: %s",
                            err_ss.str().c_str());
  }

  const std::string &incoming_context_id = term_req.getN32fContextId();
  Logger::sepp_app().info(
      "Terminating N32-f Context ID: %s (current local context: %s)",
      incoming_context_id.c_str(), m_n32c_context_id.c_str());

  resp_headers[SBI_TARGET_API_ROOT_HEADER] = HEADER_VALUE_YES;

  N32fContextInfo term_resp;
  term_resp.setN32fContextId(incoming_context_id);
  to_json(resp_data, term_resp);

  if (m_n32c_context_id == incoming_context_id || m_n32c_context_id.empty()) {
    m_n32c_context_id.clear();
  }

  Logger::sepp_app().info("N32-f context %s successfully terminated",
                          incoming_context_id.c_str());
  return true;
}
//------------------------------------------------------------------------------
bool sepp_n32c_handshake::handle_n32f_terminate_response(
    const nlohmann::json &resp_data) {
  Logger::sepp_app().info("Processing N32-f context termination response");

  if (resp_data.is_null() || resp_data.empty()) {
    m_n32c_context_id.clear();
    Logger::sepp_app().info("Peer confirmed N32-f context termination with "
                            "empty body (204 No Content)");
    return true;
  }

  try {
    N32fContextInfo term_rsp;
    from_json(resp_data, term_rsp);
    std::stringstream err_ss;
    if (!term_rsp.validate(err_ss)) {
      Logger::sepp_app().warn("N32fContextInfo response validation warning: %s",
                              err_ss.str().c_str());
    }
    Logger::sepp_app().info("Terminated context confirmed by peer: %s",
                            term_rsp.getN32fContextId().c_str());
  } catch (const std::exception &e) {
    Logger::sepp_app().warn(
        "Could not parse N32fContextInfo response model: %s", e.what());
  }

  m_n32c_context_id.clear();
  return true;
}

//------------------------------------------------------------------------------
bool sepp_n32c_handshake::handle_n32f_error(
    const nlohmann::json &req_data, nlohmann::json &resp_data,
    std::unordered_map<std::string, std::string> &resp_headers) {
  Logger::sepp_app().info("Processing N32-f error notification");

  if (!validate_handshake_request(req_data)) {
    return false;
  }

  resp_headers[SBI_TARGET_API_ROOT_HEADER] = HEADER_VALUE_YES;
  Logger::sepp_app().warn("N32-f error received from peer SEPP: %s",
                          req_data.dump().c_str());
  resp_data["acknowledged"] = true;
  return true;
}