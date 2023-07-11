#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);
unsigned sys_tell(int fd);
void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}


static void
syscall_handler (struct intr_frame *f UNUSED)
{
  switch (syscall_number) {
  case SYS_TELL: 
    {
      int fd;
      unsigned return_code;

      memread_user(f->esp + 4, &fd, sizeof(fd));

      return_code = sys_tell(fd);
      f->eax = (uint32_t) return_code;
      break;
    }

  default:
    printf("system call %d failed!\n", syscall_number);

    sys_exit(-1);
    break;
  }

  printf ("system call!\n");
  thread_exit ();
}

unsigned sys_tell(int fd) {
  lock_acquire (&filesys_lock);
  struct file_desc* file_d = find_file_desc(thread_current(), fd, FD_FILE);

  unsigned ret;
  if(file_d && file_d->file) {
    ret = file_tell(file_d->file);
  }
  else
    ret = -1;

  lock_release (&filesys_lock);
  return ret;
}
