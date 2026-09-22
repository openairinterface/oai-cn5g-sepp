/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#pragma once

#include <boost/atomic.hpp>
#include <string>

#include "3gpp_29.500.h"
#include "PatchItem.h"
#include "sbi_helper.hpp"
#include "sepp_event.hpp"
#include "sepp_profile.hpp"

namespace oai::sepp::app {

class sepp_nrf {
  const uint32_t HEART_BEAT_TIMER = 10;

public:
  explicit sepp_nrf(sepp_event &ev);
  sepp_nrf(sepp_nrf const &) = delete;
  void operator=(sepp_nrf const &) = delete;

  virtual ~sepp_nrf();

  /**
   * Start event nf heartbeat procedure
   */
  void start_event_nf_heartbeat(std::string &remoteURI);
  /**
   * Trigger NF heartbeat procedure
   */
  void trigger_nf_heartbeat_procedure(uint64_t ms);
  /**
   * Trigger NF instance registration to NRF
   */
  void register_to_nrf();

  /**
   * Trigger NF instance de-registration to NRF
   */
  void deregister_to_nrf();

private:
  sepp_profile m_nf_instance_profile; // SEPP profile
  std::string m_sepp_instance_id;     // SEPP instance id
  // for Event Handling
  sepp_event &m_event_sub;
  bs2::connection m_task_connection;
  std::string m_nrf_url;

  /**
   * Generate SEPP profile and stores it in nf_instance_profile
   */
  void generate_sepp_profile();
};
} // namespace oai::sepp::app
