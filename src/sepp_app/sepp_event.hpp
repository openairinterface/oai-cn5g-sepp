/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef FILE_SEPP_EVENT_HPP_SEEN
#define FILE_SEPP_EVENT_HPP_SEEN

#include <boost/signals2.hpp>
#include <optional>
namespace bs2 = boost::signals2;

#include "sepp_event_sig.hpp"
#include "task_manager.hpp"

namespace oai::sepp::app {
class task_manager;
class sepp_event {
public:
  sepp_event(){};
  sepp_event(sepp_event const &) = delete;
  void operator=(sepp_event const &) = delete;

  static sepp_event &get_instance() {
    static sepp_event instance;
    return instance;
  }

  // class register/handle event
  friend class sepp_app;
  friend class sepp_nrf;
  friend class task_manager;
  // friend class sepp_policy_authorization;
  // friend class sepp_smpc;

  //------------------------------------------------------------------------------
  /*
   * Subscribe to the task tick event
   * @param [const task_sig_t::slot_type &] sig
   * @param [uint64_t] period: interval between two events
   * @param [uint64_t] start:
   * @return void
   */
  bs2::connection subscribe_task_nf_heartbeat(const task_sig_t::slot_type &sig,
                                              uint64_t period,
                                              uint64_t start = 0);
  //------------------------------------------------------------------------------
  /*
   * Subscribe to UE Loss of Connectivity Status signal
   * @param [const loss_of_connectivity_sig_t::slot_type&] sig: slot_type
   * parameter
   * @return boost::signals2::connection: the connection between the signal and
   * the slot
   */
  //   bs2::connection subscribe_loss_of_connectivity(
  //       const loss_of_connectivity_sig_t::slot_type& sig);

  /*
   * Subscribe to UE Reachability for Data signal
   * @param [const ue_reachability_for_data_sig_t::slot_type&] sig: slot_type
   * parameter
   * @return boost::signals2::connection: the connection between the signal and
   * the slot
   */
  //   bs2::connection subscribe_ue_reachability_for_data(
  //       const ue_reachability_for_data_sig_t::slot_type& sig);

  /**
   * Subscribe to SM Session Binding signal.
   * @param [const sm_session_binding_sig_t::slot_type&] sig: slot_type
   * parameter
   * @return boost::signals2::connection: the connection between the signal and
   * the slot
   */
  //   bs2::connection subscribe_sm_session_binding(
  //       const sm_session_binding_sig_t::slot_type& sig);

  /**
   * Subscribe to SM Update Decision signal.
   * @param [const sm_update_decision_sig_t::slot_type&] sig: slot_type
   * parameter
   * @return boost::signals2::connection: the connection between the signal and
   * the slot
   */
  //   bs2::connection subscribe_sm_update_decision(
  //       const sm_update_decision_sig_t::slot_type& sig);

private:
  task_sig_t task_tick;

  //   loss_of_connectivity_sig_t
  //       loss_of_connectivity;  // Signal for Loss of Connectivity Report
  //   ue_reachability_for_data_sig_t
  //       ue_reachability_for_data;  // Signal for UE Reachability for Data
  //       Report

  //   sm_session_binding_sig_t sm_session_binding;  // Signal for SM Session
  //   Binding

  //   sm_update_decision_sig_t sm_update_decision;  // Signal for SM Update
  //   Decision
};
} // namespace oai::sepp::app
#endif
