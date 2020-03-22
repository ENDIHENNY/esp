#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "mac2.h"

#define DRV_NAME	"mac2"

/* <<--regs-->> */
#define MAC2_MAC_N_REG 0x48
#define MAC2_MAC_VEC_REG 0x44
#define MAC2_MAC_LEN_REG 0x40

struct mac2_device {
	struct esp_device esp;
};

static struct esp_driver mac2_driver;

static struct of_device_id mac2_device_ids[] = {
	{
		.name = "SLD_MAC2",
	},
	{
		.name = "eb_152",
	},
	{
		.compatible = "sld,mac2",
	},
	{ },
};

static int mac2_devs;

static inline struct mac2_device *to_mac2(struct esp_device *esp)
{
	return container_of(esp, struct mac2_device, esp);
}

static void mac2_prep_xfer(struct esp_device *esp, void *arg)
{
	struct mac2_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->mac_n, esp->iomem + MAC2_MAC_N_REG);
	iowrite32be(a->mac_vec, esp->iomem + MAC2_MAC_VEC_REG);
	iowrite32be(a->mac_len, esp->iomem + MAC2_MAC_LEN_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool mac2_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct mac2_device *mac2 = to_mac2(esp); */
	/* struct mac2_access *a = arg; */

	return true;
}

static int mac2_probe(struct platform_device *pdev)
{
	struct mac2_device *mac2;
	struct esp_device *esp;
	int rc;

	mac2 = kzalloc(sizeof(*mac2), GFP_KERNEL);
	if (mac2 == NULL)
		return -ENOMEM;
	esp = &mac2->esp;
	esp->module = THIS_MODULE;
	esp->number = mac2_devs;
	esp->driver = &mac2_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	mac2_devs++;
	return 0;
 err:
	kfree(mac2);
	return rc;
}

static int __exit mac2_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct mac2_device *mac2 = to_mac2(esp);

	esp_device_unregister(esp);
	kfree(mac2);
	return 0;
}

static struct esp_driver mac2_driver = {
	.plat = {
		.probe		= mac2_probe,
		.remove		= mac2_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = mac2_device_ids,
		},
	},
	.xfer_input_ok	= mac2_xfer_input_ok,
	.prep_xfer	= mac2_prep_xfer,
	.ioctl_cm	= MAC2_IOC_ACCESS,
	.arg_size	= sizeof(struct mac2_access),
};

static int __init mac2_init(void)
{
	return esp_driver_register(&mac2_driver);
}

static void __exit mac2_exit(void)
{
	esp_driver_unregister(&mac2_driver);
}

module_init(mac2_init)
module_exit(mac2_exit)

MODULE_DEVICE_TABLE(of, mac2_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("mac2 driver");
