#ifndef SOURCES_H
#define SOURCES_H

#include <stddef.h>

/* List all configured sources as JSON. */
int sources_list_json(char *buf, size_t size);

/* Replace the sources list from a JSON POST body. */
int sources_save(const char *json, size_t len);

/* Add a new source by URL.  On success, msg_buf receives the source name. */
int sources_add(const char *url, char *msg_buf, size_t msg_size);

/* Remove a source by index.  Index 0 (default) cannot be removed. */
int sources_remove(int index, char *msg_buf, size_t msg_size);

/* Build multi-source repository JSON (grouped by source).
 * Returns bytes written. */
size_t sources_multi_repository_list_json(char *buf, size_t size, int force_refresh);

/* Download and install a payload from a specific source.
 * source_id identifies the source; repo_url is used as fallback detail. */
int sources_multi_repository_install(const char *filename, const char *source_id,
                                     const char *repo_url, char *msg, size_t msg_size);

#endif
