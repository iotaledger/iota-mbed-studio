// Copyright 2021 IOTA Stiftung
// SPDX-License-Identifier: Apache-2.0

/**
 * @author Sam Chen
 * @brief IOTA Client
 *
 */
#ifndef __IOTA_CLIENT_H__
#define __IOTA_CLIENT_H__

#include "httpClient/httpClient.h"
#include "jsonUtils.h"
#include "main_config.h"
#include <string>
#include <vector>

class iotaAPI {
public:
  int getNodeInfo();
  int sendIndexation(const std::string &index, const std::string &data,
                     std::string &msg_id);
  int getTips(std::vector<std::string> &tips);

private:
  httpClient _http;
  jsonUtils _json;
};

#endif