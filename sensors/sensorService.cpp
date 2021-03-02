// Copyright 2021 IOTA Stiftung
// SPDX-License-Identifier: Apache-2.0

/**
 * @author Sam Chen
 * @brief sensor service for getting sensor data
 */

#include <memory>
#include <string>

#include "sensorService.h"

// https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
template <typename... Args>
std::string string_format(const std::string &format, Args... args) {
  int size =
      snprintf(nullptr, 0, format.c_str(), args...) + 1; // Extra space for '\0'
  if (size <= 0) {
    return "";
  }
  std::unique_ptr<char[]> buf(new char[size]);
  snprintf(buf.get(), size, format.c_str(), args...);
  return std::string(buf.get(),
                     buf.get() + size - 1); // We don't want the '\0' inside
}

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

std::string sensorService::toJSON() {
  /*
  {
    "ID": "IOTA-Sensor-001",
    "time": 1614500719,
    "temp": 19.04,
    "humi": 30.1
  }
  */
  time_t timestampe = time(NULL);
  return string_format(
      "{\"ID\":\"%s\",\"time\":%zu,\"temp\":%.2f,\"humi\":%.2f}",
      _ID.c_str(), timestampe, temperature(), humidity());
}