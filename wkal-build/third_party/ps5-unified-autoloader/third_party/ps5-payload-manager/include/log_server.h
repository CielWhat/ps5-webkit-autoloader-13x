#ifndef LOG_SERVER_H
#define LOG_SERVER_H

#include <stddef.h>
#include <stdint.h>
#include <microhttpd.h>

#define MAX_LOG_LINES 100
#define MAX_LOG_LINE_LEN 256

/* SSE log stream connection state */
struct LogSSEConnection {
    int last_log_version;
    int sent_initial;
};

/* MHD content reader callback for SSE log streaming */
ssize_t log_stream_callback(void *cls, uint64_t pos, char *buf, size_t max);

/* MHD free callback for SSE connections */
void log_stream_cleanup(void *cls);

/* Build the /log JSON response into buf. Returns bytes written. */
size_t log_build_json(char *buf, size_t buf_size);

/* Track whether any browser client has connected */
void log_server_set_active(void);
/* pldmgr_server_is_active() and pldmgr_log() are declared in pldmgr.h */

#endif
