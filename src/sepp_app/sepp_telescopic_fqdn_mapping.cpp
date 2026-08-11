/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "sepp_telescopic_fqdn_mapping.hpp"
#include "logger.hpp"
#include <nlohmann/json.hpp>
#include <sstream>

namespace oai::sepp::app {

bool sepp_telescopic_fqdn_mapping::is_roaming_partner(
    const std::string &plmnId) const {
  for (const auto &partner : m_roaming_partners) {
    std::string partnerMcc = partner.mcc.get_value();
    std::string partnerMnc = partner.mnc.get_value();

    std::string partnerMncPadded =
        (partnerMnc.length() == 2) ? ("0" + partnerMnc) : partnerMnc;

    std::string partnerPlmnRaw = partnerMcc + partnerMnc;
    std::string partnerPlmnPadded = partnerMcc + partnerMncPadded;

    if (partnerPlmnRaw == plmnId || partnerPlmnPadded == plmnId) {
      return true;
    }
  }
  return false;
}

//------------------------------------------------------------------------------
bool sepp_telescopic_fqdn_mapping::get_telescopic_mapping(
    const std::string &foreignFqdn, const std::string &telescopicLabel,
    oai::_3gpp::model::TelescopicMapping &mapping_model) {

  Logger::sepp_app().info(
      "Telescopic mapping request: foreignFqdn=%s, telescopicLabel=%s",
      foreignFqdn.c_str(), telescopicLabel.c_str());

  if (!foreignFqdn.empty() && telescopicLabel.empty()) {
    return fqdn_to_mapping(foreignFqdn, mapping_model);
  }

  if (foreignFqdn.empty() && !telescopicLabel.empty()) {
    return mapping_to_fqdn(telescopicLabel, mapping_model);
  }

  Logger::sepp_app().warn("Invalid telescopic mapping request: foreignFqdn=%s, "
                          "telescopicLabel=%s",
                          foreignFqdn.c_str(), telescopicLabel.c_str());
  return false;
}

//------------------------------------------------------------------------------
bool sepp_telescopic_fqdn_mapping::get_telescopic_mapping(
    const std::string &foreignFqdn, const std::string &telescopicLabel,
    std::string &telescopicMapping) {

  oai::_3gpp::model::TelescopicMapping mapping_model;
  if (!get_telescopic_mapping(foreignFqdn, telescopicLabel, mapping_model)) {
    return false;
  }

  nlohmann::json j;
  to_json(j, mapping_model);
  telescopicMapping = j.dump();
  return true;
}

//------------------------------------------------------------------------------
bool sepp_telescopic_fqdn_mapping::parse_fqdn(const std::string &fqdn,
                                              std::string &nfType,
                                              std::string &plmnId) const {

  const std::string suffix = ".5gc.";
  const auto pos = fqdn.find(suffix);
  if (pos == std::string::npos) {
    return false;
  }

  nfType = fqdn.substr(0, pos);
  const std::string remaining = fqdn.substr(pos + suffix.length());

  const std::string mncPrefix = "mnc";
  const std::string mccPrefix = ".mcc";
  const auto mccPos = remaining.find(mccPrefix);

  if (mccPos == std::string::npos) {
    return false;
  }

  const std::string mnc =
      remaining.substr(mncPrefix.length(), mccPos - mncPrefix.length());
  const std::string mccStart = remaining.substr(mccPos + mccPrefix.length());
  const std::string mccSuffix = ".3gppnetwork.org";

  if (mccStart.size() <= mccSuffix.size() ||
      mccStart.compare(mccStart.size() - mccSuffix.size(), mccSuffix.size(),
                       mccSuffix) != 0) {
    return false;
  }

  const std::string mcc =
      mccStart.substr(0, mccStart.size() - mccSuffix.size());

  plmnId = mcc + mnc;
  return true;
}

//------------------------------------------------------------------------------
bool sepp_telescopic_fqdn_mapping::fqdn_to_mapping(
    const std::string &foreignFqdn,
    oai::_3gpp::model::TelescopicMapping &mapping_model) {

  std::string nfType;
  std::string plmnId;

  if (!parse_fqdn(foreignFqdn, nfType, plmnId)) {
    Logger::sepp_app().warn("Invalid foreign FQDN: %s", foreignFqdn.c_str());
    return false;
  }

  if (m_allowed_nfs.find(nfType) == m_allowed_nfs.end()) {
    Logger::sepp_app().warn("NF type %s is not allowed for telescopic mapping",
                            nfType.c_str());
    return false;
  }

  if (!is_roaming_partner(plmnId)) {
    Logger::sepp_app().warn("PLMN %s is not allowed for telescopic mapping",
                            plmnId.c_str());
    return false;
  }

  mapping_model.setTelescopicLabel(generate_telescopic_label(nfType, plmnId));
  mapping_model.setSeppDomain(m_sepp_domain);

  std::stringstream err_ss;
  if (!mapping_model.validate(err_ss)) {
    Logger::sepp_app().warn(
        "Generated TelescopicMapping model validation warning: %s",
        err_ss.str().c_str());
  }

  return true;
}

//------------------------------------------------------------------------------
std::string sepp_telescopic_fqdn_mapping::generate_telescopic_label(
    const std::string &nfType, const std::string &plmnId) const {
  return nfType + "plmn" + plmnId;
}

//------------------------------------------------------------------------------
bool sepp_telescopic_fqdn_mapping::mapping_to_fqdn(
    const std::string &telescopicLabel,
    oai::_3gpp::model::TelescopicMapping &mapping_model) {

  const std::string keyword = "plmn";
  const size_t pos = telescopicLabel.find(keyword);

  if (pos != std::string::npos) {
    std::string nfType = telescopicLabel.substr(0, pos);
    std::string plmnId = telescopicLabel.substr(pos + keyword.length());

    if (m_allowed_nfs.count(nfType) && is_roaming_partner(plmnId)) {
      std::string mcc = plmnId.substr(0, 3);
      std::string mnc = plmnId.substr(3);

      if (mnc.length() == 2) {
        mnc = "0" + mnc;
      }

      std::string resolvedFqdn =
          nfType + ".5gc.mnc" + mnc + ".mcc" + mcc + ".3gppnetwork.org";

      mapping_model.setForeignFqdn(resolvedFqdn);
      mapping_model.setTelescopicLabel(telescopicLabel);
      mapping_model.setSeppDomain(m_sepp_domain);

      std::stringstream err_ss;
      if (!mapping_model.validate(err_ss)) {
        Logger::sepp_app().warn(
            "Generated TelescopicMapping model validation warning: %s",
            err_ss.str().c_str());
      }

      Logger::sepp_app().info("Resolved telescopic label %s -> %s",
                              telescopicLabel.c_str(), resolvedFqdn.c_str());
      return true;
    }
  }

  Logger::sepp_app().warn(
      "No foreign FQDN mapping found for telescopic label: %s",
      telescopicLabel.c_str());
  return false;
}

} // namespace oai::sepp::app