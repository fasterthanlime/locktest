#define _ATFILE_SOURCE
#define _GNU_SOURCE
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/file.h>

#define perror(...) { fprintf(stderr, "perror at %s:%d in %s\n", __FILE__, __LINE__, __FUNCTION__); perror(__VA_ARGS__); }

#define LOCK_NAME "lock"

enum {
  LOCKTYPE_FCNTL,
  LOCKTYPE_FLOCK_EX,
  LOCKTYPE_FLOCK_SH
};

int _top_fd;

int open_top() {
  _top_fd = open(".", O_RDONLY);
}

int tup_top_fd() {
  return _top_fd;
}

typedef int tup_lock_t;

int tup_lock_open(const char *lockname, tup_lock_t *lock) {
  int fd;

  fd = openat(tup_top_fd(), lockname, O_RDWR|O_CREAT, 0666);
  if (fd < 0) {
    perror(lockname);
    fprintf(stderr, "test error: Unable to open lockfile.\n");
    return -1;
  }

  *lock = fd;
  return 0;
}

void tup_lock_close(tup_lock_t lock) {
  if (close(lock) < 0) {
    perror("close(lock)");
  }
}

int tup_flock(tup_lock_t fd, int locktype)
{
  if (locktype == LOCKTYPE_FCNTL) {
    struct flock fl = {
      .l_type = F_WRLCK,
      .l_whence = SEEK_SET,
      .l_start = 0,
      .l_len = 0,
    };

    if(fcntl(fd, F_SETLKW, &fl) < 0) {
      perror("fcntl F_WRLCK");
      return -1;
    }
    return 0;
  } else if (locktype == LOCKTYPE_FLOCK_EX) {
    return flock(fd, LOCK_EX);
  } else if (locktype == LOCKTYPE_FLOCK_SH) {
    return flock(fd, LOCK_SH);
  }
}

int tup_unflock(tup_lock_t fd, int locktype)
{
  if (locktype == LOCKTYPE_FCNTL) {
    struct flock fl = {
      .l_type = F_UNLCK,
      .l_whence = SEEK_SET,
      .l_start = 0,
      .l_len = 0,
    };

    if(fcntl(fd, F_SETLKW, &fl) < 0) {
      perror("fcntl F_UNLCK");
      return -1;
    }
    return 0;
  } else if (locktype == LOCKTYPE_FLOCK_EX) {
    return flock(fd, LOCK_UN);
  } else if (locktype == LOCKTYPE_FLOCK_SH) {
    return flock(fd, LOCK_UN);
  }
}

void start_proc(int locktype, int i) {
  int pid = fork();

  if (pid == 0) {
    open_top();

    tup_lock_t lock;
    if (tup_lock_open(LOCK_NAME, &lock) < 0) {
      fprintf(stderr, "[%d] failed to open lock\n", i);
      exit(1);
    }

    if (tup_flock(lock, locktype) < 0) {
      fprintf(stderr, "[%d] failed to flock\n", i);
      exit(1);
    }

    fprintf(stderr, "[%d] locked!\n", i);
    sleep(1);

    if (tup_unflock(lock, locktype) < 0) {
      fprintf(stderr, "[%d] failed to unflock\n", i);
      exit(1);
    }
    fprintf(stderr, "[%d] unlocked!\n", i);
    exit(0);
  } else {
    return;
  }
}

const char *locktype_string(int locktype) {
  switch (locktype) {
    case LOCKTYPE_FCNTL:
      return "fcntl";
    case LOCKTYPE_FLOCK_EX:
      return "flock_ex";
    case LOCKTYPE_FLOCK_SH:
      return "flock_sh";
    default:
      return "unknown";
  }
}

void run_test(int locktype) {
  fprintf(stderr, "\n");
  fprintf(stderr, "================ <%s>\n", locktype_string(locktype));

  int num_procs = 3;
  for (int i = 0; i < num_procs; i++) {
    start_proc(locktype, i);
    usleep(100000);
  }

  for (int i = 0; i < num_procs; i++) {
    wait(NULL);
  }
  unlink(LOCK_NAME);

  fprintf(stderr, "================ </%s>\n", locktype_string(locktype));
}

int main(int argc, char **argv) {
  run_test(LOCKTYPE_FCNTL);
  run_test(LOCKTYPE_FLOCK_SH);
  run_test(LOCKTYPE_FLOCK_EX);
}
