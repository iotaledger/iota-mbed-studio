/*
 * Copyright (c) 2020 Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 */

#if MBED_TEST_MODE

#include "greentea-client/test_env.h"
#include "mbed.h"
#include "unity/unity.h"
#include "utest/utest.h"

#include "blake2b_data.h"
#include "core/address.h"
#include "core/utils/byte_buffer.h"
#include "crypto/iota_crypto.h"
#include "httpClient.h"
#include "main_config.h"

using namespace utest::v1;

static control_t test_http_client(const size_t call_count) {
  WiFiInterface *wifi = WiFiInterface::get_default_instance();
  TEST_ASSERT_NOT_NULL(wifi);
  nsapi_error_t net_err = wifi->connect(WIFI_SSID, WIFI_PWD, WIFI_SECURITY);
  TEST_ASSERT(net_err == NSAPI_ERROR_OK);

  httpClient client;
  int ret = client.get("/api/v1/info");
  TEST_ASSERT(ret > 0);
  printf("%d, %s\n", ret, client.response_data().c_str());
  wifi->disconnect();
  return CaseNext;
}

// BLAKE2 hash function
// test vectors: https://github.com/BLAKE2/BLAKE2/tree/master/testvectors
static control_t test_blake2b_hash(const size_t call_count) {
  uint8_t msg[256] = {};
  uint8_t out_512[64] = {};
  uint8_t out_256[32] = {};

  for (size_t i = 0; i < sizeof(msg); i++) {
    msg[i] = i;
  }

  for (size_t i = 0; i < sizeof(msg); i++) {
    iota_blake2b_sum(msg, i, out_256, sizeof(out_256));
    iota_blake2b_sum(msg, i, out_512, sizeof(out_512));
    TEST_ASSERT_EQUAL_MEMORY(blake2b_256[i], out_256, sizeof(out_256));
    TEST_ASSERT_EQUAL_MEMORY(blake2b_512[i], out_512, sizeof(out_512));
  }
  return CaseNext;
}

// HMAC-SHA-256 and HMAC-SHA-512
// test vectors: https://tools.ietf.org/html/rfc4231#section-4.2
static control_t test_hmacsha(const size_t call_count) {
  uint8_t tmp_sha256_out[32] = {};
  uint8_t tmp_sha512_out[64] = {};
  // Test case 1
  uint8_t key_1[32];
  // 20-bytes 0x0b
  memset(key_1, 0x0b, 20);
  memset(key_1 + 20, 0x00, 32 - 20);
  // "Hi There"
  uint8_t data_1[] = {0x48, 0x69, 0x20, 0x54, 0x68, 0x65, 0x72, 0x65};
  uint8_t exp_sha256_out1[] = {0xb0, 0x34, 0x4c, 0x61, 0xd8, 0xdb, 0x38, 0x53,
                               0x5c, 0xa8, 0xaf, 0xce, 0xaf, 0x0b, 0xf1, 0x2b,
                               0x88, 0x1d, 0xc2, 0x00, 0xc9, 0x83, 0x3d, 0xa7,
                               0x26, 0xe9, 0x37, 0x6c, 0x2e, 0x32, 0xcf, 0xf7};
  uint8_t exp_sha512_out1[] = {
      0x87, 0xaa, 0x7c, 0xde, 0xa5, 0xef, 0x61, 0x9d, 0x4f, 0xf0, 0xb4,
      0x24, 0x1a, 0x1d, 0x6c, 0xb0, 0x23, 0x79, 0xf4, 0xe2, 0xce, 0x4e,
      0xc2, 0x78, 0x7a, 0xd0, 0xb3, 0x05, 0x45, 0xe1, 0x7c, 0xde, 0xda,
      0xa8, 0x33, 0xb7, 0xd6, 0xb8, 0xa7, 0x02, 0x03, 0x8b, 0x27, 0x4e,
      0xae, 0xa3, 0xf4, 0xe4, 0xbe, 0x9d, 0x91, 0x4e, 0xeb, 0x61, 0xf1,
      0x70, 0x2e, 0x69, 0x6c, 0x20, 0x3a, 0x12, 0x68, 0x54};

  TEST_ASSERT(iota_crypto_hmacsha256(key_1, data_1, sizeof(data_1),
                                     tmp_sha256_out) == 0);
  TEST_ASSERT_EQUAL_MEMORY(exp_sha256_out1, tmp_sha256_out,
                           sizeof(exp_sha256_out1));

  TEST_ASSERT(iota_crypto_hmacsha512(key_1, data_1, sizeof(data_1),
                                     tmp_sha512_out) == 0);
  TEST_ASSERT_EQUAL_MEMORY(exp_sha512_out1, tmp_sha512_out,
                           sizeof(exp_sha512_out1));

  // Test case 2
  // "Jefe"
  uint8_t key_2[32] = {0x4a, 0x65, 0x66, 0x65};
  // "what do ya want for nothing?""
  uint8_t data_2[] = {0x77, 0x68, 0x61, 0x74, 0x20, 0x64, 0x6f,
                      0x20, 0x79, 0x61, 0x20, 0x77, 0x61, 0x6e,
                      0x74, 0x20, 0x66, 0x6f, 0x72, 0x20, 0x6e,
                      0x6f, 0x74, 0x68, 0x69, 0x6e, 0x67, 0x3f};
  uint8_t exp_sha256_out2[] = {0x5b, 0xdc, 0xc1, 0x46, 0xbf, 0x60, 0x75, 0x4e,
                               0x6a, 0x04, 0x24, 0x26, 0x08, 0x95, 0x75, 0xc7};
  uint8_t exp_sha512_out2[] = {
      0x16, 0x4b, 0x7a, 0x7b, 0xfc, 0xf8, 0x19, 0xe2, 0xe3, 0x95, 0xfb,
      0xe7, 0x3b, 0x56, 0xe0, 0xa3, 0x87, 0xbd, 0x64, 0x22, 0x2e, 0x83,
      0x1f, 0xd6, 0x10, 0x27, 0x0c, 0xd7, 0xea, 0x25, 0x05, 0x54, 0x97,
      0x58, 0xbf, 0x75, 0xc0, 0x5a, 0x99, 0x4a, 0x6d, 0x03, 0x4f, 0x65,
      0xf8, 0xf0, 0xe6, 0xfd, 0xca, 0xea, 0xb1, 0xa3, 0x4d, 0x4a, 0x6b,
      0x4b, 0x63, 0x6e, 0x07, 0x0a, 0x38, 0xbc, 0xe7, 0x37};

  TEST_ASSERT(iota_crypto_hmacsha256(key_2, data_2, sizeof(data_2),
                                     tmp_sha256_out) == 0);
  TEST_ASSERT_EQUAL_MEMORY(exp_sha256_out2, tmp_sha256_out,
                           sizeof(exp_sha256_out2));

  TEST_ASSERT(iota_crypto_hmacsha512(key_2, data_2, sizeof(data_2),
                                     tmp_sha512_out) == 0);
  TEST_ASSERT_EQUAL_MEMORY(exp_sha512_out2, tmp_sha512_out,
                           sizeof(exp_sha512_out2));
  return CaseNext;
}

static control_t test_addr_gen(const size_t call_count) {
  char const *const exp_iot_bech32 =
      "iot1qpg4tqh7vj9s7y9zk2smj8t4qgvse9um42l7apdkhw6syp5ju4w3v6ffg6n";
  char const *const exp_iota_bech32 =
      "iota1qpg4tqh7vj9s7y9zk2smj8t4qgvse9um42l7apdkhw6syp5ju4w3v79tf3l";
  byte_t exp_addr[IOTA_ADDRESS_BYTES] = {
      0x00, 0x51, 0x55, 0x82, 0xfe, 0x64, 0x8b, 0xf,  0x10, 0xa2, 0xb2,
      0xa1, 0xb9, 0x1d, 0x75, 0x2,  0x19, 0xc,  0x97, 0x9b, 0xaa, 0xbf,
      0xee, 0x85, 0xb6, 0xbb, 0xb5, 0x2,  0x6,  0x92, 0xe5, 0x5d, 0x16};
  byte_t exp_ed_addr[ED25519_ADDRESS_BYTES] = {
      0x4d, 0xbc, 0x7b, 0x45, 0x32, 0x46, 0x64, 0x20, 0x9a, 0xe5, 0x59,
      0xcf, 0xd1, 0x73, 0xc,  0xb,  0xb1, 0x90, 0x5a, 0x7f, 0x83, 0xe6,
      0x5d, 0x48, 0x37, 0xa9, 0x87, 0xe2, 0x24, 0xc1, 0xc5, 0x1e};

  byte_t seed[IOTA_SEED_BYTES] = {};
  byte_t addr_from_path[ED25519_ADDRESS_BYTES] = {};
  char bech32_addr[128] = {};
  byte_t addr_with_ver[IOTA_ADDRESS_BYTES] = {};
  byte_t addr_from_bech32[IOTA_ADDRESS_BYTES] = {};

  // convert seed from hex string to binary
  TEST_ASSERT(
      hex2bin(
          "e57fb750f3a3a67969ece5bd9ae7eef5b2256a818b2aac458941f7274985a410",
          IOTA_SEED_BYTES * 2, seed, IOTA_SEED_BYTES) == 0);

  TEST_ASSERT(address_from_path(seed, "m/44'/4218'/0'/0'/0'", addr_from_path) ==
              0);
  // dump_hex(addr_from_path, ED25519_ADDRESS_BYTES);

  // ed25519 address to IOTA address
  addr_with_ver[0] = ADDRESS_VER_ED25519;
  memcpy(addr_with_ver + 1, addr_from_path, ED25519_ADDRESS_BYTES);
  TEST_ASSERT_EQUAL_MEMORY(exp_addr, addr_with_ver, IOTA_ADDRESS_BYTES);
  // dump_hex(addr_with_ver, IOTA_ADDRESS_BYTES);

  // convert binary address to bech32 with iot HRP
  TEST_ASSERT(address_2_bech32(addr_with_ver, "iot", bech32_addr) == 0);
  TEST_ASSERT_EQUAL_STRING(exp_iot_bech32, bech32_addr);
  printf("bech32 [iot]: %s\n", bech32_addr);
  // bech32 to binary address
  TEST_ASSERT(address_from_bech32("iot", bech32_addr, addr_from_bech32) == 0);
  TEST_ASSERT_EQUAL_MEMORY(addr_with_ver, addr_from_bech32, IOTA_ADDRESS_BYTES);

  // convert binary address to bech32 with iota HRP
  TEST_ASSERT(address_2_bech32(addr_with_ver, "iota", bech32_addr) == 0);
  TEST_ASSERT_EQUAL_STRING(exp_iota_bech32, bech32_addr);
  printf("bech32 [iota]: %s\n", bech32_addr);
  // bech32 to binary address
  TEST_ASSERT(address_from_bech32("iota", bech32_addr, addr_from_bech32) == 0);
  TEST_ASSERT_EQUAL_MEMORY(addr_with_ver, addr_from_bech32, IOTA_ADDRESS_BYTES);

  // address from ed25519 keypair
  iota_keypair_t seed_keypair = {};
  byte_t ed_addr[ED25519_ADDRESS_BYTES] = {};

  // address from ed25519 public key
  iota_crypto_keypair(seed, &seed_keypair);
  TEST_ASSERT(address_from_ed25519_pub(seed_keypair.pub, ed_addr) == 0);
  TEST_ASSERT_EQUAL_MEMORY(exp_ed_addr, ed_addr, ED25519_ADDRESS_BYTES);

  return CaseNext;
}

utest::v1::status_t greentea_setup(const size_t number_of_cases) {
  // Here, we specify the timeout (60s) and the host test (a built-in host test
  // or the name of our Python file)
  GREENTEA_SETUP(60, "default_auto");

  return greentea_test_setup_handler(number_of_cases);
}

// List of test cases in this file
Case cases[] = {
    Case("HTTP Client", test_http_client), Case("IOTA Address", test_addr_gen),
    Case("BLAKE2", test_blake2b_hash), Case("HMAC SHA", test_hmacsha)};

Specification specification(greentea_setup, cases);

int main() { return !Harness::run(specification); }

#endif