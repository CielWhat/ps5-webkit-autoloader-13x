#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "config.h"
#include "pldmgr.h"
#include "json_helpers.h"

/* ── Read / Write full config ──────────────────────────────── */

void config_read(PldmgrConfig *cfg) {
    FILE *f;
    char line[256];

    /* Defaults */
    cfg->autoload_enabled      = 0;
    cfg->last_repository_update = 0;
    cfg->auto_browser_open     = 1;
    cfg->autoload_delay        = 5;
    cfg->kill_disc_player      = 1;
    cfg->scan_usb_payloads     = 0;
    cfg->auto_install_app      = 1;
    cfg->multi_sources_enabled = 0;

    f = fopen(PLDMGR_CONFIG_PATH, "r");
    if (!f)
        return;

    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "AUTOLOAD_ENABLED=", 17) == 0)
            cfg->autoload_enabled = atoi(line + 17);
        else if (strncmp(line, "LAST_REPOSITORY_UPDATE=", 23) == 0)
            cfg->last_repository_update = atol(line + 23);
        else if (strncmp(line, "AUTO_BROWSER_OPEN=", 18) == 0)
            cfg->auto_browser_open = atoi(line + 18);
        else if (strncmp(line, "AUTOLOAD_DELAY=", 15) == 0)
            cfg->autoload_delay = atoi(line + 15);
        else if (strncmp(line, "KILL_DISC_PLAYER_ON_STARTUP=", 28) == 0)
            cfg->kill_disc_player = atoi(line + 28);
        else if (strncmp(line, "SCAN_USB_PAYLOADS=", 18) == 0)
            cfg->scan_usb_payloads = atoi(line + 18);
        else if (strncmp(line, "AUTO_INSTALL_APP=", 17) == 0)
            cfg->auto_install_app = atoi(line + 17);
        else if (strncmp(line, "MULTI_SOURCES_ENABLED=", 22) == 0)
            cfg->multi_sources_enabled = atoi(line + 22);
    }
    fclose(f);
}

int config_write(const PldmgrConfig *cfg) {
    FILE *f;

    mkdir(BASE_DATA_DIR, 0777);
    f = fopen(PLDMGR_CONFIG_PATH, "w");
    if (!f)
        return -1;

    fprintf(f, "AUTOLOAD_ENABLED=%d\n",      cfg->autoload_enabled ? 1 : 0);
    fprintf(f, "LAST_REPOSITORY_UPDATE=%ld\n", cfg->last_repository_update);
    fprintf(f, "AUTO_BROWSER_OPEN=%d\n",      cfg->auto_browser_open ? 1 : 0);
    fprintf(f, "AUTOLOAD_DELAY=%d\n",         cfg->autoload_delay);
    fprintf(f, "KILL_DISC_PLAYER_ON_STARTUP=%d\n", cfg->kill_disc_player ? 1 : 0);
    fprintf(f, "SCAN_USB_PAYLOADS=%d\n",      cfg->scan_usb_payloads ? 1 : 0);
    fprintf(f, "AUTO_INSTALL_APP=%d\n",       cfg->auto_install_app ? 1 : 0);
    fprintf(f, "MULTI_SOURCES_ENABLED=%d\n",  cfg->multi_sources_enabled ? 1 : 0);
    fclose(f);
    return 0;
}

/* ── Single-key helpers ────────────────────────────────────── */

int config_read_bool(const char *key, int default_val) {
    FILE *f = fopen(PLDMGR_CONFIG_PATH, "r");
    if (!f) return default_val;
    char line[256];
    int res = default_val;
    size_t key_len = strlen(key);
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, key, key_len) == 0 && line[key_len] == '=') {
            res = atoi(line + key_len + 1);
            break;
        }
    }
    fclose(f);
    return res;
}

int config_upsert_value(const char *key, const char *value) {
    FILE *f;
    char line[256];
    char old_lines[64][256];
    int line_count = 0;
    int replaced = 0;
    size_t key_len = strlen(key);

    ensure_dir_recursive(BASE_DATA_DIR);

    f = fopen(PLDMGR_CONFIG_PATH, "r");
    if (f) {
        while (line_count < 64 && fgets(line, sizeof(line), f)) {
            if (strncmp(line, key, key_len) == 0 && line[key_len] == '=') {
                snprintf(old_lines[line_count], sizeof(old_lines[line_count]),
                         "%s=%s\n", key, value);
                replaced = 1;
            } else {
                strncpy(old_lines[line_count], line, sizeof(old_lines[line_count]));
                old_lines[line_count][sizeof(old_lines[line_count]) - 1] = '\0';
            }
            line_count++;
        }
        fclose(f);
    }

    if (!replaced && line_count < 64) {
        snprintf(old_lines[line_count], sizeof(old_lines[line_count]),
                 "%s=%s\n", key, value);
        line_count++;
    }

    f = fopen(PLDMGR_CONFIG_PATH, "w");
    if (!f)
        return -1;
    for (int i = 0; i < line_count; i++) {
        fputs(old_lines[i], f);
    }
    fclose(f);
    return 0;
}

int config_read_last_update(long *out_ts) {
    FILE *f;
    char line[256];
    long ts = 0;

    *out_ts = 0;
    f = fopen(PLDMGR_CONFIG_PATH, "r");
    if (!f)
        return 0;

    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "LAST_REPOSITORY_UPDATE=", 23) == 0) {
            ts = atol(line + 23);
            break;
        }
    }
    fclose(f);

    *out_ts = ts;
    return 0;
}

int config_write_last_update(long ts) {
    char ts_buf[64];
    snprintf(ts_buf, sizeof(ts_buf), "%ld", ts);
    return config_upsert_value("LAST_REPOSITORY_UPDATE", ts_buf);
}

/* ── JSON config update handler (from /set_config POST body) ── */

/* Helper: extract a JSON boolean value for a given key.
 * Returns 1 for true, 0 for false, or unchanged if key not found. */
static int extract_json_bool(const char *json, const char *key, int current) {
    char search[64];
    snprintf(search, sizeof(search), "\"%s\"", key);
    char *pos = strstr(json, search);
    if (!pos) return current;
    char *val = strchr(pos, ':');
    if (!val) return current;
    val++;
    while (*val == ' ') val++;
    if (strncmp(val, "true", 4) == 0) return 1;
    if (strncmp(val, "false", 5) == 0) return 0;
    return current;
}

static int extract_json_int(const char *json, const char *key, int current) {
    char search[64];
    snprintf(search, sizeof(search), "\"%s\"", key);
    char *pos = strstr(json, search);
    if (!pos) return current;
    char *val = strchr(pos, ':');
    if (!val) return current;
    val++;
    while (*val == ' ') val++;
    return atoi(val);
}

int config_handle_set_json(const char *json_data) {
    if (!json_data)
        return -1;

    /* Read existing config */
    PldmgrConfig cfg;
    config_read(&cfg);

    /* Check if AUTOLOAD_ENABLED is present */
    int enabled = extract_json_bool(json_data, "AUTOLOAD_ENABLED", -1);
    if (enabled != -1) {
        cfg.autoload_enabled      = enabled;
        cfg.auto_browser_open     = extract_json_bool(json_data, "AUTO_BROWSER_OPEN", cfg.auto_browser_open);
        cfg.autoload_delay        = extract_json_int(json_data, "AUTOLOAD_DELAY", cfg.autoload_delay);
        cfg.kill_disc_player      = extract_json_bool(json_data, "KILL_DISC_PLAYER_ON_STARTUP", cfg.kill_disc_player);
        cfg.scan_usb_payloads     = extract_json_bool(json_data, "SCAN_USB_PAYLOADS", cfg.scan_usb_payloads);
        cfg.auto_install_app      = extract_json_bool(json_data, "AUTO_INSTALL_APP", cfg.auto_install_app);
        cfg.multi_sources_enabled = extract_json_bool(json_data, "MULTI_SOURCES_ENABLED", cfg.multi_sources_enabled);

        if (config_write(&cfg) == 0) {
            pldmgr_log("[PLDMGR] Saved config to %s\n", PLDMGR_CONFIG_PATH);
        }
        if (enabled == 0)
            pldmgr_autoload_abort();
    }

    /* Handle AUTOLOAD_LIST if present */
    char *list_start = strstr(json_data, "\"AUTOLOAD_LIST\"");
    if (list_start) {
        char *val = strchr(list_start, ':');
        if (val) {
            val++;
            while (*val == ' ' || *val == '\"')
                val++;

            char *list_end = strchr(val, '\"');
            size_t list_len = list_end ? (size_t)(list_end - val) : 0;
            char *list_val = malloc(list_len + 1);
            if (list_val) {
                memcpy(list_val, val, list_len);
                list_val[list_len] = '\0';

                mkdir(BASE_DATA_DIR, 0777);
                FILE *f = fopen(AUTOLOAD_CONFIG_PATH, "w");
                if (f) {
                    char *token = strtok(list_val, ",");
                    while (token) {
                        fprintf(f, "%s\n", token);
                        token = strtok(NULL, ",");
                    }
                    fclose(f);
                    pldmgr_log("[PLDMGR] Saved autoload list to %s\n",
                               AUTOLOAD_CONFIG_PATH);
                }
                free(list_val);
            }
        }
    }

    return 0;
}
