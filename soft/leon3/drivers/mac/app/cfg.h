#ifndef __ESP_CFG_000_H__
#define __ESP_CFG_000_H__

#include "libesp.h"

typedef /* <<--token-type-->> */ token_t;

/* <<--params-def-->> */

/* <<--params-->> */

#define NACC 1

esp_thread_info_t cfg_000[] = {
	{
		.run = true,
		.devname = "mac.0",
		.type = mac,
		/* <<--descriptor-->> */
		.desc.mac_desc.src_offset = 0,
		.desc.mac_desc.dst_offset = 0,
		.desc.mac_desc.esp.coherence = ACC_COH_NONE,
		.desc.mac_desc.esp.p2p_store = 0,
		.desc.mac_desc.esp.p2p_nsrcs = 0,
		.desc.mac_desc.esp.p2p_srcs = {"", "", "", ""},
	}
};

#endif /* __ESP_CFG_000_H__ */
