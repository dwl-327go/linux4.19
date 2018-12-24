// SPDX-License-Identifier: GPL-2.0+
/*
 * Driver for SMSC USB4604 USB HSIC 4-port 2.0 hub controller driver
 * Based on usb3503 driver
 *
 * Copyright (c) 2012-2013 Dongjin Kim (tobetter@gmail.com)
 * Copyright (c) 2016 Linaro Ltd.
 */

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/module.h>
#include <linux/gpio/consumer.h>
#include <linux/platform_device.h>

enum usb4604_mode {
	USB4604_MODE_UNKNOWN,
	USB4604_MODE_HUB,
	USB4604_MODE_STANDBY,
};

struct usb4604 {
	enum usb4604_mode	mode;
	struct device		*dev;
	int					gpio_reset;
};

struct usb4604_platform_data {
	enum usb4604_mode	init_mode;
	int					gpio_reset;
};

static void usb4604_reset(struct usb4604 *hub, int state)
{
	if (gpio_is_valid(hub->gpio_reset))
		gpio_set_value_cansleep(hub->gpio_reset, state);

	if (state)
		usleep_range(4000, 10000);
}

static int usb4604_connect(struct usb4604 *hub)
{
	struct device *dev = hub->dev;

	usb4604_reset(hub, 1);

	hub->mode = USB4604_MODE_HUB;
	dev_dbg(dev, "switched to HUB mode\n");

	return 0;
}

static int usb4604_switch_mode(struct usb4604 *hub, enum usb4604_mode mode)
{
	struct device *dev = hub->dev;
	int err = 0;

	switch (mode) {
	case USB4604_MODE_HUB:
		err = usb4604_connect(hub);
		break;

	case USB4604_MODE_STANDBY:
		usb4604_reset(hub, 0);
		dev_dbg(dev, "switched to STANDBY mode\n");
		break;

	default:
		dev_err(dev, "unknown mode is requested\n");
		err = -EINVAL;
		break;
	}

	return err;
}

static int usb4604_probe(struct usb4604 *hub)
{
	struct device *dev = hub->dev;
	struct usb4604_platform_data *pdata = dev_get_platdata(dev);
	struct device_node *np = dev->of_node;

	u32 mode = USB4604_MODE_HUB;

	if (pdata) {
		hub->gpio_reset = pdata->gpio_reset;
		hub->mode = pdata->init_mode;
	} else if (np) {
		hub->gpio_reset = of_get_named_gpio(np, "reset-gpios", 0);
		if (hub->gpio_reset == -EPROBE_DEFER)
			return -EPROBE_DEFER;
		of_property_read_u32(np, "initial-mode", &mode);
		hub->mode = mode;
	}

	if (gpio_is_valid(hub->gpio_reset)) {
		int err = devm_gpio_request_one(dev, hub->gpio_reset,
					GPIOF_OUT_INIT_LOW, "usb4604 reset");
		if (err) {
			dev_err(dev,
				"unable to request GPIO %d as connect pin (%d)\n",
				hub->gpio_reset, err);
			return err;
		}
	}

	return usb4604_switch_mode(hub, hub->mode);
}

static int usb4604_platform_probe(struct platform_device *pdev)
{
	struct usb4604 *hub;

	hub = devm_kzalloc(&pdev->dev, sizeof(*hub), GFP_KERNEL);
	if (!hub)
		return -ENOMEM;

	hub->dev = &pdev->dev;
	platform_set_drvdata(pdev, hub);

	return usb4604_probe(hub);
}


#ifdef CONFIG_OF
static const struct of_device_id usb4604_of_match[] = {
	{ .compatible = "smsc,usb4604" },
	{}
};
MODULE_DEVICE_TABLE(of, usb4604_of_match);
#endif

static struct platform_driver usb4604_platform_driver = {
	.driver = {
		.name			= "usb4604",
		.of_match_table = of_match_ptr(usb4604_of_match),
	},
	.probe	= usb4604_platform_probe,
};

static int __init usb4604_init(void)
{
	int err;

	err = platform_driver_register(&usb4604_platform_driver);
	if (err)
		pr_err("usb4604 :Failed to register platform driver :%d\n",
				err);
	return 0;
}
module_init(usb4604_init);

static void __exit usb4604_exit(void)
{
	platform_driver_unregister(&usb4604_platform_driver);
}
module_exit(usb4604_exit);


MODULE_DESCRIPTION("USB4604 USB HUB driver");
MODULE_LICENSE("GPL v2");
