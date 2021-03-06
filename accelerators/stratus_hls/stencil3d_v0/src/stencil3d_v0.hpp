// Copyright (c) 2011-2019 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __STENCIL3D_V0_HPP__
#define __STENCIL3D_V0_HPP__

#include "stencil3d_v0_conf_info.hpp"
#include "stencil3d_v0_debug_info.hpp"
#include "fpdata.hpp"

#include "esp_templates.hpp"

#include "stencil3d_v0_directives.hpp"

#define __round_mask(x, y) ((y)-1)
#define round_up(x, y) ((((x)-1) | __round_mask(x, y))+1)
/* <<--defines-->> */
#if (TYPEDEF == 0)
	#define DATA_WIDTH 32
#elif (TYPEDEF == 1)
	#define DATA_WIDTH FX_WIDTH
#endif

#define DMA_SIZE SIZE_WORD
//#define PLM_OUT_WORD 65536
//#define PLM_IN_WORD 65536
//#define PLM_OUT_WORD 144
//#define PLM_IN_WORD 144
#define PLM_OUT_WORD 4096
#define PLM_IN_WORD 4096

class stencil3d_v0 : public esp_accelerator_3P<DMA_WIDTH>
{
public:
    // Constructor
    SC_HAS_PROCESS(stencil3d_v0);
    stencil3d_v0(const sc_module_name& name)
    : esp_accelerator_3P<DMA_WIDTH>(name)
        , cfg("config")
    {
        // Signal binding
        cfg.bind_with(*this);

        // Map arrays to memories
        /* <<--plm-bind-->> */
        HLS_MAP_plm(plm_out_pong, PLM_OUT_NAME);
        HLS_MAP_plm(plm_out_ping, PLM_OUT_NAME);
        HLS_MAP_plm(plm_in_pong, PLM_IN_NAME);
        HLS_MAP_plm(plm_in_ping, PLM_IN_NAME);
    }

    // Processes

    // Load the input data
    void load_input();

    // Computation
    void compute_kernel();

    // Store the output data
    void store_output();

    // Configure stencil3d_v0
    esp_config_proc cfg;

    // Functions

    // Private local memories
    int32_t map_adj;
    int32_t rem_fwd;
    sc_dt::sc_int<DATA_WIDTH> plm_in_ping[PLM_IN_WORD];
    sc_dt::sc_int<DATA_WIDTH> plm_in_pong[PLM_IN_WORD];
    sc_dt::sc_int<DATA_WIDTH> plm_out_ping[PLM_OUT_WORD];
    sc_dt::sc_int<DATA_WIDTH> plm_out_pong[PLM_OUT_WORD];


};


#endif /* __STENCIL3D_V0_HPP__ */
