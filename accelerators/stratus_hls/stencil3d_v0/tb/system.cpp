// Copyright (c) 2011-2019 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#include <sstream>
#include <time.h>
#include "system.hpp"
#include <random>

// Helper random generator - for both int & float
#if (TYPEDEF == 0)
    static std::uniform_int_distribution<TYPE> *dis;
#elif (TYPEDEF == 1)
    static std::uniform_real_distribution<TYPE> *dis;
#endif

static std::random_device rd;
static std::mt19937 *gen;

static void init_random_distribution(void)
{
    // Different type of LO & HO needs different format of the value

    gen = new std::mt19937(rd());
    #if (TYPEDEF == 1)
        const TYPE LO = 0.0;
        const TYPE HI = 100.0;
	// dis = new std::uniform_real_distribution<TYPE>(LO, HI);
	default_random_engine generator;
	dis = new std::uniform_real_distribution<TYPE>(LO, HI);
    #elif (TYPEDEF == 0)    
        const TYPE LO = 0;
        const TYPE HI = 100;
	default_random_engine generator;
	dis = new std::uniform_int_distribution<TYPE>(LO, HI);
    #endif
}

static TYPE gen_random_num(void)
{
    return (*dis)(*gen);
}
// Process
void system_t::config_proc()
{

    // Reset
    {
        conf_done.write(false);
        conf_info.write(conf_info_t());
        wait();
    }

    ESP_REPORT_INFO("reset done");
    ESP_REPORT_INFO("load begin");

    // Config
    sc_time begin_time = sc_time_stamp();
    ESP_REPORT_TIME(begin_time, "BEGIN Load - stencil3d_v0");

    load_memory();

    sc_time end_time = sc_time_stamp();
    ESP_REPORT_TIME(end_time, "END Load - stencil3d_v0");
    {
        conf_info_t config;
        // Custom configuration
        /* <<--params-->> */
        config.row_size = row_size;
        config.height_size = height_size;
        config.coef_1 = coef_1;
        config.col_size = col_size;
        config.coef_0 = coef_0;
        config.stencil_n = stencil_n;
        sc_time begin_time = sc_time_stamp();
        ESP_REPORT_TIME(begin_time, "BEGIN Config - stencil3d_v0");

        wait(); conf_info.write(config);
        conf_done.write(true);

        sc_time end_time = sc_time_stamp();
        ESP_REPORT_TIME(end_time, "END Config - stencil3d_v0");
    }

    ESP_REPORT_INFO("config done");

    // Compute
    {
        // Print information about begin time
        sc_time begin_time = sc_time_stamp();
        ESP_REPORT_TIME(begin_time, "BEGIN Compute - stencil3d_v0");

        // Wait the termination of the accelerator
        do { wait(); } while (!acc_done.read());
        debug_info_t debug_code = debug.read();

        // Print information about end time
        sc_time end_time = sc_time_stamp();
        ESP_REPORT_TIME(end_time, "END Compute - stencil3d_v0");

        esc_log_latency(sc_object::basename(), clock_cycle(end_time - begin_time));
        wait(); conf_done.write(false);
    }

    // Validate
    {
        sc_time begin_time = sc_time_stamp();
        ESP_REPORT_TIME(begin_time, "BEGIN Store - stencil3d_v0");

        dump_memory(); // store the output in more suitable data structure if needed

        sc_time end_time = sc_time_stamp();
        ESP_REPORT_TIME(end_time, "END Store - stencil3d_v0");
        // check the results with the golden model
        if (validate())
        {
            ESP_REPORT_ERROR("validation failed!");
        } else
        {
            ESP_REPORT_INFO("validation passed!");
        }
    }

    // Conclude
    {
        sc_stop();
    }
}

// Functions
void system_t::load_memory()
{
    ESP_REPORT_INFO("DEBUG Info: Start loading memory\n");
    // Optional usage check
#ifdef CADENCE
    if (esc_argc() != 1)
    {
        ESP_REPORT_INFO("usage: %s\n", esc_argv()[0]);
        sc_stop();
    }
#endif

    // Input data and golden output (aligned to DMA_WIDTH makes your life easier)
#if (DMA_WORD_PER_BEAT == 0)
    in_words_adj = row_size*col_size*height_size;
    out_words_adj = row_size*col_size*height_size;
#else
    in_words_adj = round_up(row_size*col_size*height_size, DMA_WORD_PER_BEAT);
    out_words_adj = round_up(row_size*col_size*height_size, DMA_WORD_PER_BEAT);
#endif

    in_size = in_words_adj * stencil_n;
    out_size = out_words_adj * stencil_n;
    map_adj = PLM_IN_WORD / (2 * row_size * col_size);
    fwd = PLM_IN_WORD - map_adj * row_size * col_size;
    cnt = 0;
    mem_idx = 0;
    ESP_REPORT_INFO("DEBUG Info: Start initializing input\n");

    in = new TYPE[in_size];

    init_random_distribution();
    for (int i = 0; i < stencil_n; i++) {
        for (int j = 0; j < row_size*col_size*height_size; j++) {
            in[i * in_words_adj + j] = gen_random_num();
	}
    }
    ESP_REPORT_INFO("DEBUG Info: Start generating golden output\n");

    // Compute golden output
    gold = new TYPE[out_size];
    int32_t i = 0;
    int32_t j = 0; 
    int32_t k = 0;
    int32_t l = 0;
    ESP_REPORT_INFO("start boundary filling\n");

    for(l=0; l<stencil_n; l++) {

	    for(j=0; j<col_size; j++) {
		for(k=0; k<row_size; k++) {
		    int32_t index0 = l * out_words_adj + k + row_size * j; 
		    int32_t index1 = l * out_words_adj + k + row_size * (j + col_size * (height_size-1)); 
		    gold[index0] = in[index0];
		    gold[index1] = in[index1];

		      }
	    }
	    for(i=1; i<height_size-1; i++) {
		for(k=0; k<row_size; k++) {
		    int32_t index0 = l * out_words_adj + k + row_size * col_size * i;
		    int32_t index1 = l * out_words_adj + k + row_size * ((col_size-1) + col_size*i);
		    gold[index0] = in[index0];
		    gold[index1] = in[index1];
		}
	    }
	    for(i=1; i<height_size-1; i++) {
		for(j=1; j<col_size-1; j++) {
		    int32_t index0 = l * out_words_adj + row_size * (j + col_size * i);
		    int32_t index1 = l * out_words_adj + row_size-1 + row_size * (j + col_size * i);
		    gold[index0] = in[index0];
		    gold[index1] = in[index1];
		}
	    }
	    ESP_REPORT_INFO("Finish boundary filling\n");

	    // Stencil computation
	    for(i = 1; i < height_size - 1; i++){
		for(j = 1; j < col_size - 1; j++){
		    for(k = 1; k < row_size - 1; k++){
			int32_t index0 = l * out_words_adj + k + row_size * (j + col_size * i);
			int32_t index1 = l * out_words_adj + k + row_size * (j + col_size * (i + 1));
			int32_t index2 = l * out_words_adj + k + row_size * (j + col_size * (i - 1));
			int32_t index3 = l * out_words_adj + k + row_size * (j + 1 + col_size * i);
			int32_t index4 = l * out_words_adj + k + row_size * (j - 1 + col_size * i);
			int32_t index5 = l * out_words_adj + k + 1 + row_size * (j + col_size * i);
			int32_t index6 = l * out_words_adj + k - 1 + row_size * (j + col_size * i);
			
			TYPE sum0 = in[index0];
			TYPE sum1 = in[index1] + in[index2] + in[index3] + 
			       in[index4] + in[index5] + in[index6];
			TYPE mul0 = sum0 * coef_0;
			TYPE mul1 = sum1 * coef_1;

			gold[index0] = mul0 + mul1;
			    }
		}
	    }
    }



    ESP_REPORT_INFO("DEBUG Info: Completed generating golden output\n");
    ESP_REPORT_INFO("DEBUG Info: Start memory initializing\n");

    // Memory initialization:
#if (TYPEDEF == 0)
	#if (DMA_WORD_PER_BEAT == 0)
	    for (int i = 0; i < in_size; i++)  {
		sc_dt::sc_bv<DATA_WIDTH> data_bv(in[i]);
		for (int j = 0; j < DMA_BEAT_PER_WORD; j++)
		    mem[DMA_BEAT_PER_WORD * i + j] = data_bv.range((j + 1) * DMA_WIDTH - 1, j * DMA_WIDTH);
	    }
	#else
	    for (int i = 0; i < in_size / DMA_WORD_PER_BEAT; i++)  {
		sc_dt::sc_bv<DMA_WIDTH> data_bv(in[i]);
		for (int j = 0; j < DMA_WORD_PER_BEAT; j++)
		    data_bv.range((j+1) * DATA_WIDTH - 1, j * DATA_WIDTH) = in[i * DMA_WORD_PER_BEAT + j];
		mem[mem_idx] = data_bv;
		mem_idx++;

		if (PLM_IN_WORD < in_words_adj) {
			if ((i - cnt * fwd / DMA_WORD_PER_BEAT + 1) % (PLM_IN_WORD / DMA_WORD_PER_BEAT) == 0) {
				i = i - (PLM_IN_WORD - fwd) / DMA_WORD_PER_BEAT;
				cnt++;
			}
		}
	    }
	#endif

#elif (TYPEDEF == 1)
	#if (DMA_WORD_PER_BEAT == 0)
	    for (int i = 0; i < in_size; i++)  {
        	sc_dt::sc_bv<DATA_WIDTH> data_bv(fp2bv<FPDATA, WORD_SIZE>(FPDATA(in[i])));
		for (int j = 0; j < DMA_BEAT_PER_WORD; j++) {
		    mem[DMA_BEAT_PER_WORD * i + j] = data_bv.range((j + 1) * DMA_WIDTH - 1, j * DMA_WIDTH);
		}
	    }
	#else
	    cout << "DEBUG Info_tb: DMA_WORD_PER_BEAT = " << DMA_WORD_PER_BEAT << endl;
	    for (int i = 0; i < in_size / DMA_WORD_PER_BEAT; i++)  {
		sc_dt::sc_bv<DMA_WIDTH> data_bv;
		for (int j = 0; j < DMA_WORD_PER_BEAT; j++)
            	    data_bv.range((j+1) * DATA_WIDTH - 1, j * DATA_WIDTH) = fp2bv<FPDATA, WORD_SIZE>(FPDATA(in[i * DMA_WORD_PER_BEAT + j]));

		mem[mem_idx] = data_bv;
		mem_idx++;

		if (PLM_IN_WORD < in_words_adj) {
			if ((i - cnt * fwd / DMA_WORD_PER_BEAT + 1) % (PLM_IN_WORD / DMA_WORD_PER_BEAT) == 0) {
				i = i - (PLM_IN_WORD - fwd) / DMA_WORD_PER_BEAT;
				cnt++;
			}
		}
	    }
	#endif
#endif
    ESP_REPORT_INFO("load memory completed");
}

void system_t::dump_memory()
{
    // Get results from memory
    out = new TYPE[out_size];
    cnt_dump = 0;
    mem_dump_idx = 0;
    uint32_t offset;

    if(PLM_OUT_WORD < row_size * col_size * height_size) {
        offset = (row_size * col_size * height_size / fwd - 1) * PLM_IN_WORD + row_size * col_size * height_size - (row_size * col_size * height_size / fwd - 1) * fwd;
    }
    else {
        offset = in_size;
    }

#if (TYPEDEF == 0)
	#if (DMA_WORD_PER_BEAT == 0)
	    offset = offset * DMA_BEAT_PER_WORD;
	    for (int i = 0; i < out_size; i++)  {
		sc_dt::sc_bv<DATA_WIDTH> data_bv;

		for (int j = 0; j < DMA_BEAT_PER_WORD; j++)
		    data_bv.range((j + 1) * DMA_WIDTH - 1, j * DMA_WIDTH) = mem[offset + DMA_BEAT_PER_WORD * i + j];

		out[i] = data_bv.to_int64();
	    }
	#else
	    offset = offset / DMA_WORD_PER_BEAT;
	    if (PLM_OUT_WORD < row_size * col_size * height_size) {
	    	for (int i = 0; i < out_size / DMA_WORD_PER_BEAT; i++) {
			for (int j = 0; j < DMA_WORD_PER_BEAT; j++) {
			    int out_temp = mem[offset + mem_dump_idx].range((j + 1) * DATA_WIDTH - 1, j * DATA_WIDTH).to_int64();
			    if ( out_temp  ==  INT_MAX ) {
				continue;
  			    }
			    out[i * DMA_WORD_PER_BEAT + j] = out_temp;
			    }

			    mem_dump_idx++;


			    if ((i - cnt_dump * fwd / DMA_WORD_PER_BEAT + 1) % (PLM_OUT_WORD / DMA_WORD_PER_BEAT) == 0) {
				i = i - (PLM_OUT_WORD - fwd) / DMA_WORD_PER_BEAT;
				cnt_dump++;

			    }
		        }


	    }

	    else {
		    for (int i = 0; i < out_size / DMA_WORD_PER_BEAT; i++)
			for (int j = 0; j < DMA_WORD_PER_BEAT; j++)
			    out[i * DMA_WORD_PER_BEAT + j] = mem[offset + i].range((j + 1) * DATA_WIDTH - 1, j * DATA_WIDTH).to_int64();
	    }
	#endif

#elif (TYPEDEF == 1)
	#if (DMA_WORD_PER_BEAT == 0)
	    offset = offset * DMA_BEAT_PER_WORD;
	    for (int i = 0; i < out_size; i++)  {
		sc_dt::sc_bv<DATA_WIDTH> data_bv;

		for (int j = 0; j < DMA_BEAT_PER_WORD; j++)
		    data_bv.range((j + 1) * DMA_WIDTH - 1, j * DMA_WIDTH) = mem[offset + DMA_BEAT_PER_WORD * i + j];
		FPDATA out_fx = bv2fp<FPDATA, WORD_SIZE>(data_bv);
		out[i] = (float) out_fx;
	    }
	#else
	    if (PLM_OUT_WORD < row_size * col_size * height_size)	{
		    offset = offset / DMA_WORD_PER_BEAT;
		    for (int i = 0; i < out_size / DMA_WORD_PER_BEAT; i++) {
			for (int j = 0; j < DMA_WORD_PER_BEAT; j++) {
			    FPDATA out_fx = bv2fp<FPDATA, WORD_SIZE>(mem[offset + mem_dump_idx].range((j + 1) * DATA_WIDTH - 1, j * DATA_WIDTH));
			    if ((float) out_fx == (float) 0) {
				continue;
  			    }
			    out[i * DMA_WORD_PER_BEAT + j] = (float) out_fx;
			    }

			    mem_dump_idx++;


			    if ((i - cnt_dump * fwd / DMA_WORD_PER_BEAT + 1) % (PLM_OUT_WORD / DMA_WORD_PER_BEAT) == 0) {
				i = i - (PLM_OUT_WORD - fwd) / DMA_WORD_PER_BEAT;
				cnt_dump++;
			    }
		        }
	    }
	    else	{
		    offset = offset / DMA_WORD_PER_BEAT;
		    for (int i = 0; i < out_size / DMA_WORD_PER_BEAT; i++) 
			for (int j = 0; j < DMA_WORD_PER_BEAT; j++) {
			    FPDATA out_fx = bv2fp<FPDATA, WORD_SIZE>(mem[offset + i].range((j + 1) * DATA_WIDTH - 1, j * DATA_WIDTH));
			    out[i * DMA_WORD_PER_BEAT + j] = (float) out_fx;
			}

	    }
	#endif
#endif
	 
   ESP_REPORT_INFO("dump memory completed");
}

int system_t::validate()
{
    // Check for mismatches
    uint32_t errors = 0;
    const float ERR_TH = 0.05;

    for (int i = 0; i < stencil_n; i++)
        for (int j = 0; j < row_size*col_size*height_size; j++){
            if ((fabs(gold[i * out_words_adj + j] - out[i * out_words_adj + j]) / fabs(gold[i * out_words_adj + j])) > ERR_TH) {
                errors++;
	        cout << "DEBUG Info: out[" << i * out_words_adj + j <<"]" << " = " <<  out[i * out_words_adj + j] <<endl;
	        cout << "DEBUG Info: gold[" << i * out_words_adj + j <<"]" << " = " <<  gold[i * out_words_adj + j] <<endl;
 	    }
		
	}


    ESP_REPORT_INFO("Relative error > %.02f for %d output values out of %d\n", ERR_TH, errors, row_size * col_size * height_size);

    delete [] in;
    delete [] out;
    delete [] gold;

    return errors;
}
