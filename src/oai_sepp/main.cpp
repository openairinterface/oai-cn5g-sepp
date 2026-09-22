/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "common_defs.h"
#include "http_client.hpp"
#include "logger.hpp"
#include "options.hpp"
#include "pid_file.hpp"
#include "pistache/endpoint.h"
#include "pistache/http.h"
#include "pistache/router.h"
#include "sepp-api-server.h"
#include "sepp-http2-nbi-server.h"
#include "sepp-http2-sbi-server.h"
#include "sepp_app.hpp"
#include "sepp_config.hpp"

#include <algorithm>
#include <boost/asio.hpp>
#include <chrono>
#include <iostream>
#include <signal.h>
#include <stdint.h>
#include <thread>
#include <unistd.h> // get_pid(), pause()
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>

using namespace std;
using namespace oai::utils;
using namespace oai::sepp::app;
using namespace oai::sepp::api;
using namespace oai::config::sepp;

using namespace oai::config;
// sepp_app* sepp_app_inst = nullptr;
std::unique_ptr<sepp_app> sepp_app_inst = nullptr;
std::unique_ptr<sepp_config> sepp_cfg = nullptr;
std::unique_ptr<SEPPApiServer> sepp_api_server_1 = nullptr;
std::unique_ptr<sepp_http2_sbi_server> sepp_sbi_server_2 = nullptr;
std::unique_ptr<sepp_http2_nbi_server> sepp_nbi_server_2 = nullptr;
std::shared_ptr<oai::http::http_client> http_client_inst = nullptr;
std::shared_ptr<oai::http::http_client> http_client_inst_nbi = nullptr;
std::unique_ptr<task_manager> tm_inst = nullptr;
//------------------------------------------------------------------------------
void my_app_signal_handler(int s) {
  auto shutdown_start = std::chrono::system_clock::now();
  // Setting log level arbitrarly to debug to show the whole
  // shutdown procedure in the logs even in case of off-logging
  Logger::set_level(spdlog::level::debug);
  Logger::system().info("Exiting: caught signal %d", s);

  Logger::system().debug("Shutting down HTTP servers...");
  if (sepp_api_server_1) {
    sepp_api_server_1->shutdown();
  }
  if (sepp_sbi_server_2) {
    sepp_sbi_server_2->stop();
  }
  if (sepp_app_inst) {
    sepp_app_inst->stop();
  }
  // TODO exit is not always clean, check again after complete refactor
  // Ensure that objects are destructed before static libraries (e.g. Logger)
  Logger::system().debug("Freeing Allocated memory...");
  sepp_api_server_1 = nullptr;
  sepp_sbi_server_2 = nullptr;
  sepp_app_inst = nullptr;
  sepp_cfg = nullptr;

  Logger::system().debug("SEPP APP memory done");
  // if (tm_inst) {
  //   delete tm_inst;
  //   tm_inst = nullptr;
  // }
  Logger::system().debug("Stopped the UDM Task Manager.");

  Logger::system().debug("Freeing allocated memory done");
  auto elapsed = std::chrono::system_clock::now() - shutdown_start;
  auto ms_diff = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
  Logger::system().info("Bye. Shutdown Procedure took %d ms", ms_diff.count());
  exit(0);
}
//------------------------------------------------------------------------------
int main(int argc, char **argv) {
  // Command line options
  if (!Options::parse(argc, argv)) {
    std::cout << "Options::parse() failed" << std::endl;
    return 1;
  }

  // Logger
  Logger::init("sepp", Options::getlogStdout(), Options::getlogRotFilelog());

  std::signal(SIGTERM, my_app_signal_handler);
  std::signal(SIGINT, my_app_signal_handler);

  sepp_cfg = std::make_unique<sepp_config>(Options::getlibconfigConfig(),
                                           Options::getlogStdout(),
                                           Options::getlogRotFilelog());

  if (!sepp_cfg->init()) {
    sepp_cfg->display();
    Logger::system().error("Reading the configuration failed. Exiting");
    return 1;
  }

  sepp_cfg->display();

  // // HTTP Client
  // http_client_inst = oai::http::http_client::create_instance(
  //     Logger::sepp_client(), oai::common::sbi::kNfDefaultHttpRequestTimeout,
  //     sepp_cfg->local().get_sbi().get_if_name(),
  //     sepp_cfg->get_http_version());

  //   // HTTP Client
  // http_client_inst_nbi = oai::http::http_client::create_instance(
  //     Logger::sepp_client(), oai::common::sbi::kNfDefaultHttpRequestTimeout,
  //     sepp_cfg->local().get_nbi().get_if_name(),
  //     sepp_cfg->get_http_version());

  // HTTP Client for SBI
  http_client_inst = std::make_shared<oai::http::http_client>(
      Logger::sepp_client(), oai::common::sbi::kNfDefaultHttpRequestTimeout,
      sepp_cfg->local().get_sbi().get_if_name(), sepp_cfg->get_http_version());

  // HTTP Client for NBI
  http_client_inst_nbi = std::make_shared<oai::http::http_client>(
      Logger::sepp_client(), oai::common::sbi::kNfDefaultHttpRequestTimeout,
      sepp_cfg->local().get_nbi().get_if_name(), sepp_cfg->get_http_version());

  // Event subsystem
  sepp_event ev;

  // SEPP application layer
  sepp_app_inst = std::make_unique<sepp_app>(ev);

  // Task Manager
  tm_inst = std::make_unique<task_manager>(ev);
  std::thread task_manager_thread(&task_manager::run, tm_inst.get());

  std::string v4_address =
      oai::utils::conv::toString(sepp_cfg->local().get_sbi().get_addr4());

  std::string v4_address_nbi =
      oai::utils::conv::toString(sepp_cfg->local().get_nbi().get_addr4());

  if (sepp_cfg->get_http_version() == 1) {
    // SEPP Pistache API server (HTTP1)
    Pistache::Address addr(
        std::string(inet_ntoa(
            *((struct in_addr *)&sepp_cfg->local().get_sbi().get_addr4()))),
        Pistache::Port(sepp_cfg->local().get_sbi().get_port()));

    sepp_api_server_1 = std::make_unique<SEPPApiServer>(addr, sepp_app_inst);
    sepp_api_server_1->init(2);
    std::thread sepp_http1_manager(&SEPPApiServer::start,
                                   sepp_api_server_1.get());
    sepp_http1_manager.join();
  } else if (sepp_cfg->get_http_version() == 2) {

    // SEPP NGHTTP API server (HTTP2) for SBI
    sepp_sbi_server_2 = std::make_unique<sepp_http2_sbi_server>(
        v4_address, sepp_cfg->local().get_sbi().get_port(), sepp_app_inst);
    std::thread sepp_http2_manager(&sepp_http2_sbi_server::start,
                                   sepp_sbi_server_2.get());

    // SEPP NGHTTP API server (HTTP2) for NBI
    sepp_nbi_server_2 = std::make_unique<sepp_http2_nbi_server>(
        v4_address_nbi, sepp_cfg->local().get_nbi().get_port(), sepp_app_inst);
    std::thread sepp_http2_manager_nbi(&sepp_http2_nbi_server::start,
                                       sepp_nbi_server_2.get());

    sepp_http2_manager.join();
    sepp_http2_manager_nbi.join();
  }

  Logger::sepp_app().info("HTTP servers successfully stopped. Exiting");

  FILE *fp = NULL;
  std::string filename = fmt::format("/tmp/sepp_{}.status", getpid());
  fp = fopen(filename.c_str(), "w+");
  fprintf(fp, "STARTED\n");
  fflush(fp);
  fclose(fp);

  pause();
  return 0;
}
