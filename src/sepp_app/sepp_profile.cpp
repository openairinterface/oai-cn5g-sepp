/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include "logger.hpp"
#include "sepp_profile.hpp"
#include "string.hpp"

using namespace std;
using namespace oai::sepp::app;
using namespace oai::common::sbi;

//------------------------------------------------------------------------------
void nf_profile::set_nf_instance_id(const std::string &instance_id) {
  nf_instance_id = instance_id;
}

//------------------------------------------------------------------------------
void nf_profile::get_nf_instance_id(std::string &instance_id) const {
  instance_id = nf_instance_id;
}

//------------------------------------------------------------------------------
std::string nf_profile::get_nf_instance_id() const { return nf_instance_id; }

//------------------------------------------------------------------------------
void nf_profile::set_nf_instance_name(const std::string &instance_name) {
  nf_instance_name = instance_name;
}

//------------------------------------------------------------------------------
void nf_profile::get_nf_instance_name(std::string &instance_name) const {
  instance_name = nf_instance_name;
}

//------------------------------------------------------------------------------
std::string nf_profile::get_nf_instance_name() const {
  return nf_instance_name;
}

//------------------------------------------------------------------------------
void nf_profile::set_nf_type(const std::string &type) { nf_type = type; }

//------------------------------------------------------------------------------
std::string nf_profile::get_nf_type() const { return nf_type; }

//------------------------------------------------------------------------------
void nf_profile::set_nf_status(const std::string &status) {
  nf_status = status;
}

//------------------------------------------------------------------------------
void nf_profile::get_nf_status(std::string &status) const {
  status = nf_status;
}

//------------------------------------------------------------------------------
std::string nf_profile::get_nf_status() const { return nf_status; }

//------------------------------------------------------------------------------
void nf_profile::set_nf_heartBeat_timer(const int32_t &timer) {
  heartBeat_timer = timer;
}

//------------------------------------------------------------------------------
void nf_profile::get_nf_heartBeat_timer(int32_t &timer) const {
  timer = heartBeat_timer;
}

//------------------------------------------------------------------------------
int32_t nf_profile::get_nf_heartBeat_timer() const { return heartBeat_timer; }

//------------------------------------------------------------------------------
void nf_profile::set_nf_priority(const uint16_t &p) { priority = p; }

//------------------------------------------------------------------------------
void nf_profile::get_nf_priority(uint16_t &p) const { p = priority; }

//------------------------------------------------------------------------------
uint16_t nf_profile::get_nf_priority() const { return priority; }

//------------------------------------------------------------------------------
void nf_profile::set_nf_capacity(const uint16_t &c) { capacity = c; }

//------------------------------------------------------------------------------
void nf_profile::get_nf_capacity(uint16_t &c) const { c = capacity; }

//------------------------------------------------------------------------------
uint16_t nf_profile::get_nf_capacity() const { return capacity; }

//------------------------------------------------------------------------------
void nf_profile::set_nf_snssais(const std::vector<snssai_t> &s) { snssais = s; }

//------------------------------------------------------------------------------
void nf_profile::get_nf_snssais(std::vector<snssai_t> &s) const { s = snssais; }

//------------------------------------------------------------------------------
void nf_profile::add_snssai(const snssai_t &s) { snssais.push_back(s); }

//------------------------------------------------------------------------------
void nf_profile::set_fqdn(const std::string &fqdN) { fqdn = fqdN; }

//------------------------------------------------------------------------------
std::string nf_profile::get_fqdn() const { return fqdn; }

//------------------------------------------------------------------------------
void nf_profile::set_nf_ipv4_addresses(const std::vector<struct in_addr> &a) {
  ipv4_addresses = a;
}

//------------------------------------------------------------------------------
void nf_profile::add_nf_ipv4_addresses(const struct in_addr &a) {
  ipv4_addresses.push_back(a);
}

//------------------------------------------------------------------------------
void nf_profile::set_nf_ipv6_addresses(const std::vector<struct in6_addr> &a) {
  ipv6_addresses = a;
}

//------------------------------------------------------------------------------
void nf_profile::add_nf_ipv6_addresses(const struct in6_addr &a) {
  ipv6_addresses.push_back(a);
}

//------------------------------------------------------------------------------
void nf_profile::get_nf_ipv4_addresses(std::vector<struct in_addr> &a) const {
  a = ipv4_addresses;
}

//------------------------------------------------------------------------------
void nf_profile::display() const {
  Logger::sepp_app().debug("NF instance info");
  Logger::sepp_app().debug("\tInstance ID: %s", nf_instance_id.c_str());

  Logger::sepp_app().debug("\tInstance name: %s", nf_instance_name.c_str());
  Logger::sepp_app().debug("\tInstance type: %s", nf_type.c_str());
  Logger::sepp_app().debug("\tStatus: %s", nf_status.c_str());
  Logger::sepp_app().debug("\tHeartBeat timer: %d", heartBeat_timer);
  Logger::sepp_app().debug("\tPriority: %d", priority);
  Logger::sepp_app().debug("\tCapacity: %d", capacity);
  // SNSSAIs
  if (snssais.size() > 0) {
    Logger::sepp_app().debug("\tSNSSAI:");
  }
  for (auto s : snssais) {
    Logger::sepp_app().debug("\t\t SST %d, SD %d", s.sst, s.sd);
  }
  if (!fqdn.empty()) {
    Logger::sepp_app().debug("\tFQDN: %s", fqdn.c_str());
  }
  // IPv4 Addresses
  if (ipv4_addresses.size() > 0) {
    Logger::sepp_app().debug("\tIPv4 Addr:");
  }
  for (auto address : ipv4_addresses) {
    Logger::sepp_app().debug("\t\t %s", inet_ntoa(address));
  }
  // IPv6 Addresses
  // if (ipv6_addresses.size() > 0) {
  //   Logger::sepp_app().debug("\tIPv6 Addr:");
  // }
  // for (auto address : ipv6_addresses) {
  //   Logger::sepp_app().debug("\t\t %s", inet_ntoa(address));
  // }
}

//------------------------------------------------------------------------------
void nf_profile::to_json(nlohmann::json &data) const {
  data["nfInstanceId"] = nf_instance_id;
  data["nfInstanceName"] = nf_instance_name;
  data["nfType"] = nf_type;
  data["nfStatus"] = nf_status;
  data["heartBeatTimer"] = heartBeat_timer;
  // SNSSAIs
  data["sNssais"] = nlohmann::json::array();
  for (auto s : snssais) {
    nlohmann::json tmp = {};
    tmp["sst"] = s.sst;
    tmp["sd"] = s.sd;
    data["sNssais"].push_back(tmp);
  }
  if (!fqdn.empty()) {
    data["fqdn"] = fqdn;
  }
  // ipv4_addresses
  data["ipv4Addresses"] = nlohmann::json::array();
  for (auto address : ipv4_addresses) {
    data["ipv4Addresses"].push_back(inet_ntoa(address));
  }
  // // ipv6_addresses
  // data["ipv6Addresses"] = nlohmann::json::array();
  // for (auto address : ipv6_addresses) {
  //   nlohmann::json tmp = inet_ntoa(address);
  //   data["ipv6Addresses"].push_back(tmp);
  // }
  data["priority"] = priority;
  data["capacity"] = capacity;
}

//------------------------------------------------------------------------------
void nf_profile::from_json(const nlohmann::json &data) {
  if (data.find("nfInstanceId") != data.end()) {
    nf_instance_id = data["nfInstanceId"].get<std::string>();
  }

  if (data.find("nfInstanceName") != data.end()) {
    nf_instance_name = data["nfInstanceName"].get<std::string>();
  }

  if (data.find("nfType") != data.end()) {
    nf_type = data["nfType"].get<std::string>();
  }

  if (data.find("nfStatus") != data.end()) {
    nf_status = data["nfStatus"].get<std::string>();
  }

  if (data.find("heartBeatTimer") != data.end()) {
    heartBeat_timer = data["heartBeatTimer"].get<int>();
  }
  // sNssais
  if (data.find("sNssais") != data.end()) {
    for (auto it : data["sNssais"]) {
      snssai_t s(it["sst"].get<int>(), it["sd"].get<std::string>());

      snssais.push_back(s);
    }
  }

  if (data.find("fqdn") != data.end()) {
    fqdn = data["fqdn"].get<std::string>();
  }

  if (data.find("ipv4Addresses") != data.end()) {
    nlohmann::json addresses = data["ipv4Addresses"];

    for (auto it : addresses) {
      struct in_addr addr4 = {};
      std::string address = it.get<std::string>();
      unsigned char buf_in_addr[sizeof(struct in_addr)];
      if (inet_pton(AF_INET, oai::utils::trim(address).c_str(), buf_in_addr) ==
          1) {
        memcpy(&addr4, buf_in_addr, sizeof(struct in_addr));
      } else {
        Logger::sepp_app().warn("Address conversion: Bad value %s",
                                oai::utils::trim(address).c_str());
      }
      add_nf_ipv4_addresses(addr4);
    }
  }

  // ToDo: ipv6Addresses

  if (data.find("priority") != data.end()) {
    priority = data["priority"].get<int>();
  }

  if (data.find("capacity") != data.end()) {
    capacity = data["capacity"].get<int>();
  }
}

//------------------------------------------------------------------------------
void sepp_profile::set_nf_services(const std::vector<nf_service_t> &n) {
  nf_services = n;
}

//------------------------------------------------------------------------------
void sepp_profile::add_nf_service(const nf_service_t &n) {
  nf_services.push_back(n);
}

//------------------------------------------------------------------------------
void sepp_profile::get_nf_services(std::vector<nf_service_t> &n) const {
  n = nf_services;
}

//------------------------------------------------------------------------------
void sepp_profile::set_custom_info(const nlohmann::json &c) { custom_info = c; }

//------------------------------------------------------------------------------
void sepp_profile::get_custom_info(nlohmann::json &c) const { c = custom_info; }

//------------------------------------------------------------------------------
void sepp_profile::set_sepp_info(const sepp_info_t &s) { sepp_info = s; }

//------------------------------------------------------------------------------
void sepp_profile::get_sepp_info(sepp_info_t &s) const { s = sepp_info; }

//------------------------------------------------------------------------------
void sepp_profile::display() const {
  Logger::sepp_app().debug("- NF instance info");
  Logger::sepp_app().debug("    Instance ID: %s", nf_instance_id.c_str());
  Logger::sepp_app().debug("    Instance name: %s", nf_instance_name.c_str());
  Logger::sepp_app().debug("    Instance type: %s", nf_type.c_str());
  Logger::sepp_app().debug("    Instance fqdn: %s", fqdn.c_str());
  Logger::sepp_app().debug("    Status: %s", nf_status.c_str());
  Logger::sepp_app().debug("    HeartBeat timer: %d", heartBeat_timer);
  Logger::sepp_app().debug("    Priority: %d", priority);
  Logger::sepp_app().debug("    Capacity: %d", capacity);

  // SNSSAIs
  if (snssais.size() > 0) {
    Logger::sepp_app().debug("    SNSSAI:");
  }
  for (auto s : snssais) {
    Logger::sepp_app().debug(s.toString());
  }

  // IPv4 Addresses
  if (ipv4_addresses.size() > 0) {
    Logger::sepp_app().debug("    IPv4 Addr:");
  }
  for (auto address : ipv4_addresses) {
    Logger::sepp_app().debug("        %s", inet_ntoa(address));
  }
  Logger::sepp_app().debug("\tSEPP Info");
  Logger::sepp_app().debug("\t\t SeppPrefix: %s",
                           sepp_info.m_SeppPrefix.c_str());

  Logger::sepp_app().debug("\t\t SeppPorts:");
  for (const auto &tmp : sepp_info.m_SeppPorts) {
    Logger::sepp_app().debug("\t\t\t Interface: %s - Port: %d", tmp.first,
                             tmp.second);
  }
  Logger::sepp_app().debug("\t\t SeppPorts:");

  // for (auto supi : sepp_info.supi_ranges) {
  //   Logger::sepp_app().debug(
  //       "\t\t SupiRanges: Start - %s, End - %s, Pattern - %s",
  //       supi.supi_range.start.c_str(), supi.supi_range.end.c_str(),
  //       supi.supi_range.pattern.c_str());
  // }
  // for (auto gpsi : sepp_info.gpsi_ranges) {
  //   Logger::sepp_app().debug(
  //       "\t\t GpsiRanges: Start - %s, End - %s, Pattern - %s",
  //       gpsi.identity_range.start.c_str(), gpsi.identity_range.end.c_str(),
  //       gpsi.identity_range.pattern.c_str());
  // }
  // for (auto dnn : sepp_info.dnn_list) {
  //   Logger::sepp_app().debug("\t\t DNN: %s", dnn.c_str());
  // }
}

//------------------------------------------------------------------------------
void sepp_profile::to_json(nlohmann::json &data) const {
  nf_profile::to_json(data);

  // NF services
  data["nfServices"] = nlohmann::json::array();
  for (auto service : nf_services) {
    nlohmann::json srv_tmp = {};
    srv_tmp["serviceInstanceId"] = service.service_instance_id;
    srv_tmp["serviceName"] = service.service_name;
    srv_tmp["versions"] = nlohmann::json::array();
    for (auto const &v : service.versions) {
      nlohmann::json v_tmp = {};
      v_tmp["apiVersionInUri"] = v.api_version_in_uri;
      v_tmp["apiFullVersion"] = v.api_full_version;
      srv_tmp["versions"].push_back(v_tmp);
    }
    srv_tmp["scheme"] = service.scheme;
    srv_tmp["nfServiceStatus"] = service.nf_service_status;
    // IP endpoints
    srv_tmp["ipEndPoints"] = nlohmann::json::array();
    for (auto endpoint : service.ip_endpoints) {
      nlohmann::json ep_tmp = {};
      ep_tmp["ipv4Address"] = inet_ntoa(endpoint.ipv4_address);
      ep_tmp["transport"] = endpoint.transport;
      ep_tmp["port"] = endpoint.port;
      srv_tmp["ipEndPoints"].push_back(ep_tmp);
    }

    data["nfServices"].push_back(srv_tmp);
  }

  data["custom_info"] = custom_info;

  // sepp info
  data["seppInfo"]["seppPrefix"] = sepp_info.m_SeppPrefix;
  data["seppInfo"]["seppPorts"] = nlohmann::json::object();
  for (const auto &[Interface, Port] : sepp_info.m_SeppPorts) {
    data["seppInfo"]["seppPorts"][Interface] = Port;
  }
  data["seppInfo"]["remotePlmnList"] = nlohmann::json::array();

  for (const auto &plmn : sepp_info.m_RemotePlmnList) {
    data["seppInfo"]["remotePlmnList"].push_back(
        {{"mcc", plmn.mcc}, {"mnc", plmn.mnc}});
  }
  data["seppInfo"]["n32Purposes"] = nlohmann::json::array();

  for (const auto &purpose : sepp_info.m_n32Purposes) {
    data["seppInfo"]["n32Purposes"].push_back(purpose);
  }
  Logger::sepp_app().debug("sepp profile to json:\n %s", data.dump().c_str());
}

//------------------------------------------------------------------------------
void sepp_profile::from_json(const nlohmann::json &data) {
  nf_profile::from_json(data);

  if (data.find("nfInstanceId") != data.end()) {
    nf_instance_id = data["nfInstanceId"].get<std::string>();
  }

  if (data.find("nfInstanceName") != data.end()) {
    nf_instance_name = data["nfInstanceName"].get<std::string>();
  }

  if (data.find("nfType") != data.end()) {
    nf_type = data["nfType"].get<std::string>();
  }

  if (data.find("nfStatus") != data.end()) {
    nf_status = data["nfStatus"].get<std::string>();
  }

  if (data.find("heartBeatTimer") != data.end()) {
    heartBeat_timer = data["heartBeatTimer"].get<int>();
  }
  // sNssais
  if (data.find("sNssais") != data.end()) {
    for (auto it : data["sNssais"]) {
      snssai_t s(it["sst"].get<int>(), it["sd"].get<std::string>());
      snssais.push_back(s);
    }
  }
  if (data.find("ipv4Addresses") != data.end()) {
    nlohmann::json addresses = data["ipv4Addresses"];

    for (auto it : addresses) {
      struct in_addr addr4 = {};
      std::string address = it.get<std::string>();
      unsigned char buf_in_addr[sizeof(struct in_addr)];
      if (inet_pton(AF_INET, oai::utils::trim(address).c_str(), buf_in_addr) ==
          1) {
        memcpy(&addr4, buf_in_addr, sizeof(struct in_addr));
      } else {
        Logger::sepp_app().warn("Address conversion: Bad value %s",
                                oai::utils::trim(address).c_str());
      }
      add_nf_ipv4_addresses(addr4);
    }
  }
  if (data.find("priority") != data.end()) {
    priority = data["priority"].get<int>();
  }

  if (data.find("capacity") != data.end()) {
    capacity = data["capacity"].get<int>();
  }
  // sepp info
  if (data.find("seppInfo") != data.end()) {
    nlohmann::json info = data["seppInfo"];
    if (info.find("seppPrefix") != info.end()) {
      sepp_info.m_SeppPrefix = info["seppPrefix"].get<std::string>();
    }
    // if (data.find("seppPorts") != data.end()) {

    // }
    //   if (info.find("dnnList") != info.end()) {
    //     nlohmann::json dnnList = data["seppInfo"]["dnnList"];
    //     for (auto d : dnnList) {
    //       sepp_info.dnn_list.push_back(d);
    //     }
    //   }
    //   if (info.find("supiRanges") != info.end()) {
    //     nlohmann::json supi_ranges = data["seppInfo"]["supiRanges"];
    //     for (auto d : supi_ranges) {
    //       supi_range_info_item_t supi;
    //       supi.supi_range.start   = d["start"];
    //       supi.supi_range.end     = d["end"];
    //       supi.supi_range.pattern = d["pattern"];
    //       sepp_info.supi_ranges.push_back(supi);
    //     }
    //   }
    //   if (info.find("gpsiRanges") != info.end()) {
    //     nlohmann::json gpsi_ranges = data["seppInfo"]["gpsiRanges"];
    //     for (auto d : gpsi_ranges) {
    //       identity_range_info_item_t gpsi;
    //       gpsi.identity_range.start   = d["start"];
    //       gpsi.identity_range.end     = d["end"];
    //       gpsi.identity_range.pattern = d["pattern"];
    //       sepp_info.gpsi_ranges.push_back(gpsi);
    //     }

    //     // TODO: custom_info;
  }
  // }

  display();
}

//------------------------------------------------------------------------------
void sepp_profile::handle_heartbeart_timeout(uint64_t ms) {
  Logger::sepp_app().info("Handle heartbeart timeout profile %s, time %d",
                          nf_instance_id.c_str(), ms);
  set_nf_status("SUSPENDED");
}
