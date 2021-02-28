// Copyright 2021 IOTA Stiftung
// SPDX-License-Identifier: Apache-2.0

/**
 * @author Sam Chen
 * @brief sensor service for getting sensor data
 *
 */

#ifndef __SENSOR_SERVICE_H__
#define __SENSOR_SERVICE_H__

#include <string>

#include "HTS221.h"
#include "I2C.h"

class sensorService {
public:
  int init(const mbed::I2C *i2c, const std::string &id);
  int toJSON(std::string &json_str);
  float temperature();
  float humidity();

private:
  HTS221 _hts221;
  std::string _ID;
};

#endif
