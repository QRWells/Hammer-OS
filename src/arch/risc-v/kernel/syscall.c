#include "defs.h"
#include "interrupt.h"
#include "types.h"
#include "stdin.h"
#include "thread.h"
#include "fs.h"

const usize SYS_SHUTDOWN = 13;
const usize SYS_LSDIR = 20;
const usize SYS_CDDIR = 21;
const usize SYS_PWD = 22;
const usize SYS_OPEN = 56;
const usize SYS_CLOSE = 57;
const usize SYS_READ = 63;
const usize SYS_WRITE = 64;
const usize SYS_EXIT = 93;
const usize SYS_EXEC = 221;

usize sys_read(usize fd, u8 *base, usize len) {
  *base = (u8)pop_char();
  return 1;
}

usize sys_exec(char *path) {
  if (execute_cpu(path, get_current_tid()))
    yield_cpu();

  return 0;
}

usize sys_ls(char *path, int fd) {
  inode *current = get_current_thread()->process.fd[fd].inode;
  inode *inode;
  if (*path == 0) {
    inode = current;
  } else {
    inode = lookup(current, path);
  }
  if (inode == 0) {
    printf("ls: No such file or directory\n");
    return 0;
  }
  ls(inode);
  return 0;
}

usize sys_cd(char *path, int fd) {
  inode *current = get_current_thread()->process.fd[fd].inode;
  inode *inode = lookup(current, path);
  if (inode == 0) {
    printf("cd: No such file or directory\n");
    return 0;
  }
  if (inode->type != TYPE_DIR) {
    printf("%s: is not a directory!\n", inode->filename);
    return 0;
  }
  get_current_thread()->process.fd[fd].inode = inode;
  return 0;
}

int sys_open(char *path) {
  thread *thread = get_current_thread();
  int fd = alloc_fd(thread);
  if (fd == -1) {
    panic("Max file open!\n");
  }
  thread->process.fd_occupied[fd] = 1;
  file f;
  f.fd_type = FD_INODE;
  f.offset = 0;
  f.inode = lookup(0, path);
  thread->process.fd[fd] = f;
  return fd;
}

void sys_pwd(int fd) {
  inode *current = get_current_thread()->process.fd[fd].inode;
  char buf[256];
  char *path = get_inode_path(current, buf);
  printf("%s\n", path);
}

void sys_close(int fd) {
  thread *t = get_current_thread();
  dealloc_fd(t, fd);
}

usize syscall(usize id, usize args[3], interrupt_context *context) {
  switch (id) {
  case SYS_SHUTDOWN:
    shutdown();
    return 0;
  case SYS_LSDIR:
    sys_ls((char *)args[0], args[1]);
    return 0;
  case SYS_CDDIR:
    sys_cd((char *)args[0], args[1]);
    return 0;
  case SYS_OPEN:
    return sys_open((char *)args[0]);
  case SYS_PWD:
    sys_pwd(args[0]);
    return 0;
  case SYS_CLOSE:
    sys_close(args[0]);
    return 0;
  case SYS_READ:
    return sys_read(args[0], (u8 *)args[1], args[2]);
  case SYS_WRITE:
    console_putchar(args[0]);
    return 0;
  case SYS_EXIT:
    exit_from_cpu(args[0]);
    return 0;
  case SYS_EXEC:
    sys_exec((char *)args[0]);
    return 0;
  default:
    printf("Unknown syscall id %d\n", id);
    panic("");
    return -1;
  }
}
