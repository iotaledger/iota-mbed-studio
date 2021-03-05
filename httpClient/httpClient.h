// Copyright 2021 IOTA Stiftung
// SPDX-License-Identifier: Apache-2.0

/**
 * @author Sam Chen
 * @brief Simple HTTPS Client for sending string data
 *
 */

#ifndef __HTTP_CLIENT_H__
#define __HTTP_CLIENT_H__

#include "llhttp.h"
#include "mbed.h"
#include <string>

#define HTTP_BUF_SIZE MBED_CONF_APP_HTTP_BUF

typedef struct {
  string buffer;            // data buffer
  uint64_t content_length;  // content_length
  unsigned int status_code; // http status code
} http_data_t;

typedef enum {
  HTTP_ST_UNINIT = 0,
  HTTP_ST_INIT,
  HTTP_ST_CONNECTED,
  HTTP_ST_REQ_HEADER_COMPLETE,
  HTTP_ST_REQ_DATA_COMPLETE,
  HTTP_ST_RES_HEADER_COMPLETE,
  HTTP_ST_RES_DATA_COMPLETE,
  HTTP_ST_CLOSE,
  HTTP_ST_ERROR
} http_state_t;

class httpClient {
public:
  httpClient() : _wifi(WiFiInterface::get_default_instance()) {
    // http parser init
    parser_setting.on_message_begin = on_message_begin;
    parser_setting.on_headers_complete = on_headers_complete;
    parser_setting.on_body = on_body;
    llhttp_init(&http_parser, HTTP_RESPONSE, &parser_setting);
  };
  ~httpClient();
  int connect();
  int post(const string &path, const string &data);
  int get(const string &path);
  int response_status_code();
  int socket_send(llhttp_method_t method, const string &path,
                  const string &data);
  string response_data(); // get response data

private:
  static int on_message_begin(llhttp_t *parser);
  static int on_headers_complete(llhttp_t *parser);
  static int on_body(llhttp_t *parser, char const *at, size_t length);
  int socket_prepare(); // init buffer and http status
  int socket_close();

  int send_header(llhttp_method_t method, const string &path, size_t data_len);
  int send_data(const string &data);
  int fetch_response_header();
  int fetch_response_data();
  int recv(string &response);

  http_data_t request;
  static http_data_t response;
  static http_state_t http_st;
  llhttp_t http_parser;
  llhttp_settings_t parser_setting;

  char recv_buf[HTTP_BUF_SIZE];

  WiFiInterface *_wifi;
  TLSSocket _tls;
};

#endif