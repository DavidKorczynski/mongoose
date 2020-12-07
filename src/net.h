#pragma once

#include "arch.h"
#include "event.h"
#include "iobuf.h"

struct mg_mgr {
  struct mg_connection *conns;  // List of active connections
  struct mg_connection *dnsc;   // DNS resolver connection
  const char *dnsserver;        // DNS server URL
  int dnstimeout;               // DNS resolve timeout in milliseconds
#if MG_ARCH == MG_ARCH_FREERTOS
  SocketSet_t ss;  // NOTE(lsm): referenced from socket struct
#endif
};

struct mg_addr {
  uint16_t port;  // TCP or UDP port in network byte order
  uint32_t ip;    // IP address in network byte order
};

struct mg_connection {
  struct mg_connection *next;  // Linkage in struct mg_mgr :: connections
  struct mg_mgr *mgr;          // Our container
  struct mg_addr peer;         // Remote peer address
  void *fd;                    // Connected socket, or LWIP data
  struct mg_iobuf recv;        // Incoming data
  struct mg_iobuf send;        // Outgoing data
  mg_event_handler_t fn;       // User-specified event handler function
  void *fn_data;               // User-speficied function parameter
  mg_event_handler_t pfn;      // Protocol-specific handler function
  void *pfn_data;              // Protocol-specific function parameter
  char label[32];              // Arbitrary label
  void *tls;                   // TLS specific data
  unsigned is_listening : 1;   // Listening connection
  unsigned is_client : 1;      // Outbound (client) connection
  unsigned is_accepted : 1;    // Accepted (server) connection
  unsigned is_resolving : 1;   // Non-blocking DNS resolv is in progress
  unsigned is_connecting : 1;  // Non-blocking connect is in progress
  unsigned is_tls : 1;         // TLS-enabled connection
  unsigned is_tls_hs : 1;      // TLS handshake is in progress
  unsigned is_udp : 1;         // UDP connection
  unsigned is_websocket : 1;   // WebSocket connection
  unsigned is_hexdumping : 1;  // Hexdump in/out traffic
  unsigned is_draining : 1;    // Send remaining data, then close and free
  unsigned is_closing : 1;     // Close and free the connection immediately
  unsigned is_readable : 1;    // Connection is ready to read
  unsigned is_writable : 1;    // Connection is ready to write
};

void mg_mgr_poll(struct mg_mgr *, int ms);
void mg_mgr_init(struct mg_mgr *);
void mg_mgr_free(struct mg_mgr *);

struct mg_connection *mg_listen(struct mg_mgr *, const char *url,
                                mg_event_handler_t fn, void *fn_data);
struct mg_connection *mg_connect(struct mg_mgr *, const char *url,
                                 mg_event_handler_t fn, void *fn_data);
int mg_send(struct mg_connection *, const void *, size_t);
int mg_printf(struct mg_connection *, const char *fmt, ...);
int mg_vprintf(struct mg_connection *, const char *fmt, va_list ap);
char *mg_straddr(struct mg_connection *, char *, size_t);
bool mg_socketpair(int *s1, int *s2);
