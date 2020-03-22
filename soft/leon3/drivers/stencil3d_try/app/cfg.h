#ifndef __ESP_CFG_000_H__
#define __ESP_CFG_000_H__

#include "libesp.h"

typedef int32_t token_t;

/* <<--params-def-->> */
#define ROW_SIZE 16
#define HEIGHT_SIZE 32
#define COL_SIZE 16

/* <<--params-->> */
const int32_t row_size = ROW_SIZE;
const int32_t height_size = HEIGHT_SIZE;
const int32_t col_size = COL_SIZE;

#define NACC 1

esp_thread_info_t cfg_000[] = {
	{
		.run = true,
		.devname = "stencil3d_try.0",
		.type = stencil3d_try,
		/* <<--descriptor-->> */
		.desc.stencil3d_try_desc.row_size = ROW_SIZE,
		.desc.stencil3d_try_desc.height_size = HEIGHT_SIZE,
		.desc.stencil3d_try_desc.col_size = COL_SIZE,
		.desc.stencil3d_try_desc.src_offset = 0,
		.desc.stencil3d_try_desc.dst_offset = 0,
		.desc.stencil3d_try_desc.esp.coherence = ACC_COH_NONE,
		.desc.stencil3d_try_desc.esp.p2p_store = 0,
		.desc.stencil3d_try_desc.esp.p2p_nsrcs = 0,
		.desc.stencil3d_try_desc.esp.p2p_srcs = {"", "", "", ""},
	}
};

#endif /* __ESP_CFG_000_H__ */
