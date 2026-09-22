/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "sepp_JOSE.hpp"
#include "logger.hpp"

#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>

namespace oai::sepp::app {

//------------------------------------------------------------------------------
// Helper: Base64URL Encoding
//------------------------------------------------------------------------------
std::string jose::base64url_encode(const unsigned char *input, size_t length) {
  BIO *bio, *b64;
  BUF_MEM *bufferPtr;

  b64 = BIO_new(BIO_f_base64());
  BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
  bio = BIO_new(BIO_s_mem());
  bio = BIO_push(b64, bio);

  BIO_write(bio, input, static_cast<int>(length));
  BIO_flush(bio);
  BIO_get_mem_ptr(bio, &bufferPtr);

  std::string ret(bufferPtr->data, bufferPtr->length);
  BIO_free_all(bio);

  std::string b64url;
  for (char c : ret) {
    if (c == '+')
      b64url += '-';
    else if (c == '/')
      b64url += '_';
    else if (c != '=')
      b64url += c;
  }
  return b64url;
}

//------------------------------------------------------------------------------
std::string jose::base64url_encode(const std::string &input) {
  return base64url_encode(reinterpret_cast<const unsigned char *>(input.data()),
                          input.size());
}

//------------------------------------------------------------------------------
// Helper: Base64URL Decoding
//------------------------------------------------------------------------------
std::string jose::base64url_decode(const std::string &input) {
  std::string b64 = input;
  for (char &c : b64) {
    if (c == '-')
      c = '+';
    else if (c == '_')
      c = '/';
  }
  while (b64.size() % 4 != 0) {
    b64.push_back('=');
  }

  BIO *bio, *b64_dec;
  std::vector<char> buffer(b64.size());

  b64_dec = BIO_new(BIO_f_base64());
  BIO_set_flags(b64_dec, BIO_FLAGS_BASE64_NO_NL);
  bio = BIO_new_mem_buf(b64.data(), static_cast<int>(b64.size()));
  bio = BIO_push(b64_dec, bio);

  int decoded_size =
      BIO_read(bio, buffer.data(), static_cast<int>(buffer.size()));
  BIO_free_all(bio);

  if (decoded_size < 0)
    return "";
  return std::string(buffer.data(), decoded_size);
}

//------------------------------------------------------------------------------
// Apply AES-128-GCM JWE Encryption
//------------------------------------------------------------------------------
bool jose::apply_jwe_encryption(const std::string &plaintext,
                                const std::string &aad_b64,
                                const std::vector<uint8_t> &key,
                                std::string &out_iv_b64,
                                std::string &out_ciphertext_b64,
                                std::string &out_tag_b64) {
  if (key.size() != 16) {
    Logger::sepp_app().error(
        "Invalid key size for AES-128-GCM JWE (requires 16 bytes)");
    return false;
  }

  unsigned char iv[12];
  if (RAND_bytes(iv, sizeof(iv)) != 1) {
    Logger::sepp_app().error("Failed to generate random IV for JWE");
    return false;
  }

  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
  if (!ctx)
    return false;

  if (EVP_EncryptInit_ex(ctx, EVP_aes_128_gcm(), nullptr, nullptr, nullptr) !=
          1 ||
      EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, sizeof(iv), nullptr) !=
          1 ||
      EVP_EncryptInit_ex(ctx, nullptr, nullptr, key.data(), iv) != 1) {
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }

  int len = 0;
  EVP_EncryptUpdate(ctx, nullptr, &len,
                    reinterpret_cast<const unsigned char *>(aad_b64.data()),
                    aad_b64.size());

  std::vector<unsigned char> ciphertext(plaintext.size() + 16);
  if (EVP_EncryptUpdate(
          ctx, ciphertext.data(), &len,
          reinterpret_cast<const unsigned char *>(plaintext.data()),
          plaintext.size()) != 1) {
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }
  int ciphertext_len = len;

  if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }
  ciphertext_len += len;

  unsigned char tag[16];
  if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, sizeof(tag), tag) != 1) {
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }
  EVP_CIPHER_CTX_free(ctx);

  out_iv_b64 = base64url_encode(iv, sizeof(iv));
  out_ciphertext_b64 = base64url_encode(ciphertext.data(), ciphertext_len);
  out_tag_b64 = base64url_encode(tag, sizeof(tag));

  return true;
}

//------------------------------------------------------------------------------
// Decrypt AES-128-GCM JWE Ciphertext
//------------------------------------------------------------------------------
bool jose::decrypt_jwe_payload(const std::string &ciphertext_b64,
                               const std::string &aad_b64,
                               const std::string &iv_b64,
                               const std::string &tag_b64,
                               const std::vector<uint8_t> &key,
                               std::string &out_plaintext) {

  std::string ciphertext_raw = base64url_decode(ciphertext_b64);
  std::string iv_raw = base64url_decode(iv_b64);
  std::string tag_raw = base64url_decode(tag_b64);

  if (key.size() != 16 || iv_raw.size() != 12 || tag_raw.size() != 16) {
    return false;
  }

  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
  if (!ctx)
    return false;

  if (EVP_DecryptInit_ex(ctx, EVP_aes_128_gcm(), nullptr, nullptr, nullptr) !=
          1 ||
      EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN,
                          static_cast<int>(iv_raw.size()), nullptr) != 1 ||
      EVP_DecryptInit_ex(
          ctx, nullptr, nullptr, key.data(),
          reinterpret_cast<const unsigned char *>(iv_raw.data())) != 1) {
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }

  int len = 0;
  EVP_DecryptUpdate(ctx, nullptr, &len,
                    reinterpret_cast<const unsigned char *>(aad_b64.data()),
                    aad_b64.size());

  std::vector<unsigned char> plaintext(ciphertext_raw.size());
  if (EVP_DecryptUpdate(
          ctx, plaintext.data(), &len,
          reinterpret_cast<const unsigned char *>(ciphertext_raw.data()),
          ciphertext_raw.size()) != 1) {
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }
  int plaintext_len = len;

  if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG,
                          static_cast<int>(tag_raw.size()),
                          const_cast<char *>(tag_raw.data())) != 1) {
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }

  if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) <= 0) {
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }
  plaintext_len += len;

  EVP_CIPHER_CTX_free(ctx);
  out_plaintext.assign(reinterpret_cast<char *>(plaintext.data()),
                       plaintext_len);
  return true;
}

} // namespace oai::sepp::app