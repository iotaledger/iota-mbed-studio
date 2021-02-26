#include "mbed.h"

#include "blake2.h"
#include "cJSON.h"
#include "utarray.h"
#include <chrono>

#include "HTS221.h"

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
  // internal I2C
  // NFC, 3-axix, 3D gyroscope, Barometer, Humi and Temp, STSAFE-A110
  I2C i2c(PB_11, PB_10); // internal I2C

  HTS221 temp;
  float temp_c = 0.0;
  float humi = 0.0;
  // init hts221 with internal I2C bus
  temp.init(&i2c);
  // init onboard LED2
  DigitalOut led2(LED2);

  while (true) {
    taggle_led(led2);
    ThisThread::sleep_for(chrono::milliseconds(3000));
    temp.getTempture(&temp_c);
    temp.getHumidity(&humi);
    printf("temp: %.2f, humi: %.2f\n", temp_c, humi);
  }
}
