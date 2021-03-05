// Copyright 2021 IOTA Stiftung
// SPDX-License-Identifier: Apache-2.0

/**
 * @author Sam Chen
 * @brief JSON Utils
 *
 */
#ifndef __JSON_UTILS_H__
#define __JSON_UTILS_H__

#include "cJSON.h"
#include <string>
#include <vector>

class jsonUtils {
public:
  int getString(cJSON const *const obj, char const key[], std::string &str);
  int getBool(cJSON const *const obj, char const key[], bool *b);
  int getInt(cJSON const *const obj, char const key[], int *i);
  int getArrayString(cJSON const *const obj, char const key[],
                     std::vector<std::string> &arr);
  int arrayString2JSON(std::vector<std::string> const &arr, cJSON *const obj,
                       char const key[]);
};

#endif