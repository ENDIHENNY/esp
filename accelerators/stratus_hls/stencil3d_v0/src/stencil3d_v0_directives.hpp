// Copyright (c) 2011-2019 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __STENCIL3D_V0_DIRECTIVES_HPP__
#define __STENCIL3D_V0_DIRECTIVES_HPP__

#if (DMA_WIDTH == 32)
	#define DMA_BEAT_PER_WORD 1
	#define DMA_WORD_PER_BEAT 1
	#define PLM_IN_NAME "stencil3d_v0_plm_block_in_dma32_1w1r"
	#define PLM_OUT_NAME "stencil3d_v0_plm_block_out_dma32_1w1r"
#elif (DMA_WIDTH == 24)
	#define DMA_BEAT_PER_WORD 1
	#define DMA_WORD_PER_BEAT 1
	#define PLM_IN_NAME "stencil3d_v0_plm_block_in_dma24_1w1r"
	#define PLM_OUT_NAME "stencil3d_v0_plm_block_out_dma24_1w1r"
#elif (DMA_WIDTH == 28)
	#define DMA_BEAT_PER_WORD 1
	#define DMA_WORD_PER_BEAT 1
	#define PLM_IN_NAME "stencil3d_v0_plm_block_in_dma28_1w1r"
	#define PLM_OUT_NAME "stencil3d_v0_plm_block_out_dma28_1w1r"
#elif (DMA_WIDTH == 48)
	#define DMA_BEAT_PER_WORD 1
	#define DMA_WORD_PER_BEAT 2
	#define PLM_IN_NAME "stencil3d_v0_plm_block_in_dma48_2w1r"
	#define PLM_OUT_NAME "stencil3d_v0_plm_block_out_dma48_1w2r"
#elif (DMA_WIDTH == 16)
	#define DMA_BEAT_PER_WORD 1
	#define DMA_WORD_PER_BEAT 0
	#define PLM_IN_NAME "stencil3d_v0_plm_block_in_dma32_1w1r"
	#define PLM_OUT_NAME "stencil3d_v0_plm_block_out_dma32_1w1r"
#elif (DMA_WIDTH == 64)
	#define DMA_BEAT_PER_WORD 1
	#define DMA_WORD_PER_BEAT 2
	#define PLM_IN_NAME "stencil3d_v0_plm_block_in_dma64_2w1r"
	#define PLM_OUT_NAME "stencil3d_v0_plm_block_out_dma64_1w2r"
#elif (DMA_WIDTH == 256)
	#define DMA_BEAT_PER_WORD 1
	#define DMA_WORD_PER_BEAT 8
	#define PLM_IN_NAME "stencil3d_v0_plm_block_in_dma256_8w1r"
	#define PLM_OUT_NAME "stencil3d_v0_plm_block_out_dma256_1w8r"
#elif (DMA_WIDTH == 128)
	#define DMA_BEAT_PER_WORD 1
	#define DMA_WORD_PER_BEAT 4
	#define PLM_IN_NAME "stencil3d_v0_plm_block_in_dma128_4w1r"
	#define PLM_OUT_NAME "stencil3d_v0_plm_block_out_dma128_1w4r"
#endif


#if defined(STRATUS_HLS)

#define HLS_MAP_plm(_mem, _plm_block_name)      \
    HLS_MAP_TO_MEMORY(_mem, _plm_block_name)

#define HLS_PROTO(_s)                           \
    HLS_DEFINE_PROTOCOL(_s)

#define HLS_FLAT(_a)                            \
    HLS_FLATTEN_ARRAY(_a);

#define HLS_BREAK_DEP(_a)                       \
    HLS_BREAK_ARRAY_DEPENDENCY(_a)

#define HLS_UNROLL_SIMPLE                       \
    HLS_UNROLL_LOOP(ON)


#if defined(HLS_DIRECTIVES_BASIC)
// Load
# define HLS_LOAD_PLM_WRITE			\
    HLS_UNROLL_SIMPLE;				\
    HLS_BREAK_DEP(plm_in_ping);			\
    HLS_BREAK_DEP(plm_in_pong);	HLS_CONSTRAIN_LATENCY(0, HLS_ACHIEVABLE, "constraint-LOAD"); 

// Compute
# define HLS_COMPUTE_STENCIL 			\
    HLS_UNROLL_SIMPLE;				\
    HLS_BREAK_DEP(sol);				\
    HLS_BREAK_DEP(orig); HLS_CONSTRAIN_LATENCY(0, HLS_ACHIEVABLE, "constraint-COMPUTE"); 

// Store
# define HLS_STORE_PLM_READ			\
    HLS_UNROLL_SIMPLE;				\
    HLS_BREAK_DEP(plm_out_ping);		\
    HLS_BREAK_DEP(plm_out_pong); HLS_CONSTRAIN_LATENCY(0, HLS_ACHIEVABLE, "constraint-STORE"); 

#else

#error Unsupported or undefined HLS configuration

#endif /* HLS_DIRECTIVES_* */

#else /* !STRATUS_HLS */

#define HLS_MAP_plm(_mem, _plm_block_name)
#define HLS_PROTO(_s)
#define HLS_FLAT(_a)
#define HLS_BREAK_DEP(_a)
#define HLS_UNROLL_SIMPLE
#define HLS_LOAD_PLM_WRITE
#define HLS_COMPUTE_STENCIL
#define HLS_STORE_PLM_READ			

#endif /* STRATUS_HLS */

#endif /* __STENCIL3D_V0_DIRECTIVES_HPP_ */
