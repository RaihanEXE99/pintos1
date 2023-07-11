#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);
// static int memread_user (void *src, void *des, size_t bytes);
void sys_exit(int status);

unsigned sys_tell(int fd);
void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}


static void
syscall_handler (struct intr_frame *f UNUSED)
{
  int syscall_number;

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

// static int
// memread_user (void *src, void *dst, size_t bytes)
// {
//   int32_t value;
//   size_t i;
//   for(i=0; i<bytes; i++) {
//     value = get_user(src + i);
//     if(value == -1) 
//       fail_invalid_access();

//     *(char*)(dst + i) = value & 0xff;
//   }
//   return (int)bytes;
// }

void sys_exit(int status) {
  printf("%s: exit(%d)\n", thread_current()->name, status);

  struct process_control_block *pcb = thread_current()->pcb;
  if(pcb != NULL) {
    pcb->exited = true;
    pcb->exitcode = status;
  }

  thread_exit();
}