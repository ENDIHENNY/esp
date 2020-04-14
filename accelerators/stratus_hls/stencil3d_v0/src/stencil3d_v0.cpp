// Copyright (c) 2011-2019 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#include "stencil3d_v0.hpp"
#include "stencil3d_v0_directives.hpp"

// Functions

#include "stencil3d_v0_functions.hpp"
using namespace std;

// Processes

void stencil3d_v0::load_input()
{

    // Reset
    {
        HLS_PROTO("load-reset");

        this->reset_load_input();

        // explicit PLM ports reset if any

        // User-defined reset code

        wait();
    }

    // Config
    /* <<--params-->> */
    int32_t row_size;
    int32_t height_size;
    int32_t coef_1;
    int32_t col_size;
    int32_t coef_0;
    {
        HLS_PROTO("load-config");

        cfg.wait_for_config(); // config process
        conf_info_t config = this->conf_info.read();

        // User-defined config code
        /* <<--local-params-->> */
        row_size = config.row_size;
        height_size = config.height_size;
        coef_1 = config.coef_1;
        col_size = config.col_size;
        coef_0 = config.coef_0;
    }

        //sc_time begin_time = sc_time_stamp();
	//cout << "SHOW ME SHOW ME " << begin_time << endl;
	//printf("Info: accelerator: BEGIN loading memory at %c\n", a[0]);
    // Load
    {
        HLS_PROTO("load-dma");
        wait();

        bool ping = true;
        uint32_t offset = 0;

        // Batching
        for (uint16_t b = 0; b < 1; b++)
        {
            wait();
#if (DMA_WORD_PER_BEAT == 0)
            uint32_t length = row_size*col_size*height_size;
#else
            uint32_t length = round_up(row_size*col_size*height_size, DMA_WORD_PER_BEAT);
#endif
            // Chunking
            for (int rem = length; rem > 0; rem -= PLM_IN_WORD)
            {
                wait();
                // Configure DMA transaction
                uint32_t len = rem > PLM_IN_WORD ? PLM_IN_WORD : rem;
#if (DMA_WORD_PER_BEAT == 0)
                // data word is wider than NoC links
                dma_info_t dma_info(offset * DMA_BEAT_PER_WORD, len * DMA_BEAT_PER_WORD, DMA_SIZE);
#else
                dma_info_t dma_info(offset / DMA_WORD_PER_BEAT, len / DMA_WORD_PER_BEAT, DMA_SIZE);
#endif
                offset += len;

                this->dma_read_ctrl.put(dma_info);

#if (DMA_WORD_PER_BEAT == 0)
                // data word is wider than NoC links
                for (uint16_t i = 0; i < len; i++)
                {
                    sc_dt::sc_bv<DATA_WIDTH> dataBv;

                    for (uint16_t k = 0; k < DMA_BEAT_PER_WORD; k++)
                    {
                        dataBv.range((k+1) * DMA_WIDTH - 1, k * DMA_WIDTH) = this->dma_read_chnl.get();
                        wait();
                    }

                    // Write to PLM
                    if (ping)
                        plm_in_ping[i] = dataBv.to_int64();
                    else
                        plm_in_pong[i] = dataBv.to_int64();
                }
#else
                for (uint16_t i = 0; i < len; i += DMA_WORD_PER_BEAT)
                {
                    HLS_BREAK_DEP(plm_in_ping);
                    HLS_BREAK_DEP(plm_in_pong);

                    sc_dt::sc_bv<DMA_WIDTH> dataBv;

                    dataBv = this->dma_read_chnl.get();
                    wait();

                    // Write to PLM (all DMA_WORD_PER_BEAT words in one cycle)
                    for (uint16_t k = 0; k < DMA_WORD_PER_BEAT; k++)
                    {
                        HLS_UNROLL_SIMPLE;
                        if (ping) {
                            plm_in_ping[i + k] = dataBv.range((k+1) * DATA_WIDTH - 1, k * DATA_WIDTH).to_int64();
			    cout << "DEBUG INFO: plm_in = " << dataBv.range((k+1) * DATA_WIDTH - 1, k * DATA_WIDTH) << endl;
			    cout << "DEBUG INFO: DATA_WIDTH = " << DATA_WIDTH << endl;
			}
                        else 
			    plm_in_pong[i + k] = dataBv.range((k+1) * DATA_WIDTH - 1, k * DATA_WIDTH).to_int64();
                    }
                }
#endif
                this->load_compute_handshake();
                ping = !ping;
            }
        }
	
        sc_time end_time_compute = sc_time_stamp();
	//float b = end_time_compute.to_double();
	//printf("Info: accelerator: END loading memory at %f ps\n", b);
    }

    // Conclude
    {
        this->process_done();
    }
}



void stencil3d_v0::store_output()
{
    // Reset
    {
        //sc_time begin_time = sc_time_stamp();
	//printf("Info: accelerator: BEGIN storing output at %f ps\n", begin_time.to_double());
        HLS_PROTO("store-reset");

        this->reset_store_output();

        // explicit PLM ports reset if any

        // User-defined reset code

        wait();
    }

    // Config
    /* <<--params-->> */
    int32_t row_size;
    int32_t height_size;
    int32_t coef_1;
    int32_t col_size;
    int32_t coef_0;
    {
        HLS_PROTO("store-config");

        cfg.wait_for_config(); // config process
        conf_info_t config = this->conf_info.read();

        // User-defined config code
        /* <<--local-params-->> */
        row_size = config.row_size;
        height_size = config.height_size;
        coef_1 = config.coef_1;
        col_size = config.col_size;
        coef_0 = config.coef_0;
    }

    // Store
    {
        HLS_PROTO("store-dma");
        wait();

        bool ping = true;
#if (DMA_WORD_PER_BEAT == 0)
        uint32_t store_offset = (row_size*col_size*height_size) * 1;
#else
        uint32_t store_offset = round_up(row_size*col_size*height_size, DMA_WORD_PER_BEAT) * 1;
#endif
        uint32_t offset = store_offset;

        wait();
        // Batching
        for (uint16_t b = 0; b < 1; b++)
        {
            wait();
#if (DMA_WORD_PER_BEAT == 0)
            uint32_t length = row_size*col_size*height_size;
#else
            uint32_t length = round_up(row_size*col_size*height_size, DMA_WORD_PER_BEAT);
#endif
            // Chunking
            for (int rem = length; rem > 0; rem -= PLM_OUT_WORD)
            {

                this->store_compute_handshake();

                // Configure DMA transaction
                uint32_t len = rem > PLM_OUT_WORD ? PLM_OUT_WORD : rem;
#if (DMA_WORD_PER_BEAT == 0)
                // data word is wider than NoC links
                dma_info_t dma_info(offset * DMA_BEAT_PER_WORD, len * DMA_BEAT_PER_WORD, DMA_SIZE);
#else
                dma_info_t dma_info(offset / DMA_WORD_PER_BEAT, len / DMA_WORD_PER_BEAT, DMA_SIZE);
#endif
                offset += len;

                this->dma_write_ctrl.put(dma_info);

#if (DMA_WORD_PER_BEAT == 0)
                // data word is wider than NoC links
                for (uint16_t i = 0; i < len; i++)
                {
                    // Read from PLM
                    sc_dt::sc_int<DATA_WIDTH> data;
                    wait();
                    if (ping)
                        data = plm_out_ping[i];
                    else
                        data = plm_out_pong[i];
                    sc_dt::sc_bv<DATA_WIDTH> dataBv(data);

                    uint16_t k = 0;
                    for (k = 0; k < DMA_BEAT_PER_WORD - 1; k++)
                    {
                        this->dma_write_chnl.put(dataBv.range((k+1) * DMA_WIDTH - 1, k * DMA_WIDTH));
                        wait();
                    }
                    // Last beat on the bus does not require wait(), which is
                    // placed before accessing the PLM
                    this->dma_write_chnl.put(dataBv.range((k+1) * DMA_WIDTH - 1, k * DMA_WIDTH));
                }
#else
                for (uint16_t i = 0; i < len; i += DMA_WORD_PER_BEAT)
                {
                    sc_dt::sc_bv<DMA_WIDTH> dataBv;
	    	    //cout << "DEBUG::len = " << len << endl;

                    // Read from PLM
                    wait();
                    for (uint16_t k = 0; k < DMA_WORD_PER_BEAT; k++)
                    {
                        HLS_UNROLL_SIMPLE;
                        if (ping)
                            dataBv.range((k+1) * DATA_WIDTH - 1, k * DATA_WIDTH) = plm_out_ping[i + k];
                        else
                            dataBv.range((k+1) * DATA_WIDTH - 1, k * DATA_WIDTH) = plm_out_pong[i + k];
                    }
                    this->dma_write_chnl.put(dataBv);
                }
#endif
        	//sc_time end_time_store = sc_time_stamp();
		//printf("Info: accelerator: END storing output at %f ps\n", end_time_store.to_double());
                ping = !ping;
            }
        }
    }

    // Conclude
    {
        this->accelerator_done();
        this->process_done();
    }
}


void stencil3d_v0::compute_kernel()
{
    // Reset
    {
        HLS_PROTO("compute-reset");

        this->reset_compute_kernel();

        // explicit PLM ports reset if any

        // User-defined reset code

        wait();
    }

    // Config
    /* <<--params-->> */
    int32_t row_size;
    int32_t height_size;
    int32_t coef_1;
    int32_t col_size;
    int32_t coef_0;
    {
        HLS_PROTO("compute-config");
        //sc_time begin_time = sc_time_stamp();
	//printf("Info: accelerator: BEGIN Compute Config at %f ps\n", begin_time.to_double());

        cfg.wait_for_config(); // config process
        conf_info_t config = this->conf_info.read();

        // User-defined config code
        /* <<--local-params-->> */
        row_size = config.row_size;
        height_size = config.height_size;
        coef_1 = config.coef_1;
        col_size = config.col_size;
        coef_0 = config.coef_0;
        //sc_time end_time_config = sc_time_stamp();
	//printf("Info: accelerator: END Compute Config at %f ps\n", end_time_config.to_double());
    }


    // Compute
    bool ping = true;
    //sc_time begin_time = sc_time_stamp();
    //printf("Info: accelerator: BEGIN Compute at %f ps\n", begin_time.to_double());

    {
        for (uint16_t b = 0; b < 1; b++)
        {
            uint32_t in_length = row_size*col_size*height_size;
            uint32_t out_length = row_size*col_size*height_size;
            int out_rem = out_length;
	    cout << "DEBUG::in_length = " << in_length << endl;
	    cout << "DEBUG::out_length = " << out_length << endl;
	    cout << "DEBUG::out_rem = " << out_rem << endl;
	    cout << "DEBUG::PLM_IN_WORD = " << PLM_IN_WORD << endl;
	    cout << "DEBUG::PLM_OUT_WORD = " << PLM_OUT_WORD << endl;
	    cout << "DEBUG::DMA_SIZE = " << DMA_SIZE << endl;


            for (int in_rem = in_length; in_rem > 0; in_rem -= PLM_IN_WORD)
            {

                uint32_t in_len  = in_rem  > PLM_IN_WORD  ? PLM_IN_WORD  : in_rem;
                uint32_t out_len = out_rem > PLM_OUT_WORD ? PLM_OUT_WORD : out_rem;

                this->compute_load_handshake();

                // Computing phase implementation v0 
                if (ping) {
			boundary_fill(row_size, col_size, height_size, plm_in_ping, plm_out_ping);
			stencil_compute(coef_0, coef_1, row_size, col_size, height_size, plm_in_ping, plm_out_ping);
		}
		else {
			boundary_fill(row_size, col_size, height_size, plm_in_pong, plm_out_pong);
			stencil_compute(coef_0, coef_1, row_size, col_size, height_size, plm_in_pong, plm_out_pong);
		}

                // for (int i = 0; i < in_len; i++) {
                //    if (ping)
                //        plm_out_ping[i] = plm_in_ping[i];
                //    else
                //        plm_out_pong[i] = plm_in_pong[i];
                // }

                out_rem -= PLM_OUT_WORD;
                this->compute_store_handshake();
                ping = !ping;
    		//sc_time end_time_compute = sc_time_stamp();
    		//printf("Info: accelerator: END Compute at %f ps\n", end_time_compute.to_double());
            }
        }

        // Conclude
        {
            this->process_done();
        }
    }
}
