#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "stencil3d_v0.h"

#define DRV_NAME	"stencil3d_v0"

/* <<--regs-->> */
#define STENCIL3D_V0_ROW_SIZE_REG 0x50
#define STENCIL3D_V0_HEIGHT_SIZE_REG 0x4c
#define STENCIL3D_V0_COEF_1_REG 0x48
#define STENCIL3D_V0_COL_SIZE_REG 0x44
#define STENCIL3D_V0_COEF_0_REG 0x40

struct stencil3d_v0_device {
	struct esp_device esp;
};

static struct esp_driver stencil3d_v0_driver;

static struct of_device_id stencil3d_v0_device_ids[] = {
	{
		.name = "SLD_STENCIL3D_V0",
	},
	{
		.name = "eb_045",
	},
	{
		.compatible = "sld,stencil3d_v0",
	},
	{ },
};

static int stencil3d_v0_devs;

static inline struct stencil3d_v0_device *to_stencil3d_v0(struct esp_device *esp)
{
	return container_of(esp, struct stencil3d_v0_device, esp);
}

static void stencil3d_v0_prep_xfer(struct esp_device *esp, void *arg)
{
	struct stencil3d_v0_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->row_size, esp->iomem + STENCIL3D_V0_ROW_SIZE_REG);
	iowrite32be(a->height_size, esp->iomem + STENCIL3D_V0_HEIGHT_SIZE_REG);
	iowrite32be(a->coef_1, esp->iomem + STENCIL3D_V0_COEF_1_REG);
	iowrite32be(a->col_size, esp->iomem + STENCIL3D_V0_COL_SIZE_REG);
	iowrite32be(a->coef_0, esp->iomem + STENCIL3D_V0_COEF_0_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool stencil3d_v0_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct stencil3d_v0_device *stencil3d_v0 = to_stencil3d_v0(esp); */
	/* struct stencil3d_v0_access *a = arg; */

	return true;
}

static int stencil3d_v0_probe(struct platform_device *pdev)
{
	struct stencil3d_v0_device *stencil3d_v0;
	struct esp_device *esp;
	int rc;

	stencil3d_v0 = kzalloc(sizeof(*stencil3d_v0), GFP_KERNEL);
	if (stencil3d_v0 == NULL)
		return -ENOMEM;
	esp = &stencil3d_v0->esp;
	esp->module = THIS_MODULE;
	esp->number = stencil3d_v0_devs;
	esp->driver = &stencil3d_v0_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	stencil3d_v0_devs++;
	return 0;
 err:
	kfree(stencil3d_v0);
	return rc;
}

static int __exit stencil3d_v0_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct stencil3d_v0_device *stencil3d_v0 = to_stencil3d_v0(esp);

	esp_device_unregister(esp);
	kfree(stencil3d_v0);
	return 0;
}

static struct esp_driver stencil3d_v0_driver = {
	.plat = {
		.probe		= stencil3d_v0_probe,
		.remove		= stencil3d_v0_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = stencil3d_v0_device_ids,
		},
	},
	.xfer_input_ok	= stencil3d_v0_xfer_input_ok,
	.prep_xfer	= stencil3d_v0_prep_xfer,
	.ioctl_cm	= STENCIL3D_V0_IOC_ACCESS,
	.arg_size	= sizeof(struct stencil3d_v0_access),
};

static int __init stencil3d_v0_init(void)
{
	return esp_driver_register(&stencil3d_v0_driver);
}

static void __exit stencil3d_v0_exit(void)
{
	esp_driver_unregister(&stencil3d_v0_driver);
}

module_init(stencil3d_v0_init)
module_exit(stencil3d_v0_exit)

MODULE_DEVICE_TABLE(of, stencil3d_v0_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("stencil3d_v0 driver");
