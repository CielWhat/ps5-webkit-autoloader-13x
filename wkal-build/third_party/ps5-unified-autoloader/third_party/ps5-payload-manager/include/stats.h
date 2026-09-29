#pragma once

#include <stddef.h>

/**
 * Get system statistics as a JSON string.
 * @param buf The buffer to write to.
 * @param size The size of the buffer.
 * @return The number of bytes written.
 */
size_t pldmgr_get_system_stats_json(char *buf, size_t size);
