/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef FILE_SEPP_HTTP2_NBI_SERVER_SEEN
#define FILE_SEPP_HTTP2_NBI_SERVER_SEEN

#include <nghttp2/asio_http2_server.h>

#include "N32FContextTerminateApi.h"
#include "conversions.hpp"
#include "sepp_app.hpp"
#include "string.hpp"
#include "uint_generator.hpp"

using namespace nghttp2::asio_http2;
using namespace nghttp2::asio_http2::server;
using namespace oai::_3gpp::model;

class sepp_http2_nbi_server {
public:
  sepp_http2_nbi_server(
      const std::string addr, uint32_t port,
      const std::unique_ptr<oai::sepp::app::sepp_app> &sepp_app_inst)
      : m_address(addr), m_port(port), server() {}
  void start();
  void init(size_t thr) {}

  void get_api_list(const response &response);
  void stop();

private:
  // Standard Response Helpers
  void send_problem_details(const response &resp, int status_code,
                            const std::string &title, const std::string &cause,
                            const std::string &detail);
  void respond_not_implemented(const response &response,
                               const std::string &detail);

  // Generic POST Payload Handler
  using AppHandlerFunc =
      std::function<bool(const nlohmann::json &, nlohmann::json &,
                         std::unordered_map<std::string, std::string> &)>;

  void process_post_request(const request &req, const response &resp,
                            const std::string &endpoint_name,
                            AppHandlerFunc app_handler,
                            const std::string &failure_cause,
                            const std::string &failure_detail,
                            bool allow_no_content_on_success = false);

  // Individual Endpoint Handlers
  void handle_exchange_capability(const request &req, const response &resp);
  void handle_exchange_params(const request &req, const response &resp);
  void handle_n32f_terminate(const request &req, const response &resp);
  void handle_n32f_error(const request &req, const response &resp);
  void handle_n32f_forward(const request &req, const response &resp);
  void handle_n32f_process(const request &req, const response &resp);

  oai::utils::uint_generator<uint32_t> m_promise_id_generator;
  std::string m_address;
  uint32_t m_port;
  http2 server;
  bool running_server;
};

#endif