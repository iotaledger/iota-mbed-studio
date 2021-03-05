// Copyright 2021 IOTA Stiftung
// SPDX-License-Identifier: Apache-2.0

/**
 * @author Sam Chen
 * @brief Simple IOTA Client implementation on Mbed OS
 */

#include "iotaAPI.h"

#define JSON_KEY_DATA "data"
#define JSON_KEY_TIP_MSG_IDS "tipMessageIds"

std::string string_to_hex(const std::string &input) {
  static const char hex_digits[] = "0123456789ABCDEF";

  std::string output;
  output.reserve(input.length() * 2);
  for (unsigned char c : input) {
    output.push_back(hex_digits[c >> 4]);
    output.push_back(hex_digits[c & 15]);
  }
  return output;
}

int iotaAPI::getNodeInfo() {
  // get node info
  if (_http.get("/api/v1/info") > 0) {
    printf("%s\n", _http.response_data().c_str());
    return 0;
  } else {
    return -1;
  }
}

int iotaAPI::getTips(std::vector<std::string> &tips) {
  // get tips
  if (_http.get("/api/v1/tips") > 0) {
    // printf("%s\n", _http.response_data().c_str());
    cJSON *obj = cJSON_Parse(_http.response_data().c_str());
    cJSON *data_obj = cJSON_GetObjectItemCaseSensitive(obj, JSON_KEY_DATA);
    if (data_obj) {
      _json.getArrayString(data_obj, JSON_KEY_TIP_MSG_IDS, tips);
    } else {
      cJSON_Delete(obj);
      return -1;
    }
    cJSON_Delete(obj);
    return 0;
  } else {
    return -1;
  }
}

int iotaAPI::sendIndexation(const std::string &index, const std::string &data,
                            std::string &msg_id) {
  int ret = 0;

  // get tips
  vector<string> tips;
  if ((ret = getTips(tips)) != 0) {
    printf("get tips failed\n");
    return -1;
  }

  // compose indexaction data in JSON
  cJSON *json_msg = cJSON_CreateObject();
  if (!json_msg) {
    printf("creating root json failed\n");
    return -1;
  }

  // message object
  /*
  {
  "networkId": "",
  "parentMessageIds": [
      "7dabd008324378d65e607975e9f1740aa8b2f624b9e25248370454dcd07027f3",
      "9f5066de0e3225f062e9ac8c285306f56815677fe5d1db0bbccecfc8f7f1e82c",
      "ccf9bf6b76a2659f332e17bfdc20f278ce25bc45e807e89cc2ab526cd2101c52",
      "fe63a9194eadb45e456a3c618d970119dbcac25221dbf5f53e5a838ef6ef518a"
  ],
  "payload": payload object
  "nonce": ""
  }
  */
  cJSON_AddStringToObject(json_msg, "networkId", "");
  _json.arrayString2JSON(tips, json_msg, "parentMessageIds");

  // indexation payload
  /*
  "payload": {
      "type": 2,
      "index": "696F74612E63",
      "data": "426172"
  }
  */
  cJSON *j_payload = cJSON_CreateObject();
  cJSON_AddNumberToObject(j_payload, "type", 2);
  string hex_index = string_to_hex(index);
  cJSON_AddStringToObject(j_payload, "index", hex_index.c_str());
  string hex_data = string_to_hex(data);
  cJSON_AddStringToObject(j_payload, "data", hex_data.c_str());
  cJSON_AddItemToObject(json_msg, "payload", j_payload);

  cJSON_AddStringToObject(json_msg, "nonce", "");

  string j_str = cJSON_PrintUnformatted(json_msg);
  // printf("%s\n", j_str.c_str());
  // send to node
  ret = _http.post("/api/v1/messages", j_str);
  if (ret > 0) {
    printf("%s\n", _http.response_data().c_str());
  }
  cJSON_Delete(json_msg);
  return 0;
}