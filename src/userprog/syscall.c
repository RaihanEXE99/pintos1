#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);
static struct file_desc* find_file_desc(struct thread *, int fd);
static int memread_user (void *src, void *des, size_t bytes);
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
  struct file_desc* file_d = find_file_desc(thread_current(), fd);

  unsigned ret;
  if(file_d && file_d->file) {
    ret = file_tell(file_d->file);
  }
  else
    ret = -1;

  lock_release (&filesys_lock);
  return ret;
}

static int
memread_user (void *src, void *dst, size_t bytes)
{
  int32_t value;
  size_t i;
  for(i=0; i<bytes; i++) {
    value = get_user(src + i);
    if(value == -1) 
      fail_invalid_access();

    *(char*)(dst + i) = value & 0xff;
  }
  return (int)bytes;
}
static struct file_desc*
find_file_desc(struct thread *t, int fd)
{
  ASSERT (t != NULL);

  if (fd < 3) {
    return NULL;
  }

  struct list_elem *e;

  if (! list_empty(&t->file_descriptors)) {
    for(e = list_begin(&t->file_descriptors);
        e != list_end(&t->file_descriptors); e = list_next(e))
    {
      struct file_desc *desc = list_entry(e, struct file_desc, elem);
      if(desc->id == fd) {
        return desc;
      }
    }
  }
}
void sys_exit(int status) {
  printf("%s: exit(%d)\n", thread_current()->name, status);

  struct process_control_block *pcb = thread_current()->pcb;
  if(pcb != NULL) {
    pcb->exited = true;
    pcb->exitcode = status;
  }

  thread_exit();
}