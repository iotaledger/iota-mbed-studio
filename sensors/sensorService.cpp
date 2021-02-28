// Copyright 2021 IOTA Stiftung
// SPDX-License-Identifier: Apache-2.0

/**
 * @author Sam Chen
 * @brief sensor service for getting sensor data
 */

#include "sensorService.h"

int sensorService::init(const mbed::I2C *i2c, const std::string &id) {
  _ID = id;
  return _hts221.init(i2c);
}

float sensorService::temperature() {
  float temp = 0.0;
  _hts221.getTemperature(&temp);
  return temp;
}

float sensorService::humidity() {
  float humi = 0.0;
  _hts221.getHumidity(&humi);
  return humi;
}

int sensorService::toJSON(std::string &json_str) {
  /*
  {
    "ID": "IOTA-Sensor-001",
    "time": 1614500719,
    "temp": 19.04,
    "humi": 30.1
  }
  */
  return 0;
}