/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef FILE_SEPP_TELESCOPIC_FQDN_MAPPING_HPP_SEEN
#define FILE_SEPP_TELESCOPIC_FQDN_MAPPING_HPP_SEEN

#pragma once

#include "TelescopicMapping.h"
#include "config.hpp"

#include <string>
#include <unordered_set>
#include <vector>

namespace oai::sepp::app {

class sepp_telescopic_fqdn_mapping {
  friend class sepp_app;

public:
  sepp_telescopic_fqdn_mapping() = default;
  ~sepp_telescopic_fqdn_mapping() = default;

  void set_roaming_partners(
      const std::vector<oai::config::plmn_config> &roaming_partners) {
    m_roaming_partners = roaming_partners;
  }

  // Model-based mapping resolution
  bool
  get_telescopic_mapping(const std::string &foreignFqdn,
                         const std::string &telescopicLabel,
                         oai::_3gpp::model::TelescopicMapping &mapping_model);

  // Backward-compatible JSON string overload
  bool get_telescopic_mapping(const std::string &foreignFqdn,
                              const std::string &telescopicLabel,
                              std::string &telescopicMapping);

private:
  bool fqdn_to_mapping(const std::string &foreignFqdn,
                       oai::_3gpp::model::TelescopicMapping &mapping_model);

  bool mapping_to_fqdn(const std::string &telescopicLabel,
                       oai::_3gpp::model::TelescopicMapping &mapping_model);

  bool parse_fqdn(const std::string &fqdn, std::string &nfType,
                  std::string &plmnId) const;

  std::string generate_telescopic_label(const std::string &nfType,
                                        const std::string &plmnId) const;

  bool is_roaming_partner(const std::string &plmnId) const;

  std::unordered_set<std::string> m_allowed_nfs = {"nrf", "ausf", "smf", "amf",
                                                   "udm", "pcf",  "nssf"};

  std::vector<oai::config::plmn_config> m_roaming_partners;

  const std::string m_sepp_domain = "sepp.5gc.mnc001.mcc001.3gppnetwork.org";
};

} // namespace oai::sepp::app
#endif /* FILE_SEPP_TELESCOPIC_FQDN_MAPPING_HPP_SEEN */