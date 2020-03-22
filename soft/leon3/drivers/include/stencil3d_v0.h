#ifndef _STENCIL3D_V0_H_
#define _STENCIL3D_V0_H_

#ifdef __KERNEL__
#include <linux/ioctl.h>
#include <linux/types.h>
#else
#include <sys/ioctl.h>
#include <stdint.h>
#ifndef __user
#define __user
#endif
#endif /* __KERNEL__ */

#include <esp.h>
#include <esp_accelerator.h>

struct stencil3d_v0_access {
	struct esp_access esp;
	/* <<--regs-->> */
	unsigned row_size;
	unsigned height_size;
	unsigned coef_1;
	unsigned col_size;
	unsigned coef_0;
	unsigned src_offset;
	unsigned dst_offset;
};

#define STENCIL3D_V0_IOC_ACCESS	_IOW ('S', 0, struct stencil3d_v0_access)

#endif /* _STENCIL3D_V0_H_ */
