#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x7305f8c7, "module_layout" },
	{ 0x427acfe4, "cdev_del" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x27e95276, "class_destroy" },
	{ 0xe5be0937, "platform_driver_unregister" },
	{ 0x511f2cfb, "cdev_add" },
	{ 0x76cff1fa, "cdev_init" },
	{ 0x8c86bcdd, "__platform_driver_register" },
	{ 0x88c3bdb4, "__class_create" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0xc6f46339, "init_timer_key" },
	{ 0x6ba78371, "device_create" },
	{ 0xc1a4d86d, "gpiod_direction_output_raw" },
	{ 0x47229b5c, "gpio_request" },
	{ 0xa7e72047, "of_get_named_gpio_flags" },
	{ 0xace72b4f, "_dev_err" },
	{ 0xfcee4474, "of_count_phandle_with_args" },
	{ 0xfe990052, "gpio_free" },
	{ 0x78dd2cab, "device_destroy" },
	{ 0x2b68bd2f, "del_timer" },
	{ 0xda02d67, "jiffies" },
	{ 0xc38c83b8, "mod_timer" },
	{ 0x7f02188f, "__msecs_to_jiffies" },
	{ 0x996bdb64, "_kstrtoul" },
	{ 0x93977d7e, "gpiod_set_raw_value" },
	{ 0x1e047854, "warn_slowpath_fmt" },
	{ 0x5f754e5a, "memset" },
	{ 0x28cc25db, "arm_copy_from_user" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0xdb7305a1, "__stack_chk_fail" },
	{ 0xf4fa543b, "arm_copy_to_user" },
	{ 0x97255bdf, "strlen" },
	{ 0x91715312, "sprintf" },
	{ 0x7b8ab45, "gpiod_get_raw_value" },
	{ 0x28c093b3, "gpio_to_desc" },
	{ 0x2e5810c6, "__aeabi_unwind_cpp_pr1" },
	{ 0x7c32d0f0, "printk" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "C33530706B5B5BC91758DC3");
