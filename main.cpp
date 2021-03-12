// Copyright 2021 IOTA Stiftung
// SPDX-License-Identifier: Apache-2.0

/**
 * @author Sam Chen
 * @brief IOTA Client example with Mbed Studio
 *
 */

#if !MBED_TEST_MODE

#include "mbed-trace/mbed_trace.h"
#include "mbed.h"
#include <chrono>

#include "NTPClient.h"
#include "clientpp/iotaAPI.h"
#include "jsonUtils.h"
#include "main_config.h"
#include "sensorService.h"

// #define _TEST_IOTA_MSG_

void test_json() {
  char const *const json_tips =
      "{\"index\":\"Foo\",\"maxResults\":1000,\"isBool\":true,\"arr\":["
      "\"this\",\"is\",\"a\",\"string\",\"array\"]}";
  jsonUtils json;
  string data;
  bool b;
  vector<string> arr;

  cJSON *obj = cJSON_Parse(json_tips);
  printf("%s\n", cJSON_PrintUnformatted(obj));
  json.getString(obj, "index", data);
  json.getBool(obj, "isBool", &b);
  json.getArrayString(obj, "arr", arr);
  printf("index = %s, bool = %d\n", data.c_str(), b);
  for (auto i : arr) {
    printf("%s ", i.c_str());
  }
  printf("\n");

  cJSON_Delete(obj);
}

#ifdef _TEST_IOTA_MSG_
#include "core/models/message.h"
void test_iota_message() {
  byte_t tx_id0[TRANSACTION_ID_BYTES] = {};
  byte_t addr0[ED25519_ADDRESS_BYTES] = {
      0x51, 0x55, 0x82, 0xfe, 0x64, 0x8b, 0x0f, 0x10, 0xa2, 0xb2, 0xa1,
      0xb9, 0x1d, 0x75, 0x02, 0x19, 0x0c, 0x97, 0x9b, 0xaa, 0xbf, 0xee,
      0x85, 0xb6, 0xbb, 0xb5, 0x02, 0x06, 0x92, 0xe5, 0x5d, 0x16};
  byte_t addr1[ED25519_ADDRESS_BYTES] = {
      0x69, 0x20, 0xb1, 0x76, 0xf6, 0x13, 0xec, 0x7b, 0xe5, 0x9e, 0x68,
      0xfc, 0x68, 0xf5, 0x97, 0xeb, 0x33, 0x93, 0xaf, 0x80, 0xf7, 0x4c,
      0x7c, 0x3d, 0xb7, 0x81, 0x98, 0x14, 0x7d, 0x5f, 0x1f, 0x92};

  iota_keypair_t seed_keypair = {};
  hex2bin("f7868ab6bb55800b77b8b74191ad8285a9bf428ace579d541fda47661803ff44",
          64, seed_keypair.pub, ED_PUBLIC_KEY_BYTES);
  hex2bin("256a818b2aac458941f7274985a410e57fb750f3a3a67969ece5bd9ae7eef5b2f786"
          "8ab6bb55800b77b8b74191ad8285a9bf428ace579d541fda47661803ff44",
          128, seed_keypair.priv, ED_PRIVATE_KEY_BYTES);

  core_message_t *msg = core_message_new();

  transaction_payload_t *tx = tx_payload_new();

  tx_payload_add_input_with_key(tx, tx_id0, 0, seed_keypair.pub,
                                seed_keypair.priv);
  tx_payload_add_output(tx, OUTPUT_SINGLE_OUTPUT, addr0, 1000);
  tx_payload_add_output(tx, OUTPUT_SINGLE_OUTPUT, addr1, 2779530283276761);

  // put tx payload into message
  msg->payload_type = 0;
  msg->payload = tx;

  // core_message_sign_transaction(msg);

  tx_payload_print(tx);

  // free message and sub entities
  core_message_free(msg);
}
#endif

void taggle_led(DigitalOut led) { led.write(!led.read()); }

// main() runs in its own thread in the OS
int main() {
  printf("IOTA example on B-L4S5I-IOT01A\n");

#ifdef MBED_MAJOR_VERSION
  printf("Mbed OS version %d.%d.%d\n\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION,
         MBED_PATCH_VERSION);
#endif

#ifdef MBED_CONF_MBED_TRACE_ENABLE
  mbed_trace_init();
#endif

  // connect to wifi AP
  WiFiInterface *wifi = WiFiInterface::get_default_instance();
  if (!wifi) {
    printf("cannot get wifi interface\n");
    return -1;
  }
  nsapi_error_t net_err = wifi->connect(WIFI_SSID, WIFI_PWD, WIFI_SECURITY);
  if (net_err != NSAPI_ERROR_OK) {
    printf("connecting to wifi failed\n");
    return -1;
  }

  // display IP address
  // SocketAddress a;
  // wifi->get_ip_address(&a);
  // printf("IP: %s\n", a.get_ip_address());

  // ntp
  NTPClient ntp(wifi);
  ntp.set_server("pool.ntp.org", 123);
  time_t timestamp = ntp.get_timestamp();
  if (timestamp < 0) {
    printf("An error occurred when getting the time. Code: %zu\n", timestamp);
  } else {
    set_time(timestamp);
    time_t sysTime = time(NULL);
    // printf("NTP [%s], Sys [%s]\n", ctime(&timestamp), ctime(&sysTime));
  }

  // internal I2C
  // NFC, 3-axix, 3D gyroscope, Barometer, Humi and Temp, STSAFE-A110
  I2C i2c(PB_11, PB_10); // internal I2C

  sensorService sensor;
  if (sensor.init(&i2c, "B-L4S5I-IOT01A") != 0) {
    printf("init sensor service failed\n");
    return -1;
  }

#ifdef _TEST_IOTA_MSG_
  test_iota_message();
#endif
  iotaAPI iota;
  string msg_id;

  // init onboard LED2
  DigitalOut led2(LED2);

  while (true) {
    taggle_led(led2);
    printf("%s\n", sensor.toJSON().c_str());
    iota.sendIndexation("iota_sensor", sensor.toJSON(), msg_id);
    ThisThread::sleep_for(chrono::milliseconds(SENSOR_DATA_INTERVAL));
  }
}

#endif
