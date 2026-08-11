/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef TASK_MANAGER_H_
#define TASK_MANAGER_H_

#include "sepp_event.hpp"

#include <linux/types.h>
#include <sys/timerfd.h>

using namespace oai::sepp::app;

namespace oai {
namespace sepp {
namespace app {

class sepp_event;
class task_manager {
public:
  task_manager(sepp_event &ev);

  /*
   * Manage the tasks
   * @param [void]
   * @return void
   */
  void manage_tasks();

  /*
   * Run the tasks (for the moment, simply call function manage_tasks)
   * @param [void]
   * @return void
   */
  void run();

private:
  /*
   * Make sure that the task tick run every 1ms
   * @param [void]
   * @return void
   */
  void wait_for_cycle();

  sepp_event &event_sub_;
  int sfd;
};
} // namespace app
} // namespace sepp
} // namespace oai

#endif
