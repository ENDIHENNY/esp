#ifndef __ESP_CFG_000_H__
#define __ESP_CFG_000_H__

#include "libesp.h"
#include <stdlib.h>
#include <time.h>

typedef int32_t token_t;

/* <<--params-def-->> */
#define ROW_SIZE 16
#define HEIGHT_SIZE 32
#define COEF_1 -1
#define COL_SIZE 32
#define COEF_0 6

/* <<--params-->> */
const int32_t row_size = ROW_SIZE;
const int32_t height_size = HEIGHT_SIZE;
const int32_t coef_1 = COEF_1;
const int32_t col_size = COL_SIZE;
const int32_t coef_0 = COEF_0;

#define NACC 1

esp_thread_info_t cfg_000[] = {
	{
		.run = true,
		.devname = "stencil3d_v0.0",
		.type = stencil3d_v0,
		/* <<--descriptor-->> */
		.desc.stencil3d_v0_desc.row_size = ROW_SIZE,
		.desc.stencil3d_v0_desc.height_size = HEIGHT_SIZE,
		.desc.stencil3d_v0_desc.coef_1 = COEF_1,
		.desc.stencil3d_v0_desc.col_size = COL_SIZE,
		.desc.stencil3d_v0_desc.coef_0 = COEF_0,
		.desc.stencil3d_v0_desc.src_offset = 0,
		.desc.stencil3d_v0_desc.dst_offset = 0,
		.desc.stencil3d_v0_desc.esp.coherence = ACC_COH_NONE,
		.desc.stencil3d_v0_desc.esp.p2p_store = 0,
		.desc.stencil3d_v0_desc.esp.p2p_nsrcs = 0,
		.desc.stencil3d_v0_desc.esp.p2p_srcs = {"", "", "", ""},
	}
};

#endif /* __ESP_CFG_000_H__ */
