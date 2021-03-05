// Copyright 2021 IOTA Stiftung
// SPDX-License-Identifier: Apache-2.0

/**
 * @author Sam Chen
 * @brief JSON Utils
 *
 */

#include "jsonUtils.h"

int jsonUtils::getString(cJSON const *const obj, char const key[],
                         std::string &str) {
  if (obj == NULL || key == NULL) {
    // invalid parameters
    printf("[%s:%d] invalid parameters\n", __func__, __LINE__);
    return -1;
  }

  cJSON *json_value = cJSON_GetObjectItemCaseSensitive(obj, key);
  if (json_value == NULL) {
    printf("[%s:%d] JSON key not found: %s\n", __func__, __LINE__, key);
    return -1;
  }

  if (cJSON_IsString(json_value) && (json_value->valuestring != NULL)) {
    str = json_value->valuestring;
  } else {
    printf("[%s:%d] %s is not a string\n", __func__, __LINE__, key);
    return -1;
  }

  return 0;
}

int jsonUtils::getBool(cJSON const *const obj, char const key[], bool *b) {
  if (obj == NULL || key == NULL || b == NULL) {
    // invalid parameters
    printf("[%s:%d] invalid parameters\n", __func__, __LINE__);
    return -1;
  }

  cJSON *json_value = cJSON_GetObjectItemCaseSensitive(obj, key);
  if (json_value == NULL) {
    printf("[%s:%d] JSON key not found: %s\n", __func__, __LINE__, key);
    return -1;
  }

  if (cJSON_IsBool(json_value)) {
    *b = cJSON_IsTrue(json_value);
  } else {
    printf("[%s:%d] %s is not a boolean\n", __func__, __LINE__, key);
    return -1;
  }

  return 0;
}

int jsonUtils::getInt(cJSON const *const obj, char const key[], int *i) {
  if (obj == NULL || key == NULL || i == NULL) {
    // invalid parameters
    printf("[%s:%d] invalid parameters\n", __func__, __LINE__);
    return -1;
  }

  cJSON *json_value = cJSON_GetObjectItemCaseSensitive(obj, key);
  if (json_value == NULL) {
    printf("[%s:%d] JSON key not found: %s\n", __func__, __LINE__, key);
    return -1;
  }

  if (cJSON_IsNumber(json_value)) {
    *i = json_value->valueint;
  } else {
    printf("[%s:%d] %s is not an number\n", __func__, __LINE__, key);
    return -1;
  }

  return 0;
}

int jsonUtils::getArrayString(cJSON const *const obj, char const key[],
                              std::vector<std::string> &arr) {
  if (obj == NULL || key == NULL) {
    // invalid parameters
    printf("[%s:%d] invalid parameters\n", __func__, __LINE__);
    return -1;
  }
  arr.clear();

  char *str = NULL;
  cJSON *json_item = cJSON_GetObjectItemCaseSensitive(obj, key);
  if (json_item == NULL) {
    printf("[%s:%d] JSON key not found: %s\n", __func__, __LINE__, key);
    return -1;
  }

  if (cJSON_IsArray(json_item)) {
    cJSON *current_obj = NULL;
    cJSON_ArrayForEach(current_obj, json_item) {
      str = cJSON_GetStringValue(current_obj);
      if (!str) {
        printf("[%s:%d] encountered non-string array member", __func__,
               __LINE__);
        return -1;
      }
      arr.push_back(str);
      // utarray_push_back(ut, &str);
    }
  } else {
    printf("[%s:%d] %s is not an array object\n", __func__, __LINE__, key);
    return -1;
  }

  return 0;
}

int jsonUtils::arrayString2JSON(std::vector<std::string> const &arr,
                                cJSON *const obj, char const key[]) {
  cJSON *array_obj = cJSON_CreateArray();
  if (arr.size() == 0 || !obj || !key) {
    printf("[%s:%d] invalid parameters\n", __func__, __LINE__);
    return -1;
  }

  if (array_obj == NULL) {
    printf("[%s:%d] create json array failed\n", __func__, __LINE__);
    return -1;
  }

  cJSON_AddItemToObject(obj, key, array_obj);

  for (auto str : arr) {
    cJSON_AddItemToArray(array_obj, cJSON_CreateString(str.c_str()));
  }
  return 0;
}