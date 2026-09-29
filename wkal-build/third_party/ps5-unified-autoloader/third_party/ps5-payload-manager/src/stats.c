#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/mount.h>
#include <sys/vmmeter.h>
#include "stats.h"
#include "pldmgr.h"

/* PS5 Kernel Exports */
extern int sceKernelGetCpuTemperature(int *temp);

/* CPU Usage Global State */
static long prev_cp_time[5] = {0};

static float get_cpu_usage() {
    long cp_time[5];
    size_t size = sizeof(cp_time);
    
    // CTL_KERN = 1, KERN_CP_TIME = 40
    int mib[2] = {1, 40};
    if (sysctl(mib, 2, &cp_time, &size, NULL, 0) < 0) {
        // Fallback to string name if numeric fails
        if (sysctlbyname("kern.cp_time", &cp_time, &size, NULL, 0) < 0) {
            return 0.0f;
        }
    }

    long total_diff = 0;
    for (int i = 0; i < 5; i++) {
        total_diff += (cp_time[i] - prev_cp_time[i]);
    }

    if (total_diff <= 0) {
        memcpy(prev_cp_time, cp_time, sizeof(cp_time));
        return 0.0f;
    }

    long idle_diff = cp_time[4] - prev_cp_time[4]; // CP_IDLE = 4
    float usage = 100.0f * (1.0f - ((float)idle_diff / (float)total_diff));

    memcpy(prev_cp_time, cp_time, sizeof(cp_time));
    return usage < 0 ? 0 : usage;
}

size_t pldmgr_get_system_stats_json(char *buf, size_t size) {
    /* CPU Info */
    float cpu_usage = get_cpu_usage();
    int cpu_temp = 0;
    sceKernelGetCpuTemperature(&cpu_temp);

    /* RAM Info */
    struct vmtotal vmt;
    size_t vmt_size = sizeof(vmt);
    uint64_t ram_total = 0;
    size_t ram_total_size = sizeof(ram_total);
    
    // hw.physmem: CTL_HW = 6, HW_PHYSMEM = 6
    int mib_phys[2] = {6, 6};
    if (sysctl(mib_phys, 2, &ram_total, &ram_total_size, NULL, 0) < 0) {
        if (sysctlbyname("hw.physmem", &ram_total, &ram_total_size, NULL, 0) < 0) {
            ram_total = 16 * 1024 * 1024 * 1024ULL; // Fallback 16GB
        }
    }

    uint64_t ram_used = 0;
    // vm.vmtotal: CTL_VM = 2, VM_METER = 1
    int mib_vmt[2] = {2, 1};
    if (sysctl(mib_vmt, 2, &vmt, &vmt_size, NULL, 0) == 0 || sysctlbyname("vm.vmtotal", &vmt, &vmt_size, NULL, 0) == 0) {
        unsigned int page_size = getpagesize();
        if (page_size == 0) page_size = 4096;
        
        // t_rm is total real memory in use (pages)
        ram_used = (uint64_t)vmt.t_rm * page_size;
        
        // If vmt.t_rm seems off, fallback to free pages count
        if (ram_used == 0 || ram_used > ram_total) {
            unsigned int free_pages = 0;
            size_t free_pages_size = sizeof(free_pages);
            if (sysctlbyname("vm.stats.vm.v_free_count", &free_pages, &free_pages_size, NULL, 0) == 0) {
                ram_used = ram_total - ((uint64_t)free_pages * page_size);
            }
        }
    }

    /* Storage Info */
    struct statfs *mounts;
    int num_fs = getfsstat(NULL, 0, MNT_NOWAIT);
    
    char storage_json[4096] = "[";
    size_t storage_pos = 1;

    if (num_fs > 0 && (mounts = malloc(num_fs * sizeof(struct statfs)))) {
        num_fs = getfsstat(mounts, num_fs * sizeof(struct statfs), MNT_NOWAIT);
        int entry_count = 0;
        for (int i = 0; i < num_fs && entry_count < 10; i++) {
            const char *label = NULL;
            const char *path = mounts[i].f_mntonname;
            
            if (strcmp(path, "/data") == 0) {
                label = "Internal storage";
            } else if (strncmp(path, "/mnt/ext", 8) == 0) {
                label = "NVME SSD";
            } else if (strncmp(path, "/mnt/usb", 8) == 0) {
                label = "External storage";
            } else if (strncmp(path, "/mnt/", 5) == 0) {
                // Any other mount in /mnt that wasn't caught above
                label = "System mount";
            }

            if (label) {
                uint64_t total = (uint64_t)mounts[i].f_blocks * mounts[i].f_bsize;
                uint64_t avail = (uint64_t)mounts[i].f_bavail * mounts[i].f_bsize;
                uint64_t used = total - avail;

                if (total == 0) continue; // Skip empty/invalid mounts

                if (entry_count > 0) {
                    storage_json[storage_pos++] = ',';
                }
                storage_pos += snprintf(storage_json + storage_pos, sizeof(storage_json) - storage_pos,
                    "{\"label\":\"%s\",\"total\":%lu,\"used\":%lu,\"path\":\"%s\"}",
                    label, total, used, path);
                entry_count++;
            }
        }
        free(mounts);
    }
    strcat(storage_json, "]");

    return snprintf(buf, size,
        "{\"cpu\":{\"usage\":%.1f,\"temp\":%d},"
        "\"ram\":{\"total\":%lu,\"used\":%lu},"
        "\"storage\":%s}",
        cpu_usage, cpu_temp, ram_total, ram_used, storage_json);
}
