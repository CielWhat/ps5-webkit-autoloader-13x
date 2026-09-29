#pragma once

#include <stdint.h>
#include <sys/types.h>

/**
 * Launch an ELF payload from the filesystem.
 * @param path Full path to the .elf or .bin file.
 * @return 0 on success, -1 on failure.
 */
int ps5_launch_elf(const char* path);

/**
 * Launch the PS5 web browser with a specific URL.
 * @param uri The URL to open.
 * @return 0 on success, -1 on failure.
 */
int ps5_launch_browser(const char* uri);

/**
 * Kill the "Disc Player" app (NPXS40140) if it is running.
 * @return 0 if not running or successfully killed, -1 on error.
 */
int ps5_kill_disc_player();

