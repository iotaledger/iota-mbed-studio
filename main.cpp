// Copyright 2021 IOTA Stiftung
// SPDX-License-Identifier: Apache-2.0

/**
 * @author Sam Chen
 * @brief IOTA Client example with Mbed Studio
 *
 */
#include "mbed.h"

#include "blake2.h"
#include "cJSON.h"
#include "utarray.h"
#include <chrono>

#include "NTPClient.h"

#include "sensorService.h"

void test_cjson() {
  // test cjson
  cJSON *monitor = cJSON_CreateObject();
  if (monitor == NULL) {
    printf("cjson create object failed\n");
  } else {
    cJSON_free(monitor);
    printf("create and free cjson object done\n");
  }
}

void test_utarray() {
  // test utarray
  UT_array *nums;
  int i, *p;
  utarray_new(nums, &ut_int_icd);
  for (i = 0; i < 10; i++)
    utarray_push_back(nums, &i);

  for (p = (int *)utarray_front(nums); p != NULL;
       p = (int *)utarray_next(nums, p)) {
    printf("%d", *p);
  }
  printf("\n");
  utarray_free(nums);
}

void test_blake2b() {
  // test blake2b
  int ret = 0;
  char buf[100] = {};

  char blake2b_sum[32] = {};
  ret = blake2b(blake2b_sum, sizeof(blake2b_sum), buf, sizeof(buf), NULL, 0);
  printf("blake2 : %d\n", ret);
  for (size_t i = 0; i < sizeof(blake2b_sum); i++) {
    printf("%02X ", buf[i]);
  }
  printf("\n");
}

void taggle_led(DigitalOut led) { led.write(!led.read()); }

// main() runs in its own thread in the OS
int main() {
  printf("IOTA example\n");

#ifdef MBED_MAJOR_VERSION
  printf("Mbed OS version %d.%d.%d\n\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION,
         MBED_PATCH_VERSION);
#endif
  // init and connect to network
  WiFiInterface *wifi = WiFiInterface::get_default_instance();
  if (!wifi) {
    printf("ERROR: No WiFiInterface found.\n");
    return -1;
  }

  printf("\nConnecting to %s...\n", MBED_CONF_APP_WIFI_SSID);
  int ret = wifi->connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD,
                          NSAPI_SECURITY_WPA_WPA2);
  if (ret != 0) {
    printf("\nConnection error: %d\n", ret);
    return -1;
  }
  SocketAddress a;
  wifi->get_ip_address(&a);
  printf("IP: %s\n", a.get_ip_address());

  // ntp
  NTPClient ntp(wifi);
  ntp.set_server("pool.ntp.org", 123);
  time_t timestamp = ntp.get_timestamp();
  if (timestamp < 0) {
    printf("An error occurred when getting the time. Code: %zu\n", timestamp);
  } else {
    set_time(timestamp);
    time_t sysTime = time(NULL);
    printf("NTP [%s], Sys [%s]\n", ctime(&timestamp), ctime(&sysTime));
  }

  // internal I2C
  // NFC, 3-axix, 3D gyroscope, Barometer, Humi and Temp, STSAFE-A110
  I2C i2c(PB_11, PB_10); // internal I2C

  sensorService sensor;
  if (sensor.init(&i2c, "sensor_1") != 0) {
    printf("init sensor service failed\n");
    return -1;
  }

  // init onboard LED2
  DigitalOut led2(LED2);

  while (true) {
    taggle_led(led2);
    ThisThread::sleep_for(chrono::milliseconds(3000));
    printf("temp: %.2f, humi: %.2f\n", sensor.temperature(), sensor.humidity());
  }

  //
  wifi->disconnect();
}
