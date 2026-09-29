#include "history_mgr.h"
#include "json_helpers.h"
#include "pldmgr.h"
#include <string.h>
#include <stdio.h>

#define MAX_HISTORY_ITEMS 20
#define MAX_PATH_LEN 512

static char history[MAX_HISTORY_ITEMS][MAX_PATH_LEN];
static int history_count = 0;

void history_mgr_add(const char *path) {
    if (!path || path[0] == '\0') return;

    /* Check if already in history */
    int existing_idx = -1;
    for (int i = 0; i < history_count; i++) {
        if (strcmp(history[i], path) == 0) {
            existing_idx = i;
            break;
        }
    }

    if (existing_idx != -1) {
        /* Move existing item to the front (index 0 is most recent) */
        char temp[MAX_PATH_LEN];
        strncpy(temp, history[existing_idx], MAX_PATH_LEN - 1);
        temp[MAX_PATH_LEN - 1] = '\0';
        
        for (int i = existing_idx; i > 0; i--) {
            strncpy(history[i], history[i-1], MAX_PATH_LEN - 1);
            history[i][MAX_PATH_LEN - 1] = '\0';
        }
        strncpy(history[0], temp, MAX_PATH_LEN - 1);
        history[0][MAX_PATH_LEN - 1] = '\0';
    } else {
        /* Shift everything down */
        int shift_end = history_count < MAX_HISTORY_ITEMS ? history_count : MAX_HISTORY_ITEMS - 1;
        for (int i = shift_end; i > 0; i--) {
            strncpy(history[i], history[i-1], MAX_PATH_LEN - 1);
            history[i][MAX_PATH_LEN - 1] = '\0';
        }
        /* Insert at front */
        strncpy(history[0], path, MAX_PATH_LEN - 1);
        history[0][MAX_PATH_LEN - 1] = '\0';
        if (history_count < MAX_HISTORY_ITEMS) {
            history_count++;
        }
    }
}

size_t history_mgr_to_json(char *buf, size_t max_size) {
    JsonListBuilder jb = { buf, max_size, 0, 1 };
    buf[0] = '\0';

    json_append(&jb, "{\"history\":[\n");

    for (int i = 0; i < history_count; i++) {
        char path_escaped[1024];
        pldmgr_json_escape(history[i], path_escaped, sizeof(path_escaped));
        json_append(&jb, "%s  \"%s\"", (i > 0) ? ",\n" : "", path_escaped);
    }

    json_append(&jb, "\n]}\n");
    return jb.pos;
}
