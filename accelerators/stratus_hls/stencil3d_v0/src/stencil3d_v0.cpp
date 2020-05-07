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
    int32_t stencil_n;
    int32_t load_cnt;

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
        stencil_n = config.stencil_n;
	load_cnt = 0;
        map_adj = PLM_IN_WORD / (2 * row_size * col_size);
	
	cout << "DEBUG_Info: DMA_WIDTH = " << DMA_WIDTH << endl;

        if (PLM_IN_WORD < row_size * col_size * height_size) {
		rem_fwd = PLM_IN_WORD - map_adj * row_size * col_size;
	}
	else {
		rem_fwd = PLM_IN_WORD;
	}
    }

    // Load
    {
        HLS_PROTO("load-dma");
        wait();

        bool ping = true;
        uint32_t offset = 0;

        // Batching
        for (uint16_t b = 0; b < stencil_n; b++)
        {
            wait();
#if (DMA_WORD_PER_BEAT == 0)
            uint32_t length = row_size*col_size*height_size;
#else
            uint32_t length = round_up(row_size*col_size*height_size, DMA_WORD_PER_BEAT);
#endif
            // Chunking
            for (int rem = length; rem > 0; rem -= rem_fwd)
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
		//cout << "DEBUG_INFO: load offset = " << offset / DMA_WORD_PER_BEAT << endl;
		//cout << "DEBUG_INFO: load len = " << len / DMA_WORD_PER_BEAT << endl;
		//cout << "DEBUG_INFO: load rem = " << rem << endl;
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
                    //HLS_BREAK_DEP(plm_in_ping);
                    //HLS_BREAK_DEP(plm_in_pong);

                    sc_dt::sc_bv<DMA_WIDTH> dataBv;

                    dataBv = this->dma_read_chnl.get();
                    wait();

                    // Write to PLM (all DMA_WORD_PER_BEAT words in one cycle)
                    for (uint16_t k = 0; k < DMA_WORD_PER_BEAT; k++)
                    {
                        HLS_LOAD_PLM_WRITE;
                        if (ping) {
                            plm_in_ping[i + k] = dataBv.range((k+1) * DATA_WIDTH - 1, k * DATA_WIDTH).to_int64();
			}
                        else  {
			    plm_in_pong[i + k] = dataBv.range((k+1) * DATA_WIDTH - 1, k * DATA_WIDTH).to_int64();
			wait();
			    //cout << "DEBUG Info: plm_in_pong = " << plm_in_pong[i + k] << endl;
			}
                    }
                }
#endif
                this->load_compute_handshake();
                ping = !ping;
		load_cnt++;
		if (rem < PLM_IN_WORD) rem = 0;
                ////cout << "DEBUG INFO: SRC load cnt = " << load_cnt << endl;
		
            }
        }
	
        sc_time end_time_compute = sc_time_stamp();
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
    int32_t stencil_n;
    int32_t std_cnt;

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
        stencil_n = config.stencil_n;
	std_cnt = 0;

    }

    // Store
    {
        HLS_PROTO("store-dma");
        wait();

        bool ping = true;
#if (DMA_WORD_PER_BEAT == 0)
        uint32_t store_offset = (row_size*col_size*height_size) * stencil_n;
#else
	uint32_t store_offset;
        if (PLM_OUT_WORD < row_size * col_size * height_size) {
	    // Calculation of offset of output memory address
            store_offset = (row_size * col_size * height_size / rem_fwd - 1) * PLM_IN_WORD + row_size * col_size * height_size - (row_size * col_size * height_size / rem_fwd - 1) * rem_fwd;
	    //cout << "DEBUG_INFO: store offset initial = " << store_offset << endl;
	}
	else	{
            store_offset = round_up(row_size*col_size*height_size, DMA_WORD_PER_BEAT) * stencil_n;
	}
#endif
        uint32_t offset = store_offset;

        wait();
        // Batching
        for (uint16_t b = 0; b < stencil_n; b++)
        {
            wait();
#if (DMA_WORD_PER_BEAT == 0)
            uint32_t length = row_size*col_size*height_size;
#else
            uint32_t length = round_up(row_size*col_size*height_size, DMA_WORD_PER_BEAT);
#endif
            // Chunking
            for (int rem = length; rem > 0; rem -= rem_fwd)
            {

                this->store_compute_handshake();
	        //cout << "DEBUG_INFO: store compute handshake " << endl;

                // Configure DMA transaction
                uint32_t len = rem > PLM_OUT_WORD ? PLM_OUT_WORD : rem;
#if (DMA_WORD_PER_BEAT == 0)
                // data word is wider than NoC links
                dma_info_t dma_info(offset * DMA_BEAT_PER_WORD, len * DMA_BEAT_PER_WORD, DMA_SIZE);
#else
                dma_info_t dma_info(offset / DMA_WORD_PER_BEAT, len / DMA_WORD_PER_BEAT, DMA_SIZE);
#endif
		cout << "DEBUG_INFO: DMA_SIZE = " << DMA_SIZE << endl;
                offset += len;
		//cout << "DEBUG_INFO: store offset = " << offset / DMA_WORD_PER_BEAT << endl;
		//cout << "DEBUG_INFO: store len = " << len / DMA_WORD_PER_BEAT << endl;
		//cout << "DEBUG_INFO: store rem = " << rem << endl;
			

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

                    // Read from PLM
                    wait();
                    for (uint16_t k = 0; k < DMA_WORD_PER_BEAT; k++)
                    {
                        HLS_STORE_PLM_READ; if (ping){
                            dataBv.range((k+1) * DATA_WIDTH - 1, k * DATA_WIDTH) = plm_out_ping[i + k];
			    //cout << "DEBUG Info : plm_out_ping ["<< i + k << "]" << " = " << plm_out_ping[i + k] << endl;
			}
                        else {
                            dataBv.range((k+1) * DATA_WIDTH - 1, k * DATA_WIDTH) = plm_out_pong[i + k];
			wait();
			    //cout << "DEBUG Info : plm_out_pong = " << plm_out_pong[i + k] << endl;
			}
                    }
                    this->dma_write_chnl.put(dataBv);
                }
#endif
		//cout << "DEBUG_INFO: store done" << endl;
                ping = !ping;
		if (rem < PLM_IN_WORD) rem = 0;
		std_cnt++;
                ////cout << "DEBUG INFO: SRC store cnt = " << std_cnt << endl;
		 
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
    int32_t stencil_n;
    {
        HLS_PROTO("compute-config");

        cfg.wait_for_config(); // config process
        conf_info_t config = this->conf_info.read();

        // User-defined config code
        /* <<--local-params-->> */
        row_size = config.row_size;
        height_size = config.height_size;
        coef_1 = config.coef_1;
        col_size = config.col_size;
        coef_0 = config.coef_0;
        stencil_n = config.stencil_n;

    }


    // Compute
    bool ping = true;

    {
        for (uint16_t b = 0; b < stencil_n; b++)
        {
            uint32_t in_length = row_size*col_size*height_size;
            uint32_t out_length = row_size*col_size*height_size;
            int out_rem = out_length;


            for (int in_rem = in_length; in_rem > 0; in_rem -= rem_fwd)
            {

                uint32_t in_len  = in_rem  > PLM_IN_WORD  ? PLM_IN_WORD  : in_rem;
                uint32_t out_len = out_rem > PLM_OUT_WORD ? PLM_OUT_WORD : out_rem;

                this->compute_load_handshake();

                // Computing phase implementation v0 
                if (ping) {
			if (PLM_IN_WORD >= in_length) {
			    boundary_fill(in_rem, row_size, col_size, height_size, plm_in_ping, plm_out_ping);
			}
			stencil_compute(in_len, (in_length - in_rem), coef_0, coef_1, row_size, col_size, height_size, plm_in_ping, plm_out_ping);
		}
		else {
			if (PLM_IN_WORD >= in_length) {
			    boundary_fill(in_rem, row_size, col_size, height_size, plm_in_pong, plm_out_pong);
			}
			stencil_compute(in_len, (in_length - in_rem), coef_0, coef_1, row_size, col_size, height_size, plm_in_pong, plm_out_pong);
		}


                out_rem -= PLM_OUT_WORD;
		//cout << "DEBUG Info: Compute ready to handshake" << endl;
                this->compute_store_handshake();
		//cout << "DEBUG Info: Compute handshake complete" << endl;
                ping = !ping;
            }
        }

        // Conclude
        {
            this->process_done();
        }
    }
}
