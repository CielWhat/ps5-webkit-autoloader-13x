#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "utils.h"

int pldmgr_get_local_ip(char *ip_buf, size_t buf_size) {
    struct ifaddrs *ifaddr, *ifa;
    int family, s;

    if (getifaddrs(&ifaddr) == -1) {
        return -1;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;

        family = ifa->ifa_addr->sa_family;

        if (family == AF_INET) {
            /* Skip loopback */
            if (strncmp(ifa->ifa_name, "lo", 2) == 0) continue;

            s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                           ip_buf, buf_size, NULL, 0, NI_NUMERICHOST);
            if (s == 0) {
                /* Found a valid IP */
                freeifaddrs(ifaddr);
                return 0;
            }
        }
    }

    freeifaddrs(ifaddr);
    return -1;
}

void pldmgr_utils_get_payload_folder_name(const char *filename, char *out_buf, size_t out_size) {
    char clean[256];
    strncpy(clean, filename, sizeof(clean) - 1);
    clean[sizeof(clean) - 1] = '\0';

    /* Strip extension */
    char *dot = strrchr(clean, '.');
    if (dot) *dot = '\0';

    /* Look for version marker like _v1.2.3 or -v1.2.3 */
    char *v = strstr(clean, "_v");
    if (!v) v = strstr(clean, "-v");
    
    if (v) {
        *v = '\0';
    } else {
        /* Fallback: look for just _ or - followed by digit */
        for (int i = 0; clean[i]; i++) {
            if ((clean[i] == '_' || clean[i] == '-') && (clean[i+1] >= '0' && clean[i+1] <= '9')) {
                clean[i] = '\0';
                break;
            }
        }
    }

    /* Further clean: remove -ps4, -ps5 suffixes if they were before the version */
    char *p = strstr(clean, "-ps5");
    if (!p) p = strstr(clean, "_ps5");
    if (!p) p = strstr(clean, "-ps4");
    if (!p) p = strstr(clean, "_ps4");
    if (p) *p = '\0';

    strncpy(out_buf, clean, out_size - 1);
    out_buf[out_size - 1] = '\0';
}

void pldmgr_json_escape(const char *src, char *dst, size_t dst_size) {
    size_t pos = 0;
    if (dst_size == 0) {
        return;
    }

    for (size_t i = 0; src[i] != '\0' && pos + 1 < dst_size; i++) {
        unsigned char c = (unsigned char)src[i];
        if ((c == '"' || c == '\\') && pos + 2 < dst_size) {
            dst[pos++] = '\\';
            dst[pos++] = (char)c;
        } else if (c >= 0x20 && c <= 0x7E) {
            dst[pos++] = (char)c;
        }
    }

    dst[pos] = '\0';
}
