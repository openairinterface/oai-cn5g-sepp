/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#pragma once

#include "logger_base.hpp"

static const std::string SEPP_APP = "sepp_app";
static const std::string SEPP_SBI = "sepp_sbi";
static const std::string SEPP_NBI = "sepp_nbi";
static const std::string SEPP_CLIENT = "sepp_client";
static const std::string SEPP_DB = "sepp_db";

class Logger {
public:
  static void init(const std::string &name, bool log_stdout,
                   bool log_rot_file) {
    oai::logger::logger_registry::register_logger(name, SEPP_APP, log_stdout,
                                                  log_rot_file);
    oai::logger::logger_registry::register_logger(name, SEPP_SBI, log_stdout,
                                                  log_rot_file);
    oai::logger::logger_registry::register_logger(name, SEPP_NBI, log_stdout,
                                                  log_rot_file);
    oai::logger::logger_registry::register_logger(name, SEPP_CLIENT, log_stdout,
                                                  log_rot_file);
    oai::logger::logger_registry::register_logger(name, LOGGER_COMMON,
                                                  log_stdout, log_rot_file);
    oai::logger::logger_registry::register_logger(name, SEPP_DB, log_stdout,
                                                  log_rot_file);
    oai::logger::logger_registry::register_logger(name, SYSTEM, log_stdout,
                                                  log_rot_file);
  }

  static bool should_log(spdlog::level::level_enum level) {
    return oai::logger::logger_registry::should_log(level);
  }

  static void set_level(spdlog::level::level_enum level) {
    oai::logger::logger_registry::set_level(level);
  }

  static void set_lttng(bool isLttngActive) {
    oai::logger::logger_registry::set_lttng_is_active(isLttngActive);
  }

  static const oai::logger::printf_logger &sepp_app() {
    return oai::logger::logger_registry::get_logger(SEPP_APP);
  }

  static const oai::logger::printf_logger &sepp_sbi() {
    return oai::logger::logger_registry::get_logger(SEPP_SBI);
  }

  static const oai::logger::printf_logger &sepp_nbi() {
    return oai::logger::logger_registry::get_logger(SEPP_NBI);
  }

  static const oai::logger::printf_logger &sepp_client() {
    return oai::logger::logger_registry::get_logger(SEPP_CLIENT);
  }

  static const oai::logger::printf_logger &system() {
    return oai::logger::logger_registry::get_logger(SYSTEM);
  }
};
