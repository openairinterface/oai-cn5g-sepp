/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef FILE_JOSE_HPP_SEEN
#define FILE_JOSE_HPP_SEEN

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace oai::sepp::app {

class jose {
public:
  static std::string base64url_encode(const unsigned char *input,
                                      size_t length);
  static std::string base64url_encode(const std::string &input);
  static std::string base64url_decode(const std::string &input);

  static bool apply_jwe_encryption(const std::string &plaintext,
                                   const std::string &aad_b64,
                                   const std::vector<uint8_t> &key,
                                   std::string &out_iv_b64,
                                   std::string &out_ciphertext_b64,
                                   std::string &out_tag_b64);

  static bool decrypt_jwe_payload(const std::string &ciphertext_b64,
                                  const std::string &aad_b64,
                                  const std::string &iv_b64,
                                  const std::string &tag_b64,
                                  const std::vector<uint8_t> &key,
                                  std::string &out_plaintext);
};

} // namespace oai::sepp::app

#endif /* FILE_JOSE_HPP_SEEN */