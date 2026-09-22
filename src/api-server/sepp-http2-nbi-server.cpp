/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "sepp-http2-nbi-server.h"
#include "sepp_config.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/asio/ssl.hpp>
#include <nlohmann/json.hpp>

#include "3gpp_29.500.h"
#include "N32fContextInfo.h"
#include "N32fErrorDetail.h"
#include "N32fReformattedReqMsg.h"
#include "SecParamExchReqData.h"
#include "logger.hpp"
#include "sbi_helper.hpp"
#include "sepp_app.hpp"

using namespace nghttp2::asio_http2;
using namespace nghttp2::asio_http2::server;
using namespace oai::_3gpp::model;
using namespace oai::sepp::api;

extern std::unique_ptr<oai::config::sepp::sepp_config> sepp_cfg;
extern std::unique_ptr<oai::sepp::app::sepp_app> sepp_app_inst;

//------------------------------------------------------------------------------
namespace {
static const header_map JSON_HEADERS{
    {"content-type", header_value{"application/json"}}};
static const header_map PROBLEM_HEADERS{
    {"content-type", header_value{"application/problem+json"}}};
} // namespace

//------------------------------------------------------------------------------
void sepp_http2_nbi_server::start() {
  boost::system::error_code ec;

  server.handle("/n32c-handshake/v1/exchange-capability",
                [this](const request &req, const response &resp) {
                  handle_exchange_capability(req, resp);
                });

  server.handle("/n32c-handshake/v1/exchange-params",
                [this](const request &req, const response &resp) {
                  handle_exchange_params(req, resp);
                });

  server.handle("/n32c-handshake/v1/n32f-terminate",
                [this](const request &req, const response &resp) {
                  handle_n32f_terminate(req, resp);
                });

  server.handle("/n32c-handshake/v1/n32f-error",
                [this](const request &req, const response &resp) {
                  handle_n32f_error(req, resp);
                });

  server.handle("/", [this](const request &req, const response &resp) {
    handle_n32f_forward(req, resp);
  });

  server.handle("/n32f-forward/v1/n32f-process",
                [this](const request &req, const response &resp) {
                  handle_n32f_process(req, resp);
                });

  running_server = true;

  if (sepp_cfg->is_tls_enabled()) {
    Logger::sepp_nbi().info(
        "NBI TLS enabled (disable_tls: no). Starting NBI HTTP2 TLS server on %s:%u...",
        m_address.c_str(), m_port);

    try {
      boost::asio::ssl::context tls_ctx(boost::asio::ssl::context::sslv23);
      tls_ctx.set_options(boost::asio::ssl::context::default_workarounds |
                          boost::asio::ssl::context::no_sslv2 |
                          boost::asio::ssl::context::no_sslv3 |
                          boost::asio::ssl::context::single_dh_use);

      tls_ctx.use_certificate_chain_file(sepp_cfg->get_server_cert());
      tls_ctx.use_private_key_file(sepp_cfg->get_server_private_key(),
                                   boost::asio::ssl::context::pem);

      if (!sepp_cfg->get_client_cacert().empty()) {
        tls_ctx.load_verify_file(sepp_cfg->get_client_cacert());
      }

      configure_tls_context_easy(ec, tls_ctx);

      if (server.listen_and_serve(ec, tls_ctx, m_address, std::to_string(m_port))) {
        Logger::sepp_nbi().error("NBI TLS Server Listen Error: %s",
                                 ec.message().c_str());
      }
    } catch (const std::exception &e) {
      Logger::sepp_nbi().error("Failed to start NBI server with TLS: %s",
                               e.what());
    }
  } else {
    Logger::sepp_nbi().info(
        "NBI TLS disabled (disable_tls: yes). Starting plain NBI HTTP2 server on %s:%u...",
        m_address.c_str(), m_port);
    if (server.listen_and_serve(ec, m_address, std::to_string(m_port))) {
      Logger::sepp_nbi().error("NBI Server Listen Error: %s",
                               ec.message().c_str());
    }
  }

  running_server = false;
}

//------------------------------------------------------------------------------
void sepp_http2_nbi_server::handle_exchange_capability(const request &req,
                                                       const response &resp) {
  process_post_request(
      req, resp, "exchange-capability",
      [](const nlohmann::json &in, nlohmann::json &out,
         std::unordered_map<std::string, std::string> &headers) {
        return sepp_app_inst->handle_exchange_capability_request(in, out,
                                                                 headers);
      },
      "HANDSHAKE_FAILED", "Capability exchange failed");
}

//------------------------------------------------------------------------------
void sepp_http2_nbi_server::handle_exchange_params(const request &req,
                                                   const response &resp) {
  process_post_request(
      req, resp, "exchange-params",
      [](const nlohmann::json &in, nlohmann::json &out,
         std::unordered_map<std::string, std::string> &headers) {
        try {
          SecParamExchReqData param_req;
          from_json(in, param_req);
          std::stringstream err_ss;
          if (!param_req.validate(err_ss)) {
            Logger::sepp_nbi().warn(
                "SecParamExchReqData validation warning: %s",
                err_ss.str().c_str());
          }
        } catch (const std::exception &e) {
          Logger::sepp_nbi().warn("SecParamExchReqData parsing error: %s",
                                  e.what());
          return false;
        }

        return sepp_app_inst->handle_exchange_params(in, out, headers);
      },
      "PARAMETER_EXCHANGE_FAILED", "Parameter exchange failed");
}

//------------------------------------------------------------------------------
void sepp_http2_nbi_server::handle_n32f_terminate(const request &req,
                                                  const response &resp) {
  process_post_request(
      req, resp, "n32f-terminate",
      [](const nlohmann::json &in, nlohmann::json &out,
         std::unordered_map<std::string, std::string> &headers) {
        try {
          N32fContextInfo context;
          from_json(in, context);
          std::stringstream err_ss;
          if (!context.validate(err_ss)) {
            Logger::sepp_nbi().warn("N32fContextInfo validation failed: %s",
                                    err_ss.str().c_str());
            return false;
          }
        } catch (const std::exception &e) {
          Logger::sepp_nbi().warn("N32fContextInfo validation failed: %s",
                                  e.what());
          return false;
        }
        return sepp_app_inst->handle_n32f_terminate_req(in, out, headers);
      },
      "TERMINATION_FAILED", "N32-F termination processing failed");
}

//------------------------------------------------------------------------------
void sepp_http2_nbi_server::handle_n32f_error(const request &req,
                                              const response &resp) {
  process_post_request(
      req, resp, "n32f-error",
      [](const nlohmann::json &in, nlohmann::json &out,
         std::unordered_map<std::string, std::string> &headers) {
        return sepp_app_inst->handle_n32f_error(in, out, headers);
      },
      "ERROR_PROCESSING_FAILED", "N32-F error processing failed",
      /*allow_no_content_on_success=*/true);
}

//------------------------------------------------------------------------------
void sepp_http2_nbi_server::handle_n32f_forward(const request &req,
                                                const response &resp) {
  if (req.method() == "GET") {
    if (!sepp_app_inst) {
      send_problem_details(
          resp, oai::common::sbi::http_status_code::SERVICE_UNAVAILABLE,
          "503: Service Unavailable", "SYSTEM_UNAVAILABLE",
          "SEPP instance uninitialized");
      return;
    }

    try {
      std::string authority;
      auto auth_it = req.header().find("authority");
      if (auth_it != req.header().end()) {
        authority = auth_it->second.value;
      } else {
        auto host_it = req.header().find("host");
        if (host_it != req.header().end()) {
          authority = host_it->second.value;
        }
      }

      nlohmann::json req_headers = nlohmann::json::object();
      for (const auto &[k, v] : req.header()) {
        req_headers[k] = v.value;
      }

      nlohmann::json req_json = {{"path", req.uri().path},
                                 {"query", req.uri().raw_query},
                                 {"authority", authority},
                                 {"headers", req_headers}};

      nlohmann::json resp_json;
      std::unordered_map<std::string, std::string> resp_headers;

      if (sepp_app_inst->handle_n32f_forward(req_json, resp_json,
                                             resp_headers)) {
        header_map headers = JSON_HEADERS;
        for (const auto &[k, v] : resp_headers) {
          headers.emplace(k, header_value{v});
        }
        resp.write_head(oai::common::sbi::http_status_code::OK, headers);
        resp.end(resp_json.dump());
      } else {
        send_problem_details(resp,
                             oai::common::sbi::http_status_code::BAD_REQUEST,
                             "400: Bad Request", "FORWARDING_FAILED",
                             "N32-F GET forwarding failed");
      }
    } catch (const std::exception &e) {
      send_problem_details(resp,
                           oai::common::sbi::http_status_code::BAD_REQUEST,
                           "400: Bad Request", "MALFORMED_REQUEST", e.what());
    }
  } else {
    process_post_request(
        req, resp, "n32f-forward",
        [](const nlohmann::json &in, nlohmann::json &out,
           std::unordered_map<std::string, std::string> &headers) {
          return sepp_app_inst->handle_n32f_forward(in, out, headers);
        },
        "FORWARDING_FAILED", "N32-F message forwarding failed");
  }
}

//------------------------------------------------------------------------------
void sepp_http2_nbi_server::handle_n32f_process(const request &req,
                                                const response &resp) {
  process_post_request(
      req, resp, "n32f-process",
      [](const nlohmann::json &in, nlohmann::json &out,
         std::unordered_map<std::string, std::string> &headers) {
        try {
          N32fReformattedReqMsg req_msg;
          from_json(in, req_msg);
          std::stringstream err_ss;
          if (!req_msg.validate(err_ss)) {
            Logger::sepp_nbi().warn(
                "N32fReformattedReqMsg validation failed: %s",
                err_ss.str().c_str());
            return false;
          }
        } catch (const std::exception &e) {
          Logger::sepp_nbi().warn("N32fReformattedReqMsg parsing exception: %s",
                                  e.what());
          return false;
        }
        return sepp_app_inst->handle_n32f_process(in, out, headers);
      },
      "N32F_PROCESSING_FAILED", "N32-F message processing failed");
}

//------------------------------------------------------------------------------
void sepp_http2_nbi_server::send_problem_details(const response &resp,
                                                 int status_code,
                                                 const std::string &title,
                                                 const std::string &cause,
                                                 const std::string &detail) {
  nlohmann::json problemDetails = {{"status", status_code},
                                   {"title", title},
                                   {"cause", cause},
                                   {"detail", detail}};
  resp.write_head(status_code, PROBLEM_HEADERS);
  resp.end(problemDetails.dump());
}

//------------------------------------------------------------------------------
void sepp_http2_nbi_server::process_post_request(
    const request &req, const response &resp,
    const std::string & /*endpoint_name*/, AppHandlerFunc app_handler,
    const std::string &failure_cause, const std::string &failure_detail,
    bool allow_no_content_on_success) {

  if (req.method() != "POST") {
    send_problem_details(resp,
                         oai::common::sbi::http_status_code::METHOD_NOT_ALLOWED,
                         "Method Not Allowed", "ONLY_POST_SUPPORTED",
                         "Endpoint supports HTTP POST only");
    return;
  }

  auto body = std::make_shared<std::string>();
  body->reserve(2048);

  req.on_data([this, &resp, body, app_handler = std::move(app_handler),
               failure_cause, failure_detail, allow_no_content_on_success](
                  const uint8_t *data, std::size_t len) {
    if (data && len > 0) {
      body->append(reinterpret_cast<const char *>(data), len);
    }

    if (len == 0) {
      if (!sepp_app_inst) {
        send_problem_details(
            resp, oai::common::sbi::http_status_code::SERVICE_UNAVAILABLE,
            "503: Service Unavailable", "SYSTEM_UNAVAILABLE",
            "SEPP instance uninitialized");
        return;
      }

      try {
        nlohmann::json req_json = nlohmann::json::parse(*body);
        nlohmann::json resp_json;
        std::unordered_map<std::string, std::string> resp_headers;

        if (app_handler(req_json, resp_json, resp_headers)) {
          if (allow_no_content_on_success && resp_json.empty()) {
            resp.write_head(oai::common::sbi::http_status_code::NO_CONTENT,
                            JSON_HEADERS);
            resp.end("");
          } else {
            header_map headers = JSON_HEADERS;
            for (const auto &[k, v] : resp_headers) {
              headers.emplace(k, header_value{v});
            }
            resp.write_head(oai::common::sbi::http_status_code::OK, headers);
            resp.end(resp_json.dump());
          }
        } else {
          send_problem_details(
              resp, oai::common::sbi::http_status_code::BAD_REQUEST,
              "400: Bad Request", failure_cause, failure_detail);
        }
      } catch (const std::exception &e) {
        send_problem_details(resp,
                             oai::common::sbi::http_status_code::BAD_REQUEST,
                             "400: Bad Request", "MALFORMED_JSON", e.what());
      }
    }
  });
}

//------------------------------------------------------------------------------
void sepp_http2_nbi_server::stop() {
  server.stop();
  while (running_server) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}