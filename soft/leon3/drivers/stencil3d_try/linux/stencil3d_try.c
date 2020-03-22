#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "stencil3d_try.h"

#define DRV_NAME	"stencil3d_try"

/* <<--regs-->> */
#define STENCIL3D_TRY_ROW_SIZE_REG 0x48
#define STENCIL3D_TRY_HEIGHT_SIZE_REG 0x44
#define STENCIL3D_TRY_COL_SIZE_REG 0x40

struct stencil3d_try_device {
	struct esp_device esp;
};

static struct esp_driver stencil3d_try_driver;

static struct of_device_id stencil3d_try_device_ids[] = {
	{
		.name = "SLD_STENCIL3D_TRY",
	},
	{
		.name = "eb_052",
	},
	{
		.compatible = "sld,stencil3d_try",
	},
	{ },
};

static int stencil3d_try_devs;

static inline struct stencil3d_try_device *to_stencil3d_try(struct esp_device *esp)
{
	return container_of(esp, struct stencil3d_try_device, esp);
}

static void stencil3d_try_prep_xfer(struct esp_device *esp, void *arg)
{
	struct stencil3d_try_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->row_size, esp->iomem + STENCIL3D_TRY_ROW_SIZE_REG);
	iowrite32be(a->height_size, esp->iomem + STENCIL3D_TRY_HEIGHT_SIZE_REG);
	iowrite32be(a->col_size, esp->iomem + STENCIL3D_TRY_COL_SIZE_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool stencil3d_try_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct stencil3d_try_device *stencil3d_try = to_stencil3d_try(esp); */
	/* struct stencil3d_try_access *a = arg; */

	return true;
}

static int stencil3d_try_probe(struct platform_device *pdev)
{
	struct stencil3d_try_device *stencil3d_try;
	struct esp_device *esp;
	int rc;

	stencil3d_try = kzalloc(sizeof(*stencil3d_try), GFP_KERNEL);
	if (stencil3d_try == NULL)
		return -ENOMEM;
	esp = &stencil3d_try->esp;
	esp->module = THIS_MODULE;
	esp->number = stencil3d_try_devs;
	esp->driver = &stencil3d_try_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	stencil3d_try_devs++;
	return 0;
 err:
	kfree(stencil3d_try);
	return rc;
}

static int __exit stencil3d_try_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct stencil3d_try_device *stencil3d_try = to_stencil3d_try(esp);

	esp_device_unregister(esp);
	kfree(stencil3d_try);
	return 0;
}

static struct esp_driver stencil3d_try_driver = {
	.plat = {
		.probe		= stencil3d_try_probe,
		.remove		= stencil3d_try_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = stencil3d_try_device_ids,
		},
	},
	.xfer_input_ok	= stencil3d_try_xfer_input_ok,
	.prep_xfer	= stencil3d_try_prep_xfer,
	.ioctl_cm	= STENCIL3D_TRY_IOC_ACCESS,
	.arg_size	= sizeof(struct stencil3d_try_access),
};

static int __init stencil3d_try_init(void)
{
	return esp_driver_register(&stencil3d_try_driver);
}

static void __exit stencil3d_try_exit(void)
{
	esp_driver_unregister(&stencil3d_try_driver);
}

module_init(stencil3d_try_init)
module_exit(stencil3d_try_exit)

MODULE_DEVICE_TABLE(of, stencil3d_try_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("stencil3d_try driver");
