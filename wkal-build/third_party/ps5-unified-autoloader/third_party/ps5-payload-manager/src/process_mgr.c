#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/sysctl.h>
#include <stdint.h>

#include "process_mgr.h"
#include "json_helpers.h"
#include "pldmgr.h"

#define MiB(x) ((x) / (1024.0 * 1024))

typedef struct app_info {
    uint32_t app_id;
    uint64_t unknown1;
    char     title_id[14];
    char     unknown2[0x3c];
} app_info_t;

extern int sceKernelGetAppInfo(pid_t pid, app_info_t *info);

static int is_user_daemon(const char *name, uint32_t app_id) {
    if (!name) return 0;

    /* Specifically exclude mini-syscore.elf */
    if (strcmp(name, "mini-syscore.elf") == 0) return 0;

    /* Must have app_id == 0000 */
    if (app_id != 0) return 0;

    const char *ext = strrchr(name, '.');
    if (ext && strcasecmp(ext, ".elf") == 0) return 1;

    return 0;
}

size_t process_list_json(char *buf, size_t max_size) {
    int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PROC, 0};
    size_t buf_size = 0;
    void *sysctl_buf = NULL;

    JsonListBuilder jb = { buf, max_size, 0, 1 };
    buf[0] = '\0';

    json_append(&jb, "{\"processes\":[\n");

    if (sysctl(mib, 4, NULL, &buf_size, NULL, 0) == 0) {
        sysctl_buf = malloc(buf_size);
        if (sysctl_buf) {
            if (sysctl(mib, 4, sysctl_buf, &buf_size, NULL, 0) == 0) {
                int count = 0;
                for (void *ptr = sysctl_buf; ptr < (sysctl_buf + buf_size);) {
                    struct kinfo_proc *ki = (struct kinfo_proc*)ptr;
                    if (ki->ki_structsize == 0) break;
                    ptr += ki->ki_structsize;

                    app_info_t appinfo;
                    if(sceKernelGetAppInfo(ki->ki_pid, &appinfo)) {
                        memset(&appinfo, 0, sizeof(appinfo));
                    }

                    int is_daemon = is_user_daemon(ki->ki_comm, appinfo.app_id);

                    char name_e[512];
                    pldmgr_json_escape(ki->ki_comm, name_e, sizeof(name_e));

                    double mem_mib = MiB(ki->ki_rssize * PAGE_SIZE);

                    if (json_append(&jb, "%s  {\"pid\":%d,\"name\":\"%s\",\"memory\":%.1f,\"is_daemon\":%s}",
                        (count > 0) ? ",\n" : "",
                        (int)ki->ki_pid, name_e, mem_mib, is_daemon ? "true" : "false") != 0) {
                        break;
                    }
                    count++;
                }
            }
            free(sysctl_buf);
        }
    }

    json_append(&jb, "\n]}\n");
    return jb.pos;
}

int process_kill(int pid) {
    if (pid <= 0) return -1; /* Prevent killing kernel or init */

    /* Prevent killing our own process */
    if (pid == getpid()) return -1;

    if (kill(pid, SIGKILL) == 0) {
        return 0;
    }
    return -1;
}
