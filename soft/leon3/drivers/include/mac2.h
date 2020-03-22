#ifndef _MAC2_H_
#define _MAC2_H_

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

struct mac2_access {
	struct esp_access esp;
	/* <<--regs-->> */
	unsigned mac_n;
	unsigned mac_vec;
	unsigned mac_len;
	unsigned src_offset;
	unsigned dst_offset;
};

#define MAC2_IOC_ACCESS	_IOW ('S', 0, struct mac2_access)

#endif /* _MAC2_H_ */
