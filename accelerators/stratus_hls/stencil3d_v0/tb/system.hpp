// Copyright (c) 2011-2019 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __SYSTEM_HPP__
#define __SYSTEM_HPP__

#include "stencil3d_v0_conf_info.hpp"
#include "stencil3d_v0_debug_info.hpp"
#include "stencil3d_v0.hpp"
#include "stencil3d_v0_directives.hpp"

#include "esp_templates.hpp"

const size_t MEM_SIZE = 131072 / (DMA_WIDTH/8);

#include "core/systems/esp_system.hpp"

#ifdef CADENCE
#include "stencil3d_v0_wrap.h"
#endif

#define MAX 1000
#define MIN 1

#if (TYPEDEF == 0)
    #define TYPE int32_t
#elif (TYPEDEF == 1)
    #define TYPE float
#endif

class system_t : public esp_system<DMA_WIDTH, MEM_SIZE>
{
public:

    // ACC instance
#ifdef CADENCE
    stencil3d_v0_wrapper *acc;
#else
    stencil3d_v0 *acc;
#endif

    // Constructor
    SC_HAS_PROCESS(system_t);
    system_t(sc_module_name name)
        : esp_system<DMA_WIDTH, MEM_SIZE>(name)
    {
        // ACC
#ifdef CADENCE
        acc = new stencil3d_v0_wrapper("stencil3d_v0_wrapper");
#else
        acc = new stencil3d_v0("stencil3d_v0_wrapper");
#endif
        // Binding ACC
        acc->clk(clk);
        acc->rst(acc_rst);
        acc->dma_read_ctrl(dma_read_ctrl);
        acc->dma_write_ctrl(dma_write_ctrl);
        acc->dma_read_chnl(dma_read_chnl);
        acc->dma_write_chnl(dma_write_chnl);
        acc->conf_info(conf_info);
        acc->conf_done(conf_done);
        acc->acc_done(acc_done);
        acc->debug(debug);

        /* <<--params-default-->> */
        row_size = 16;
        height_size = 16;
        coef_1 = -1;
        col_size = 32;
        coef_0 = 6;
        stencil_n = 2;
    }

    // Processes

    // Configure accelerator
    void config_proc();

    // Load internal memory
    void load_memory();

    // Dump internal memory
    void dump_memory();

    // Validate accelerator results
    int validate();

    // Accelerator-specific data
    /* <<--params-->> */
    int32_t row_size;
    int32_t height_size;
    int32_t coef_1;
    int32_t col_size;
    int32_t coef_0;
    int32_t stencil_n;

    uint32_t in_words_adj;
    uint32_t out_words_adj;
    uint32_t in_size;
    uint32_t out_size;
    TYPE *in;
    TYPE *out;
    TYPE *gold;

    // Other Functions
};

#endif // __SYSTEM_HPP__
