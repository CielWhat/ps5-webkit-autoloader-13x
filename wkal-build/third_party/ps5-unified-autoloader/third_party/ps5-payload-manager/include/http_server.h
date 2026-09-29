#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <microhttpd.h>

/* MHD request handler callback — dispatches all routes. */
enum MHD_Result http_on_request(void *cls, struct MHD_Connection *conn,
                                const char *url, const char *method,
                                const char *version, const char *upload_data,
                                size_t *upload_data_size, void **con_cls);

#endif
