// Copyright 2021 IOTA Stiftung
// SPDX-License-Identifier: Apache-2.0

/**
 * @author Sam Chen
 * @brief HTS221 driver for Mbed OS
 *
 */

#ifndef __HTS221_DRIVER_H__
#define __HTS221_DRIVER_H__

#include "I2C.h"
#include <string>

class HTS221 {
public:
  int init(const mbed::I2C *i2c_dev);
  int getPowerStatus(bool *st);
  int getTemperature(float *value);
  int getTemperatureF(float *value);
  int getHumidity(float *value);
  int powerOn(bool st);
  int getID(uint8_t *id);
  int toJSON(std::string &j_str);
  ~HTS221();

private:
  mbed::I2C *_i2c = NULL;
  char _cmd = 0x0;
  char _buf[4] = {};
  float _tempSlope, _tempZero;
  float _humiSlope, _humiZero;
  int readReg(uint8_t addr, char *data, size_t data_len);
  int getCoefficients();
};

#endif //__HTS221_DRIVER_H__
