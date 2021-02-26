
#include "HTS221.h"

// R/W address
#define HTS221_READ_ADDR 0xBF
#define HTS221_WRITE_ADDR 0xBE

// Reg mapping
#define HTS221_WHO_AM_I_ADDR 0x0F // value should be 0xBC
#define HST221_WHO_AM_I_VALUE 0xBC

#define HTS221_AV_CONF 0x10 // Humidity and temperature resolution mode

#define HTS221_CTRL_REG1 0x20
#define HTS221_POWER_DOWN_BIT 7
#define HTS221_BDU_BIT 2
#define HTS221_DATA_RATE_BIT 0

#define HTS221_CTRL_REG2 0x21
#define HTS221_CTRL_REG3 0x22
#define HTS221_STATUS_REG 0x27

// output registers
#define HTS221_HUMIDITY_OUT_L 0x28
#define HTS221_HUMIDITY_OUT_H 0x29
#define HTS221_TEMP_OUT_L 0x2A
#define HTS221_TEMP_OUT_H 0x2B

// calibration coefficients for data conversion
#define HTS221_H0_RH_X2 0x30
#define HTS221_H1_RH_X2 0x31
#define HTS221_T0_DEGC_X8 0x32
#define HTS221_T1_DEGC_X8 0x33
#define HTS221_T0_T1_MSB 0x35
#define HTS221_H0_T0_OUT_L 0x36
#define HTS221_H0_T0_OUT_H 0x37
#define HTS221_H1_T0_OUT_L 0x3A
#define HTS221_H1_T0_OUT_H 0x3B
#define HTS221_T0_OUT_L 0x3C
#define HTS221_T0_OUT_H 0x3D
#define HTS221_T1_OUT_L 0x3E
#define HTS221_T1_OUT_H 0x3F

int HTS221::init(mbed::I2C *i2c_dev) {
  int ret = 0;
  uint8_t dev_id = 0;
  _i2c = i2c_dev;

  if ((ret = getID(&dev_id)) != 0) {
    return ret;
  }

  if (dev_id != HST221_WHO_AM_I_VALUE) {
    return -1;
  }
  // trun on sensor
  if ((ret = powerOn(true)) != 0) {
    return ret;
  }
  // tempture and humidity coefficents
  return getCoefficients();
}

int HTS221::readReg(uint8_t addr, char *data, size_t data_len) {
  int ret = 0;
  if (!_i2c) {
    return -1;
  }
  _cmd = addr;
  if ((ret = _i2c->write(HTS221_WRITE_ADDR, &_cmd, 1)) != 0) {
    return ret;
  }
  return _i2c->read(HTS221_READ_ADDR, data, data_len);
}

int HTS221::getPowerStatus(bool *st) {
  _buf[0] = 0x0;
  int ret = 0;
  if ((ret = readReg(HTS221_CTRL_REG1, _buf, 1)) == 0) {
    *st = (bool)(_buf[0] >> HTS221_POWER_DOWN_BIT);
  }
  return ret;
}

int HTS221::powerOn(bool st) {
  if (!_i2c) {
    return -1;
  }
  _buf[0] = HTS221_CTRL_REG1;
  if (st) {
    _buf[1] = 0x81; // power on, disable block update, 1Hz
  } else {
    _buf[1] = 0x00;
  }
  return _i2c->write(HTS221_WRITE_ADDR, _buf, 2, false);
}

int HTS221::getID(uint8_t *id) {
  return readReg(HTS221_WHO_AM_I_ADDR, (char *)id, 1);
}

int HTS221::getTemptureF(float *value) {
  int ret = 0;
  float temp = 0.0;
  if ((ret = getTempture(&temp)) != 0) {
    return ret;
  }
  // Fahrenheit = (Celsius * 9 / 5) + 32
  *value = (temp * 9.0 / 5.0) + 32.0;
  return ret;
}

int HTS221::getTempture(float *value) {
  int ret = 0;
  uint16_t T_out;
  // get T_out
  if ((ret = readReg(HTS221_TEMP_OUT_L, _buf, 1)) != 0) {
    return ret;
  }
  if ((ret = readReg(HTS221_TEMP_OUT_H, _buf + 1, 1)) != 0) {
    return ret;
  }
  T_out = ((uint16_t)_buf[1] << 8) | ((uint16_t)_buf[0]);
  *value = (T_out * _tempSlope + _tempZero);
  // std::printf("slope: %f,  zero: %f, temp: %f\n", _tempSlope, _tempZero,
  // *value);
  return ret;
}

int HTS221::getHumidity(float *value) {
  int ret = 0;
  // Read the humidity value in raw counts H_T_OUT from registers 0x28 & 0x29
  // H_T_OUT
  if ((ret = readReg(HTS221_HUMIDITY_OUT_L, _buf, 1)) != 0) {
    return ret;
  }
  if ((ret = readReg(HTS221_HUMIDITY_OUT_H, _buf + 1, 1)) != 0) {
    return ret;
  }
  int16_t H_T_out = (int16_t)_buf[0] | ((int16_t)_buf[1] << 8);

  *value = (H_T_out * _humiSlope + _humiZero);
  return ret;
}

int HTS221::getCoefficients() {
  int ret = 0;
  // tempture
  int16_t T0_out, T1_out;
  int16_t T0_degC, T1_degC;
  // Read the value of coefficients T0_degC_x8 and T1_degC_x8 from registers
  // 0x32 & 0x33. T0_degC_x8 in buf[0]
  if ((ret = readReg(HTS221_T0_DEGC_X8, _buf, 1)) != 0) {
    return ret;
  }
  // T1_degC_x8 in buf[1]
  if ((ret = readReg(HTS221_T1_DEGC_X8, _buf + 1, 1)) != 0) {
    return ret;
  }

  // T0/T1 MSB in buf[2]
  if ((ret = readReg(HTS221_T0_T1_MSB, _buf + 2, 1)) != 0) {
    return ret;
  }

  // Divide by 8 the content of registers 0x32 (T0_degC_x8) and 0x33
  // (T1_degC_x8) in order to obtain the value of coefficients T0_degC and
  // T1_degC Read the MSB bits of T1_degC (T1.9 and T1.8 bit) and T0_degC (T0.9
  // and T0.8 bit) from register 0x35 to compute T0_DegC and T1_DegC.
  T0_degC = (((uint16_t)_buf[0] >> 3) | (((uint16_t)_buf[2] & 0x03) << 8));
  T1_degC = (((uint16_t)_buf[1] >> 3) | (((uint16_t)_buf[2] & 0x0c) << 6));
  // std::printf("T0_degC: %d, T1_degC:%d\n", T0_degC, T1_degC);

  // read T0_out and T1_out
  if ((ret = readReg(HTS221_T0_OUT_L, _buf, 1)) != 0) {
    return ret;
  }
  if ((ret = readReg(HTS221_T0_OUT_H, _buf + 1, 1)) != 0) {
    return ret;
  }
  if ((ret = readReg(HTS221_T1_OUT_L, _buf + 2, 1)) != 0) {
    return ret;
  }
  if ((ret = readReg(HTS221_T1_OUT_H, _buf + 3, 1)) != 0) {
    return ret;
  }
  T0_out = ((uint16_t)_buf[1] << 8) | ((uint16_t)_buf[0]);
  T1_out = ((uint16_t)_buf[3] << 8) | ((uint16_t)_buf[2]);
  // std::printf("[%02x %02x %02x %02x],  T0_degC: %d, T1_degC:%d\n", _buf[0],
  //             _buf[1], _buf[2], _buf[3], T0_degC, T1_degC);

  _tempSlope = (T1_degC - T0_degC) / (7.0 * (T1_out - T0_out));
  _tempZero = (T0_degC / 7.0) - _tempSlope * T0_out;

  // humidity
  int16_t H0_T0_out, H1_T0_out, H_T_out;
  int8_t H0_rh, H1_rh;
  // Read the value of coefficients H0_rH_x2 and H1_rH_x2 from registers 0x30 &
  // 0x31 H0_rh in buf[0]
  if ((ret = readReg(HTS221_H0_RH_X2, _buf, 1)) != 0) {
    return ret;
  }

  // H1_rh in buf[1]
  if ((ret = readReg(HTS221_H1_RH_X2, _buf + 1, 1)) != 0) {
    return ret;
  }
  // Divide by two the content of registers 0x30 (H0_rH_x2) and 0x31 (H1_rH_x2)
  // in order to obtain the value of coefficients H0_rH and H1_rH.
  H0_rh = _buf[0] >> 1;
  H1_rh = _buf[1] >> 1;

  // Read the value of H0_T0_OUT from registers 0x36 & 0x37
  // H0_T0_OUT
  if ((ret = readReg(HTS221_H0_T0_OUT_L, _buf, 1)) != 0) {
    return ret;
  }
  if ((ret = readReg(HTS221_H0_T0_OUT_H, _buf + 1, 1)) != 0) {
    return ret;
  }
  H0_T0_out = (uint16_t)_buf[0] | ((uint16_t)_buf[1] << 8);

  // Read the value of H1_T0_OUT from registers 0x3A & 0x3B
  // H1_T0_OUT
  if ((ret = readReg(HTS221_H1_T0_OUT_L, _buf, 1)) != 0) {
    return ret;
  }
  if ((ret = readReg(HTS221_H1_T0_OUT_H, _buf + 1, 1)) != 0) {
    return ret;
  }
  H1_T0_out = (uint16_t)_buf[0] | ((uint16_t)_buf[1] << 8);

  _humiSlope = (H1_rh - H0_rh) / (2.5 * H1_T0_out - H0_T0_out);
  _humiZero = (H0_rh / 2.5) - _humiSlope * H0_T0_out;
  return ret;
}

HTS221::~HTS221() {
  // trun off sensor
  powerOn(false);
}
