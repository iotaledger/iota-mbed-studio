// Copyright 2021 IOTA Stiftung
// SPDX-License-Identifier: Apache-2.0

/**
 * @author Sam Chen
 * @brief Simple HTTPS Client for sending string data
 *
 */

#include "httpClient.h"
#include "root_ca_cert.h"

#define IOTA_NODE_HOST MBED_CONF_APP_HOST
#define IOTA_NODE_PORT MBED_CONF_APP_PORT
// #define HTTP_DEBUG

http_data_t httpClient::response;
http_state_t httpClient::http_st;

int httpClient::on_message_begin(llhttp_t *parser) { return 0; }

int httpClient::on_headers_complete(llhttp_t *parser) {
  response.status_code = parser->status_code;
  response.content_length = parser->content_length;
  http_st = HTTP_ST_RES_HEADER_COMPLETE;
  return 0;
}

int httpClient::on_body(llhttp_t *parser, char const *at, size_t length) {
  response.buffer.append(at, length);
  return 0;
}

httpClient::~httpClient() {
  if (_wifi) {
    _wifi->disconnect();
  }
}

int httpClient::socket_prepare() {
  nsapi_size_or_error_t ret = 0;

  // socket open
  if ((ret = _tls.open(_wifi)) != NSAPI_ERROR_OK) {
    printf("TLS socket open failed: %d\n", ret);
    return ret;
  }

  // set root ca
  if ((ret = _tls.set_root_ca_cert(root_ca_pem)) != NSAPI_ERROR_OK) {
    printf("TLS set CA failed: %d\n", ret);
    return ret;
  }
  _tls.set_hostname(IOTA_NODE_HOST);

  SocketAddress addr;
  // hostname
  if ((ret = _wifi->gethostbyname(IOTA_NODE_HOST, &addr)) !=
      NSAPI_ERROR_OK) {
    printf("get address by hostname failed: %d\n", ret);
    return ret;
  }

#ifdef HTTP_DEBUG
  printf("%s address is %s\r\n", IOTA_NODE_HOST,
         (addr.get_ip_address() ? addr.get_ip_address() : "None"));
#endif
  // set port number
  addr.set_port(IOTA_NODE_PORT);

  // overwrite ssl veryfy to optional
  mbedtls_ssl_config *ssl_conf = _tls.get_ssl_config();
  ssl_conf->authmode = MBEDTLS_SSL_VERIFY_OPTIONAL;

  if ((ret = _tls.connect(addr)) != NSAPI_ERROR_OK) {
    printf("TLS socket connect failed: %d\n", ret);
    return ret;
  }

  // response buffer init
  response.buffer.clear();
  response.content_length = 0;
  response.status_code = 0;

  // request buffer init
  response.buffer.clear();
  response.content_length = 0;
  response.status_code = 0;

  http_st = HTTP_ST_INIT;
  return ret;
}

int httpClient::send_header(llhttp_method_t method, const string &path,
                            size_t data_len) {
  string header;
  if (method == HTTP_GET) {
    header.append("GET " + path + " HTTP/1.1\r\n");
  } else if (method == HTTP_POST) {
    header.append("POST " + path + " HTTP/1.1\r\n");
  } else {
    // not support yet
    return -1;
  }
  header.append("Host: " IOTA_NODE_HOST "\r\n"
                "Content-Type: application/json\r\n"
                "User-Agent: IOTA CClient\r\n"
                "Accept: */*\r\n"
                "Content-Length: ");
  header.append(to_string(data_len));
  header.append("\r\n\r\n");
  int sent_bytes = 0;
  sent_bytes = _tls.send(header.c_str(), header.length());
  http_st = HTTP_ST_REQ_HEADER_COMPLETE;
#ifdef HTTP_DEBUG
  printf("header: \n%s\n", header.c_str());
#endif
  return sent_bytes;
}

int httpClient::send_data(const string &data) {
  nsapi_size_t send_bytes = data.length();
  if (send_bytes <= 0) {
    // printf("no data\n");
    return 0;
  }
  nsapi_size_or_error_t bytes_sent = 0;
  while (send_bytes) {
    bytes_sent = _tls.send(data.c_str() + bytes_sent, send_bytes);

    if (bytes_sent < 0) {
      printf("socket send error: %d\n", bytes_sent);
      return -1;
    }
    send_bytes -= bytes_sent;
  }
  http_st = HTTP_ST_REQ_DATA_COMPLETE;
#ifdef HTTP_DEBUG
  printf("sent: %s\n", data.c_str());
#endif
  return 0;
}

int httpClient::fetch_response_header() {
  memset(recv_buf, 0, HTTP_BUF_SIZE);
  int received_bytes = 0;
  if (http_st < HTTP_ST_REQ_HEADER_COMPLETE) {
    printf("respnse header state error\n");
    return -1;
  }

  http_st = HTTP_ST_REQ_DATA_COMPLETE;

  nsapi_size_or_error_t bytes_or_err = 0;
  while (http_st < HTTP_ST_RES_HEADER_COMPLETE) {
    bytes_or_err = _tls.recv(recv_buf, HTTP_BUF_SIZE);
    if (bytes_or_err < 0) {
      printf("Error: socket recv: %d\n", bytes_or_err);
      return -1;
    }
    llhttp_execute(&http_parser, recv_buf, bytes_or_err);
    received_bytes += bytes_or_err;
  }
#ifdef HTTP_DEBUG
  printf("response header: \n%s\n", recv_buf);
#endif
  return response.content_length;
}

int httpClient::fetch_response_data() {
  // char recv_buf[512] = {};
  memset(recv_buf, 0, HTTP_BUF_SIZE);
  if (http_st < HTTP_ST_RES_HEADER_COMPLETE) {
    printf("fetch response status error!\n");
    return -1;
  }
  nsapi_size_or_error_t bytes_or_err = _tls.recv(recv_buf, HTTP_BUF_SIZE);
  if (bytes_or_err >= 0) {
    llhttp_execute(&http_parser, recv_buf, bytes_or_err);
  }
  return bytes_or_err;
}

int httpClient::socket_send(llhttp_method_t method, const string &path,
                            const string &data) {
  int ret = 0;
  http_st = HTTP_ST_UNINIT;

  switch (http_st) {
  case HTTP_ST_UNINIT:
    if ((ret = socket_prepare()) != 0) {
      printf("socket connect error!\n");
      goto done;
    }
  case HTTP_ST_INIT:
  case HTTP_ST_CONNECTED:
    if ((ret = send_header(method, path, data.length())) < 0) {
      printf("send header failed\n");
      goto done;
    }
  case HTTP_ST_REQ_HEADER_COMPLETE:
    if ((ret = send_data(data)) < 0) {
      printf("send data failed\n");
      goto done;
    }
  case HTTP_ST_REQ_DATA_COMPLETE:
    if ((ret = fetch_response_header()) < 0) {
      printf("get response header failed\n");
      goto done;
    }
  case HTTP_ST_RES_HEADER_COMPLETE:
    if (response.status_code != 200) {
      printf("Error: http status code %d\n", response.status_code);
      goto done;
    }
    while (fetch_response_data() <= 0) {
      printf("no data or server requests close");
      break;
    }
    break;
  default:
    break;
  }

done:
  _tls.close();
  return ret;
}

int httpClient::get(const string &path) {
  return socket_send(HTTP_GET, path, "");
}

int httpClient::post(const string &path, const string &data) {
  return socket_send(HTTP_POST, path, data);
}

string httpClient::response_data() { return response.buffer; }
int httpClient::response_status_code() { return response.status_code; }