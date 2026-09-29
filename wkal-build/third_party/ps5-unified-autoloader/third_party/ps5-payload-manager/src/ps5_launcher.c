#include "ps5_launcher.h"
#include "pldmgr.h"
#include "history_mgr.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include <signal.h>
#include <stdint.h>
#include <sys/sysctl.h>
#include <sys/user.h>

int ps5_launch_elf(const char *path) {
  pldmgr_log("[PLDMGR] Sending ELF to local loader: %s\n", path);

  /* Open the payload file */
  int fd = open(path, O_RDONLY);
  if (fd < 0) {
    pldmgr_log("[PLDMGR] !!! Failed to open payload: %s\n", path);
    return -1;
  }

  struct stat st;
  fstat(fd, &st);
  size_t total_sent = 0;

  /* Create socket to local elfldr */
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("socket");
    close(fd);
    return -1;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(ELFLDR_PORT);
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    pldmgr_log("[PLDMGR] !!! Connection to elfldr (port %d) failed. Is it "
           "running?\n",
           ELFLDR_PORT);
    close(sock);
    close(fd);
    return -1;
  }

  /* Stream the file */
  char buffer[8192];
  ssize_t read_bytes;
  int error = 0;
  while ((read_bytes = read(fd, buffer, sizeof(buffer))) > 0) {
    size_t offset = 0;
    while (offset < (size_t)read_bytes) {
      ssize_t sent = send(sock, buffer + offset, (size_t)read_bytes - offset, 0);
      if (sent <= 0) {
        pldmgr_log("[PLDMGR] !!! send failed while launching payload\n");
        error = 1;
        break;
      }
      offset += (size_t)sent;
      total_sent += (size_t)sent;
    }
    if (error)
      break;
  }

  if (read_bytes < 0) {
    pldmgr_log("[PLDMGR] !!! read failed while launching payload\n");
    error = 1;
  }

  pldmgr_log("[PLDMGR] Sent %zu bytes to loader.\n", total_sent);

  close(sock);
  close(fd);
  if (!error) {
    history_mgr_add(path);
  }
  return error ? -1 : 0;
}

extern int sceSystemServiceLaunchWebBrowser(const char *uri);

int ps5_launch_browser(const char *uri) {
  pldmgr_log("[PLDMGR] Launching browser: %s\n", uri);
  if (sceSystemServiceLaunchWebBrowser(uri) != 0) {
    pldmgr_notify("[PLDMGR] !!! Failed to launch browser.");
    return -1;
  }
  return 0;
}

/* LncUtil Externs */
extern int sceLncUtilGetAppIdOfRunningBigApp();
extern int sceLncUtilGetAppTitleId(uint32_t app_id, char *title_id);
extern int sceLncUtilSuspendApp(uint32_t app_id);
extern int sceLncUtilKillApp(uint32_t app_id);

static pid_t get_pid_by_name(const char *process_name) {
  int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PROC, 0};
  size_t buf_size;

  if (sysctl(mib, 4, NULL, &buf_size, NULL, 0))
    return -1;

  void *buf = malloc(buf_size);
  if (!buf)
    return -1;

  if (sysctl(mib, 4, buf, &buf_size, NULL, 0)) {
    free(buf);
    return -1;
  }

  pid_t pid = -1;
  for (void *ptr = buf; ptr < (buf + buf_size);
       ptr += ((struct kinfo_proc *)ptr)->ki_structsize) {
    struct kinfo_proc *ki = (struct kinfo_proc *)ptr;
    if (ki->ki_structsize < sizeof(struct kinfo_proc))
      break;
    if (strncmp(ki->ki_comm, process_name, sizeof(ki->ki_comm)) == 0) {
      pid = ki->ki_pid;
      break;
    }
  }

  free(buf);
  return pid;
}

int ps5_kill_disc_player() {
  const char *target_process = "SceDiscPlayer";
  const char *target_title_id = "NPXS40140";

  uint32_t app_id = sceLncUtilGetAppIdOfRunningBigApp();
  if (app_id == 0xffffffff) {
    return 0; /* No app running */
  }

  char title_id[16] = {0};
  if (sceLncUtilGetAppTitleId(app_id, title_id) != 0) {
    return 0; /* Could not get title ID */
  }

  if (strcmp(title_id, target_title_id) != 0) {
    return 0; /* Not the disc player */
  }

  pldmgr_notify("Disc Player detected. Terminating...\n");

  /* 1. Suspend */
  if (sceLncUtilSuspendApp(app_id) != 0) {
    pldmgr_notify("Failed to suspend Disc Player");
    return -1;
  }

  /* Wait for home screen transition stability */
  sleep(2);

  /* 2. SIGKILL */
  pid_t pid = get_pid_by_name(target_process);
  if (pid != -1) {
    pldmgr_log("Sending SIGKILL to %s (PID: %d)\n", target_process, pid);
    if (kill(pid, SIGKILL) == 0) {
      pldmgr_log("SIGKILL Sent to Disc Player");
    } else {
      pldmgr_notify("Warning: SIGKILL Failed");
    }
    sleep(1);
  }

  /* 3. LncUtil Kill */
  int result = sceLncUtilKillApp(app_id);
  if (result == 0) {
    pldmgr_log("Disc Player Fully Closed");
  } else {
    /* Check if it's already gone (crashed/closed) */
    if (sceLncUtilGetAppIdOfRunningBigApp() == 0xffffffff) {
      pldmgr_log("Disc Player Closed");
    } else {
      pldmgr_notify("!!! Failed to kill Disc Player (result: %d)", result);
      return -1;
    }
  }

  return 0;
}
