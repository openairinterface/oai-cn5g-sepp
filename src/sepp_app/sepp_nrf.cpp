/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "sepp_nrf.hpp"
#include "3gpp_29.500.h"
#include "3gpp_29.510.h"
#include "Snssai.h"
#include "api_defs.h"
#include "http_client.hpp"
#include "logger.hpp"
#include "sbi_helper.hpp"
#include "sepp_config.hpp"
#include <iostream>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

using namespace oai::sepp::app;
using namespace oai::config::sepp;
using namespace oai::_3gpp::model;
using namespace boost::placeholders;
using namespace std;

extern std::unique_ptr<sepp_config> sepp_cfg;
extern std::shared_ptr<oai::http::http_client> http_client_inst;

//------------------------------------------------------------------------------
sepp_nrf::sepp_nrf(sepp_event &ev) : m_event_sub(ev) {
  m_sepp_instance_id = to_string(boost::uuids::random_generator()());
  generate_sepp_profile();
  nf_addr_t nf_addr;
  nf_addr.api_version =
      sepp_cfg->get_nf(config::NRF_CONFIG_NAME)->get_sbi().get_api_version();
  nf_addr.uri_root =
      sepp_cfg->get_nf(config::NRF_CONFIG_NAME)->get_sbi().get_url();

  sbi_helper::get_nrf_nf_instance_uri(nf_addr, m_sepp_instance_id, m_nrf_url);
}

//---------------------------------------------------------------------------------------------
void sepp_nrf::generate_sepp_profile() {
  // TODO: remove hardcoded values
  // generate UUID
  m_nf_instance_profile.set_nf_instance_id(m_sepp_instance_id);
  m_nf_instance_profile.set_nf_instance_name("OAI-SEPP");
  m_nf_instance_profile.set_nf_type("SEPP");
  m_nf_instance_profile.set_nf_status("REGISTERED");
  m_nf_instance_profile.set_fqdn(sepp_cfg->local().get_sbi().get_host());
  m_nf_instance_profile.set_nf_heartBeat_timer(50);
  m_nf_instance_profile.set_nf_priority(1);
  m_nf_instance_profile.set_nf_capacity(100);
  m_nf_instance_profile.add_nf_ipv4_addresses(
      sepp_cfg->local().get_sbi().get_addr4());

  // NF services
  nf_service_t nf_service = {};
  nf_service.service_instance_id = oai::sepp::api::sepp_telescoic_API::API_NAME;
  nf_service.service_name = oai::sepp::api::sepp_telescoic_API::API_NAME;
  nf_service_version_t version = {};
  version.api_version_in_uri = sepp_cfg->local().get_sbi().get_api_version();
  version.api_full_version = "1.0.0"; // TODO: to be updated
  nf_service.versions.push_back(version);
  nf_service.scheme = "http";
  nf_service.nf_service_status = "REGISTERED";
  // IP Endpoint
  ip_endpoint_t endpoint = {};
  // TODO: use only one IP address from cfg for now
  endpoint.ipv4_address = sepp_cfg->local().get_sbi().get_addr4();
  endpoint.transport = "TCP";
  endpoint.port = sepp_cfg->local().get_sbi().get_port();
  nf_service.ip_endpoints.push_back(endpoint);

  m_nf_instance_profile.add_nf_service(nf_service);

  // SEPP info
  sepp_info_t sepp_info_item;
  sepp_info_item.m_SeppPrefix = "oai-sepp-prefix";
  sepp_info_item.m_SeppPorts.emplace("n32f", 443);
  sepp_info_item.m_SeppPorts.emplace("n32c", 443);
  sepp_info_item.m_RemotePlmnList.emplace_back(plmn_t{"262", "10"});
  // ToDo: sepp_info_item.m_RemoteSnpnList.emplace_back()
  sepp_info_item.m_n32Purposes.push_back("ROAMING");

  m_nf_instance_profile.set_sepp_info(sepp_info_item);
  // Display the profile
  m_nf_instance_profile.display();
}

//---------------------------------------------------------------------------------------------
void sepp_nrf::register_to_nrf() {
  nlohmann::json body;
  m_nf_instance_profile.to_json(body);

  Logger::sepp_sbi().info("Sending NF registration request to NRF: %s",
                          m_nrf_url.c_str());
  auto request = http_client_inst->prepare_json_request(m_nrf_url, body.dump());
  auto http_response =
      http_client_inst->send_http_request(method_e::PUT, request);

  if (http_response.status_code == http_status_code::CREATED ||
      http_response.status_code == http_status_code::OK) {
    try {
      if (http_response.body.find("REGISTERED") != 0) {
        start_event_nf_heartbeat(m_nrf_url);
      }
      Logger::sepp_sbi().debug("NF registration successful");
    } catch (nlohmann::json::exception &e) {
      Logger::sepp_sbi().warn("NF registration procedure failed");
    }
  } else {
    Logger::sepp_sbi().warn("NF registration failed: Wrong response code: %d",
                            http_response.status_code);
  }
}
//------------------------------------------------------------------------------
void sepp_nrf::start_event_nf_heartbeat(std::string & /* remoteURI */) {
  // get current time
  uint64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();
  const uint64_t interval = HEART_BEAT_TIMER * 1000; // ms

  m_task_connection = m_event_sub.subscribe_task_nf_heartbeat(
      boost::bind(&sepp_nrf::trigger_nf_heartbeat_procedure, this, _1),
      interval, ms + interval);
}

//---------------------------------------------------------------------------------------------
void sepp_nrf::trigger_nf_heartbeat_procedure(uint64_t /* ms */) {

  PatchItem patch_item = {};
  std::vector<PatchItem> patch_items;
  PatchOperation op;
  op.setEnumValue(PatchOperation_anyOf::ePatchOperation_anyOf::REPLACE);
  patch_item.setOp(op);
  patch_item.setPath("/nfStatus");
  patch_item.setValue("REGISTERED");
  patch_items.push_back(patch_item);
  Logger::sepp_sbi().info("Sending NF heartbeat request");

  nlohmann::json json_data = nlohmann::json::array();
  json_data.push_back(patch_item);

  auto request =
      http_client_inst->prepare_json_request(m_nrf_url, json_data.dump());
  auto http_response =
      http_client_inst->send_http_request(method_e::PATCH, request);

  if (http_response.status_code == http_status_code::OK ||
      http_response.status_code == http_status_code::NO_CONTENT) {
    Logger::sepp_sbi().debug("NF heartbeat request successful");
  } else {
    // TODO what should we do in this case?
    // We disconnect, but we dont trigger anything else
    Logger::sepp_sbi().warn(
        "NF heartbeat request failed. Wrong response code %d",
        http_response.status_code);
    m_task_connection.disconnect();
  }
}
//------------------------------------------------------------------------------
sepp_nrf::~sepp_nrf() {
  Logger::sepp_sbi().debug("Delete SEPP_NRF instance...");
}

//------------------------------------------------------------------------------
void sepp_nrf::deregister_to_nrf() {
  std::string body_response;
  std::string response_header;

  Logger::sepp_sbi().info("Sending NF de-registration request");

  http::request req;
  req.uri = m_nrf_url;
  auto http_response =
      http_client_inst->send_http_request(method_e::DELETE, req);

  if (http_response.status_code == http_status_code::NO_CONTENT) {
    Logger::sepp_sbi().info("NF Deregistration successful");
  } else {
    Logger::sepp_sbi().warn("NF Deregistration failed! Wrong response code: %d",
                            http_response.status_code);
  }
}
