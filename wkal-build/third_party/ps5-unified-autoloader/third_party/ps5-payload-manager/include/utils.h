#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Gets the local IP address. Returns 0 on success, -1 on failure. */
int pldmgr_get_local_ip(char *ip_buf, size_t buf_size);
void pldmgr_utils_get_payload_folder_name(const char *filename, char *out_buf, size_t out_size);
void pldmgr_json_escape(const char *src, char *dst, size_t dst_size);

#ifdef __cplusplus
}
#endif
