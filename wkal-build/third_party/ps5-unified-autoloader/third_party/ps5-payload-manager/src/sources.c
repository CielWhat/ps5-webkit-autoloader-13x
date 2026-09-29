#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

#include "sources.h"
#include "repository.h"
#include "sha256.h"
#include "json_helpers.h"
#include "config.h"
#include "pldmgr.h"
#include "payload_mgr.h"

/* ── Source entry type ─────────────────────────────────────── */

typedef struct {
    char id[64];
    char name[256];
    char url[1024];
    int  removable;
} SourceEntry;

/* ── Load / Save sources from SOURCES_CONFIG_PATH ──────────── */

static int load_sources(SourceEntry *out, int *out_count) {
    char *json = NULL;
    size_t size = 0;
    int count = 0;

    memset(out, 0, sizeof(SourceEntry) * MAX_SOURCES);

    /* Always start with the default source at index 0 */
    strncpy(out[0].id,   "default",             sizeof(out[0].id) - 1);
    strncpy(out[0].name, "Official Repository", sizeof(out[0].name) - 1);
    strncpy(out[0].url,  REPOSITORY_SOURCE_URL, sizeof(out[0].url) - 1);
    out[0].removable = 0;
    count = 1;

    if (read_file_text(SOURCES_CONFIG_PATH, &json, &size) != 0 || !json || size == 0) {
        if (json) free(json);
        *out_count = count;
        return 0;
    }

    /* Find "sources": [...] array */
    const char *arr_start = strstr(json, "\"sources\"");
    if (!arr_start) { free(json); *out_count = count; return 0; }

    arr_start = strchr(arr_start, '[');
    if (!arr_start) { free(json); *out_count = count; return 0; }

    const char *p = arr_start + 1;
    while (count < MAX_SOURCES && (p = strchr(p, '{')) != NULL) {
        const char *end = strchr(p, '}');
        if (!end) break;

        char id[64] = "", name[256] = "", url[1024] = "";
        char removable_str[8] = "";

        json_extract_string(p, end, "id",   id,   sizeof(id));
        json_extract_string(p, end, "name", name, sizeof(name));
        json_extract_string(p, end, "url",  url,  sizeof(url));
        json_extract_string(p, end, "removable", removable_str, sizeof(removable_str));

        /* Skip the default entry stored in file — we already added it */
        if (strcmp(id, "default") == 0) { p = end + 1; continue; }

        if (name[0] && url[0] && id[0]) {
            strncpy(out[count].id,   id,   sizeof(out[count].id) - 1);
            strncpy(out[count].name, name, sizeof(out[count].name) - 1);
            strncpy(out[count].url,  url,  sizeof(out[count].url) - 1);
            out[count].removable = (strcmp(removable_str, "false") != 0);
            count++;
        }
        p = end + 1;
    }

    free(json);
    *out_count = count;
    return 0;
}

static int save_sources(SourceEntry *sources, int count) {
    char buf[16384];
    JsonListBuilder jb = { buf, sizeof(buf), 0, 1 };

    ensure_dir_recursive(BASE_DATA_DIR);

    json_append(&jb, "{\"sources\":[\n");
    for (int i = 0; i < count; i++) {
        char id_e[128], name_e[512], url_e[2048];
        pldmgr_json_escape(sources[i].id,   id_e,   sizeof(id_e));
        pldmgr_json_escape(sources[i].name, name_e, sizeof(name_e));
        pldmgr_json_escape(sources[i].url,  url_e,  sizeof(url_e));
        if (json_append(&jb, "  {\"id\":\"%s\",\"name\":\"%s\",\"url\":\"%s\",\"removable\":%s}%s\n",
            id_e, name_e, url_e,
            sources[i].removable ? "true" : "false",
            (i < count - 1) ? "," : "") != 0) {
            break;
        }
    }
    json_append(&jb, "]}\n");

    return write_file_text(SOURCES_CONFIG_PATH, buf, jb.pos);
}

/* ── Public API ────────────────────────────────────────────── */

int sources_list_json(char *buf, size_t size) {
    SourceEntry *sources = calloc(MAX_SOURCES, sizeof(SourceEntry));
    if (!sources) return -1;
    int count = 0;
    load_sources(sources, &count);

    JsonListBuilder jb = { buf, size, 0, 1 };
    buf[0] = '\0';

    json_append(&jb, "{\"sources\":[\n");
    for (int i = 0; i < count; i++) {
        char id_e[128], name_e[512], url_e[2048];
        pldmgr_json_escape(sources[i].id,   id_e,   sizeof(id_e));
        pldmgr_json_escape(sources[i].name, name_e, sizeof(name_e));
        pldmgr_json_escape(sources[i].url,  url_e,  sizeof(url_e));
        if (json_append(&jb, "  {\"id\":\"%s\",\"name\":\"%s\",\"url\":\"%s\",\"removable\":%s}%s\n",
            id_e, name_e, url_e,
            sources[i].removable ? "true" : "false",
            (i < count - 1) ? "," : "") != 0) {
            break;
        }
    }
    json_append(&jb, "]}\n");
    free(sources);
    return 0;
}

int sources_save(const char *json, size_t len) {
    SourceEntry *sources = calloc(MAX_SOURCES, sizeof(SourceEntry));
    if (!sources) return -1;
    int count = 0;

    if (!json || len == 0) {
        free(sources);
        return -1;
    }

    /* Always put default at slot 0 */
    strncpy(sources[0].id,   "default",             sizeof(sources[0].id) - 1);
    strncpy(sources[0].name, "Official Repository", sizeof(sources[0].name) - 1);
    strncpy(sources[0].url,  REPOSITORY_SOURCE_URL, sizeof(sources[0].url) - 1);
    sources[0].removable = 0;
    count = 1;

    const char *arr_start = strstr(json, "\"sources\"");
    if (!arr_start) { free(sources); return -1; }
    arr_start = strchr(arr_start, '[');
    if (!arr_start) { free(sources); return -1; }

    const char *p = arr_start + 1;
    while (count < MAX_SOURCES && (p = strchr(p, '{')) != NULL) {
        const char *end = strchr(p, '}');
        if (!end) break;

        char id[64] = "", name[256] = "", url[1024] = "";
        json_extract_string(p, end, "id",   id,   sizeof(id));
        json_extract_string(p, end, "name", name, sizeof(name));
        json_extract_string(p, end, "url",  url,  sizeof(url));

        if (strcmp(id, "default") == 0) { p = end + 1; continue; }
        if (!name[0] || !url[0] || !id[0]) { p = end + 1; continue; }

        strncpy(sources[count].id,   id,   sizeof(sources[count].id) - 1);
        strncpy(sources[count].name, name, sizeof(sources[count].name) - 1);
        strncpy(sources[count].url,  url,  sizeof(sources[count].url) - 1);
        sources[count].removable = 1;
        count++;
        p = end + 1;
    }

    int ret = save_sources(sources, count);
    free(sources);
    return ret;
}

int sources_add(const char *url, char *msg_buf, size_t msg_size) {
    char tmp_path[512];
    char *json = NULL;
    size_t json_size = 0;
    RepoPayload *items = NULL;
    size_t count = 0;
    char source_name[256] = "";

    if (!url || !url[0]) {
        snprintf(msg_buf, msg_size, "URL is empty");
        return -1;
    }

    ensure_dir_recursive(BASE_DATA_DIR);
    snprintf(tmp_path, sizeof(tmp_path), "%s/source_validate.tmp", BASE_DATA_DIR);

    if (download_to_file(url, tmp_path) != 0) {
        snprintf(msg_buf, msg_size, "URL unreachable or download failed");
        remove(tmp_path);
        return -1;
    }

    if (read_file_text(tmp_path, &json, &json_size) != 0 || !json || json_size == 0) {
        snprintf(msg_buf, msg_size, "Failed to read downloaded JSON");
        remove(tmp_path);
        if (json) free(json);
        return -1;
    }
    remove(tmp_path);

    /* Read top-level "name" field.
     * Only search BEFORE the "payloads" key to avoid picking up individual
     * payload items' own "name" fields. */
    {
        const char *payloads_key = strstr(json, "\"payloads\"");
        if (payloads_key) {
            size_t search_len = (size_t)(payloads_key - json);
            const char *name_key = NULL;
            for (size_t i = 0; i + 6 <= search_len; i++) {
                if (memcmp(json + i, "\"name\"", 6) == 0) {
                    name_key = json + i;
                    break;
                }
            }
            if (name_key) {
                const char *colon = strchr(name_key + 6, ':');
                if (colon && (size_t)(colon - json) < search_len) {
                    const char *q = colon + 1;
                    while (*q == ' ' || *q == '\t') q++;
                    if (*q == '"') {
                        q++;
                        size_t pos = 0;
                        while (*q && *q != '"' && pos < sizeof(source_name) - 1)
                            source_name[pos++] = *q++;
                        source_name[pos] = '\0';
                    }
                }
            }
        }
        /* If still empty, fall back to the hostname portion of the URL */
        if (!source_name[0]) {
            const char *host = strstr(url, "://");
            host = host ? host + 3 : url;
            size_t pos = 0;
            while (*host && *host != '/' && *host != ':' &&
                   pos < sizeof(source_name) - 1)
                source_name[pos++] = *host++;
            source_name[pos] = '\0';
        }
    }

    if (parse_repository_payloads(json, &items, &count) != 0 || count == 0) {
        free(json);
        if (items) free(items);
        snprintf(msg_buf, msg_size, "No valid payloads found in source JSON");
        return -1;
    }
    free(items);
    free(json);

    /* Load current sources and append */
    SourceEntry *sources = calloc(MAX_SOURCES, sizeof(SourceEntry));
    if (!sources) {
        snprintf(msg_buf, msg_size, "Memory allocation failed");
        return -1;
    }
    int src_count = 0;
    load_sources(sources, &src_count);

    if (src_count >= MAX_SOURCES) {
        free(sources);
        snprintf(msg_buf, msg_size, "Maximum number of sources (%d) reached", MAX_SOURCES);
        return -1;
    }

    /* Check for duplicate URL */
    for (int i = 0; i < src_count; i++) {
        if (strcmp(sources[i].url, url) == 0) {
            free(sources);
            snprintf(msg_buf, msg_size, "Source already added");
            return -1;
        }
    }

    /* Assign a unique ID */
    char new_id[64];
    snprintf(new_id, sizeof(new_id), "source_%d", (int)time(NULL));

    strncpy(sources[src_count].id,   new_id,      sizeof(sources[src_count].id) - 1);
    strncpy(sources[src_count].name, source_name, sizeof(sources[src_count].name) - 1);
    strncpy(sources[src_count].url,  url,         sizeof(sources[src_count].url) - 1);
    sources[src_count].removable = 1;
    src_count++;

    if (save_sources(sources, src_count) != 0) {
        free(sources);
        snprintf(msg_buf, msg_size, "Failed to save sources");
        return -1;
    }

    free(sources);
    /* Return the source name in msg_buf for the caller to forward to the UI */
    snprintf(msg_buf, msg_size, "%s", source_name);
    return 0;
}

int sources_remove(int index, char *msg_buf, size_t msg_size) {
    if (index <= 0) {
        snprintf(msg_buf, msg_size, "Cannot remove the default source");
        return -1;
    }

    SourceEntry *sources = calloc(MAX_SOURCES, sizeof(SourceEntry));
    if (!sources) {
        snprintf(msg_buf, msg_size, "Memory allocation failed");
        return -1;
    }
    int count = 0;
    load_sources(sources, &count);

    if (index >= count) {
        free(sources);
        snprintf(msg_buf, msg_size, "Invalid source index");
        return -1;
    }

    /* Shift entries down */
    for (int i = index; i < count - 1; i++) {
        sources[i] = sources[i + 1];
    }
    count--;

    if (save_sources(sources, count) != 0) {
        free(sources);
        snprintf(msg_buf, msg_size, "Failed to save sources");
        return -1;
    }

    free(sources);
    snprintf(msg_buf, msg_size, "OK");
    return 0;
}

/* ── Multi-source repository ───────────────────────────────── */

static void get_source_cache_path(int index, char *out, size_t out_size) {
    if (index == 0) {
        strncpy(out, REPOSITORY_CACHE_PATH, out_size - 1);
        out[out_size - 1] = '\0';
    } else {
        snprintf(out, out_size, "%s.src%d.json", REPOSITORY_CACHE_PATH, index);
    }
}

static int ensure_source_fresh(const char *url, const char *cache_path, int force_refresh) {
    long last_update = 0;
    time_t now = time(NULL);
    char tmp_path[512];
    char *json = NULL;
    size_t json_size = 0;
    RepoPayload *items = NULL;
    size_t count = 0;

    ensure_dir_recursive(BASE_DATA_DIR);

    /* Read last update time from the cache file's companion .ts file */
    char ts_path[512];
    snprintf(ts_path, sizeof(ts_path), "%s.ts", cache_path);
    FILE *tf = fopen(ts_path, "r");
    if (tf) {
        fscanf(tf, "%ld", &last_update);
        fclose(tf);
    }

    if (!force_refresh && access(cache_path, F_OK) == 0) {
        long delta = (long)now - last_update;
        if (delta >= 0 && delta < REPOSITORY_REFRESH_INTERVAL_SEC) {
            return 0;
        }
    }

    snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", cache_path);
    if (download_to_file(url, tmp_path) != 0) {
        return -1;
    }

    if (read_file_text(tmp_path, &json, &json_size) != 0 || json_size == 0) {
        if (json) free(json);
        remove(tmp_path);
        return -1;
    }

    /* Validate: must have at least 1 payload */
    if (parse_repository_payloads(json, &items, &count) != 0 || count == 0) {
        free(items);
        free(json);
        remove(tmp_path);
        return -1;
    }
    free(items);
    free(json);

    if (rename(tmp_path, cache_path) != 0) {
        remove(tmp_path);
        return -1;
    }

    /* Write timestamp */
    tf = fopen(ts_path, "w");
    if (tf) {
        fprintf(tf, "%ld", (long)now);
        fclose(tf);
    }

    return 0;
}

size_t sources_multi_repository_list_json(char *buf, size_t size, int force_refresh) {
    SourceEntry *sources = calloc(MAX_SOURCES, sizeof(SourceEntry));
    if (!sources) {
        buf[0] = '\0';
        return 0;
    }
    int src_count = 0;
    load_sources(sources, &src_count);

    JsonListBuilder jb = { buf, size, 0, 1 };
    buf[0] = '\0';

    json_append(&jb, "{\"sources\":[\n");

    for (int si = 0; si < src_count; si++) {
        char cache_path[512];
        get_source_cache_path(si, cache_path, sizeof(cache_path));

        int fetch_ok = (ensure_source_fresh(sources[si].url, cache_path, force_refresh) == 0);

        char id_e[128], name_e[512];
        pldmgr_json_escape(sources[si].id,   id_e,   sizeof(id_e));
        pldmgr_json_escape(sources[si].name, name_e, sizeof(name_e));

        /* Read last_update timestamp */
        long last_update = 0;
        char ts_path[512];
        snprintf(ts_path, sizeof(ts_path), "%s.ts", cache_path);
        FILE *tf = fopen(ts_path, "r");
        if (tf) { fscanf(tf, "%ld", &last_update); fclose(tf); }

        if (json_append(&jb, "  {\"id\":\"%s\",\"name\":\"%s\",\"last_update\":%ld,\"error\":%s,\"payloads\":[\n",
            id_e, name_e, last_update, fetch_ok ? "false" : "true") != 0) {
            break;
        }

        if (fetch_ok) {
            char *cached = NULL;
            size_t cached_size = 0;
            RepoPayload *items = NULL;
            size_t count = 0;

            if (read_file_text(cache_path, &cached, &cached_size) == 0 && cached) {
                parse_repository_payloads(cached, &items, &count);
                free(cached);
            }

            for (size_t pi = 0; pi < count; pi++) {
                char name_pe[256], filename_e[512], desc_e[2048];
                char ver_e[128], url_e[2048], src_id_e[128], src_name_e[512];
                char last_upd_e[128], cat_e[256];

                pldmgr_json_escape(items[pi].name,        name_pe,    sizeof(name_pe));
                pldmgr_json_escape(items[pi].filename,    filename_e, sizeof(filename_e));
                pldmgr_json_escape(items[pi].description, desc_e,     sizeof(desc_e));
                pldmgr_json_escape(items[pi].version,     ver_e,      sizeof(ver_e));
                pldmgr_json_escape(items[pi].url,         url_e,      sizeof(url_e));
                pldmgr_json_escape(items[pi].last_update, last_upd_e, sizeof(last_upd_e));
                pldmgr_json_escape(items[pi].category,    cat_e,      sizeof(cat_e));
                pldmgr_json_escape(sources[si].id,        src_id_e,   sizeof(src_id_e));
                pldmgr_json_escape(sources[si].name,      src_name_e, sizeof(src_name_e));

                if (json_append(&jb, "    {\"name\":\"%s\",\"filename\":\"%s\",\"description\":\"%s\","
                    "\"version\":\"%s\",\"last_update\":\"%s\",\"url\":\"%s\","
                    "\"source_id\":\"%s\",\"source_name\":\"%s\",\"category\":\"%s\"}%s\n",
                    name_pe, filename_e, desc_e, ver_e, last_upd_e, url_e,
                    src_id_e, src_name_e, cat_e,
                    (pi < count - 1) ? "," : "") != 0) {
                    break;
                }
            }

            if (items) free(items);
        }

        if (json_append(&jb, "  ]}%s\n", (si < src_count - 1) ? "," : "") != 0) {
            break;
        }
    }

    json_append(&jb, "]}\n");
    free(sources);
    return jb.pos;
}

int sources_multi_repository_install(const char *filename, const char *source_id,
                                     const char *repo_url, char *msg, size_t msg_size) {
    SourceEntry *sources = calloc(MAX_SOURCES, sizeof(SourceEntry));
    if (!sources) {
        snprintf(msg, msg_size, "Memory allocation failed");
        return -1;
    }
    int src_count = 0;
    load_sources(sources, &src_count);

    /* Find the matching source */
    int si = -1;
    for (int i = 0; i < src_count; i++) {
        if (strcmp(sources[i].id, source_id) == 0) { si = i; break; }
    }

    if (si < 0) {
        /* Fall back to matching by URL */
        for (int i = 0; i < src_count; i++) {
            if (repo_url && strcmp(sources[i].url, repo_url) == 0) { si = i; break; }
        }
    }

    if (si < 0) {
        /* Last resort: use default */
        si = 0;
    }

    char cache_path[512];
    get_source_cache_path(si, cache_path, sizeof(cache_path));

    /* Ensure cache is fresh */
    if (access(cache_path, F_OK) != 0) {
        if (ensure_source_fresh(sources[si].url, cache_path, 1) != 0) {
            free(sources);
            snprintf(msg, msg_size, "Failed to fetch source repository");
            return -1;
        }
    }

    /* Load cached payloads for this source */
    char *cached = NULL;
    size_t cached_size = 0;
    RepoPayload *items = NULL;
    size_t count = 0;

    if (read_file_text(cache_path, &cached, &cached_size) != 0 || !cached) {
        free(sources);
        snprintf(msg, msg_size, "Repository cache unavailable");
        return -1;
    }

    parse_repository_payloads(cached, &items, &count);
    free(cached);

    /* Find the requested payload */
    int found = -1;
    for (size_t i = 0; i < count; i++) {
        if (strcmp(items[i].filename, filename) == 0) { found = (int)i; break; }
    }

    if (found < 0) {
        if (items) free(items);
        free(sources);
        snprintf(msg, msg_size, "Payload not found in source");
        return -1;
    }

    /* Download */
    ensure_dir_recursive(PAYLOADS_STORAGE_DIR);
    char tmp_path[512];
    snprintf(tmp_path, sizeof(tmp_path), "%s/%s.part", PAYLOADS_STORAGE_DIR, items[found].filename);

    if (download_to_file(items[found].url, tmp_path) != 0) {
        if (items) free(items);
        free(sources);
        remove(tmp_path);
        snprintf(msg, msg_size, "Download failed");
        return -1;
    }

    /* Commit */
    const char *detail = (repo_url && repo_url[0]) ? repo_url : sources[si].url;

    char folder_name[128];
    char payload_dir[512];
    char final_path[640];
    char details_path[700];

    pldmgr_utils_get_payload_folder_name(items[found].filename, folder_name, sizeof(folder_name));
    snprintf(payload_dir, sizeof(payload_dir), "%s/%s", PAYLOADS_STORAGE_DIR, folder_name);

    if (ensure_dir_recursive(payload_dir) != 0) {
        if (items) free(items);
        free(sources);
        remove(tmp_path);
        snprintf(msg, msg_size, "Failed to create payload directory");
        return -1;
    }

    snprintf(final_path,   sizeof(final_path),   "%s/%s",      payload_dir, items[found].filename);
    snprintf(details_path, sizeof(details_path), "%s/%s.json", payload_dir, items[found].filename);

    /* Checksum verification (optional — skip if checksum absent) */
    if (strlen(items[found].checksum) == 64) {
        char calculated[65];
        if (compute_sha256_file(tmp_path, calculated) != 0) {
            if (items) free(items);
            free(sources);
            remove(tmp_path);
            snprintf(msg, msg_size, "Checksum computation failed");
            return -1;
        }
        if (strcasecmp(calculated, items[found].checksum) != 0) {
            pldmgr_log("[PLDMGR] !!! Checksum mismatch for %s\n", items[found].filename);
            if (items) free(items);
            free(sources);
            remove(tmp_path);
            snprintf(msg, msg_size, "Checksum mismatch");
            return -1;
        }
    }

    payload_mgr_remove_old_files(payload_dir, items[found].filename);

    if (rename(tmp_path, final_path) != 0) {
        if (items) free(items);
        free(sources);
        remove(tmp_path);
        snprintf(msg, msg_size, "Failed to finalize payload file");
        return -1;
    }

    /* Attach source_id and source_name to the item so metadata captures them */
    strncpy(items[found].source,        sources[si].id,   sizeof(items[found].source) - 1);
    strncpy(items[found].source_direct, sources[si].name, sizeof(items[found].source_direct) - 1);
    strncpy(items[found].source_name,   sources[si].name, sizeof(items[found].source_name) - 1);

    if (write_payload_details_json(&items[found], details_path, "repository", detail) != 0) {
        if (items) free(items);
        free(sources);
        snprintf(msg, msg_size, "Failed to write payload metadata");
        return -1;
    }

    pldmgr_log("[PLDMGR] Multi-source install complete: %s (source: %s)\n",
               items[found].filename, sources[si].name);
    snprintf(msg, msg_size, "Installed %s", items[found].filename);
    if (items) free(items);
    free(sources);
    return 0;
}
