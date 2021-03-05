// Copyright 2021 IOTA Stiftung
// SPDX-License-Identifier: Apache-2.0

/**
 * @author Sam Chen
 * @brief Application configurations
 *
 */

#ifndef __APP_CONFIG_H__
#define __APP_CONFIG_H__

// main app
#define WIFI_SSID MBED_CONF_APP_WIFI_SSID
#define WIFI_PWD MBED_CONF_APP_WIFI_PASSWORD
#define WIFI_SECURITY MBED_CONF_APP_WIFI_SECURITY
#define SENSOR_DATA_INTERVAL MBED_CONF_APP_DATA_INTERVAL

// httpClient
#define HTTP_BUF_SIZE MBED_CONF_APP_HTTP_BUF
#define IOTA_NODE_HOST MBED_CONF_APP_HOST
#define IOTA_NODE_PORT MBED_CONF_APP_PORT

#endif