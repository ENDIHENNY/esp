/* Copyright (c) 2011-2019 Columbia University, System Level Design Group */
/* SPDX-License-Identifier: Apache-2.0 */

#ifndef __riscv
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#endif

#include <esp_accelerator.h>
#include <esp_probe.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
//#include <random>

typedef int32_t token_t;

static unsigned DMA_WORD_PER_BEAT(unsigned _st)
{
        return (sizeof(void *) / _st);
}


#define SLD_STENCIL3D_V0 0x045
#define DEV_NAME "sld,stencil3d_v0"

/* <<--params-->> */
const int32_t row_size = 16;
const int32_t height_size = 32;
const int32_t coef_1 = -1;
const int32_t col_size = 16;
const int32_t coef_0 = 6;
const int32_t stencil_n = 1;

static unsigned in_words_adj;
static unsigned out_words_adj;
static unsigned in_len;
static unsigned out_len;
static unsigned in_size;
static unsigned out_size;
static unsigned out_offset;
static unsigned mem_size;

/* Size of the contiguous chunks for scatter/gather */
#define CHUNK_SHIFT 20
#define CHUNK_SIZE BIT(CHUNK_SHIFT)
#define NCHUNK(_sz) ((_sz % CHUNK_SIZE == 0) ?		\
			(_sz / CHUNK_SIZE) :		\
			(_sz / CHUNK_SIZE) + 1)

/* User defined registers */
/* <<--regs-->> */
#define STENCIL3D_V0_ROW_SIZE_REG 0x50
#define STENCIL3D_V0_HEIGHT_SIZE_REG 0x4c
#define STENCIL3D_V0_COEF_1_REG 0x48
#define STENCIL3D_V0_COL_SIZE_REG 0x44
#define STENCIL3D_V0_COEF_0_REG 0x40
#define STENCIL3D_V0_STENCIL_N_REG 0x54

//static std::uniform_real_distribution<float> *dis;
//static std::random_device rd;
//static std::mt19937 *gen;

//static void init_random_distribution(void)
//{
//    // Different type of LO & HO needs different format of the value
//
//    gen = new std::mt19937(rd());
//    const float LO = 0.0;
//    const float HI = 100.0;
//    default_random_engine generator;
//    dis = new std::uniform_real_distribution<float>(LO, HI);
//    //const int LO = 0;
//    //const int HI = 100;
//    //default_random_engine generator;
//    //dis = new std::uniform_int_distribution<int>(LO, HI);
//}



static float gen_random_num(float min, float max)
{
    return (max - min) * ((((float) rand()) / (float) RAND_MAX) + min);
}


static int validate_buf(token_t *out, token_t *gold)
{
    int i;
    int j;

    unsigned errors = 0;
    const float ERR_TH = 0.05;

    for (i = 0; i < stencil_n; i++)
        for (j = 0; j < row_size*col_size*height_size; j++){
            if ((fabs(gold[i * out_words_adj + j] - out[i * out_words_adj + j]) / fabs(gold[i * out_words_adj + j])) > ERR_TH) {
                errors++;
 	    }
		
	}

    return errors;
}

static void init_buf (token_t *in, token_t * gold)
{

    //init_random_distribution();

    int i;
    int j;
    const float min = 0.0;
    const float max = 100.0;

    for (i = 0; i < stencil_n; i++) {
        for (j = 0; j < row_size*col_size*height_size; j++) {
            in[i * in_words_adj + j] = gen_random_num(min, max);
	    }
    }

    // Compute golden output
    int k = 0;
    int l = 0;

    for(l=0; l<stencil_n; l++) {

	    for(j=0; j<col_size; j++) {
		for(k=0; k<row_size; k++) {
		    int index0 = l * out_words_adj + k + row_size * j; 
		    int index1 = l * out_words_adj + k + row_size * (j + col_size * (height_size-1)); 
		    gold[index0] = in[index0];
		    gold[index1] = in[index1];

		      }
	    }
	    for(i=1; i<height_size-1; i++) {
		for(k=0; k<row_size; k++) {
		    int index0 = l * out_words_adj + k + row_size * col_size * i;
		    int index1 = l * out_words_adj + k + row_size * ((col_size-1) + col_size*i);
		    gold[index0] = in[index0];
		    gold[index1] = in[index1];
		}
	    }
	    for(i=1; i<height_size-1; i++) {
		for(j=1; j<col_size-1; j++) {
		    int index0 = l * out_words_adj + row_size * (j + col_size * i);
		    int index1 = l * out_words_adj + row_size-1 + row_size * (j + col_size * i);
		    gold[index0] = in[index0];
		    gold[index1] = in[index1];
		}
	    }

	    // Stencil computation
	    for(i = 1; i < height_size - 1; i++){
		for(j = 1; j < col_size - 1; j++){
		    for(k = 1; k < row_size - 1; k++){
			int index0 = l * out_words_adj + k + row_size * (j + col_size * i);
			int index1 = l * out_words_adj + k + row_size * (j + col_size * (i + 1));
			int index2 = l * out_words_adj + k + row_size * (j + col_size * (i - 1));
			int index3 = l * out_words_adj + k + row_size * (j + 1 + col_size * i);
			int index4 = l * out_words_adj + k + row_size * (j - 1 + col_size * i);
			int index5 = l * out_words_adj + k + 1 + row_size * (j + col_size * i);
			int index6 = l * out_words_adj + k - 1 + row_size * (j + col_size * i);
			
			float sum0 = in[index0];
			float sum1 = in[index1] + in[index2] + in[index3] + 
			       in[index4] + in[index5] + in[index6];
			float mul0 = sum0 * coef_0;
			float mul1 = sum1 * coef_1;

			gold[index0] = mul0 + mul1;
			    }
		}
    	}
    }

}
//	int i;
//	int j;
//
//	for (i = 0; i < 1; i++)
//		for (j = 0; j < row_size*col_size*height_size; j++)
//			in[i * in_words_adj + j] = (token_t) j;
//
//	for (i = 0; i < 1; i++)
//		for (j = 0; j < row_size*col_size*height_size; j++)
//			gold[i * out_words_adj + j] = (token_t) j;
//}


int main(int argc, char * argv[])
{
	int i;
	int n;
	int ndev;
	struct esp_device *espdevs;
	struct esp_device *dev;
	unsigned done;
	unsigned **ptable;
	token_t *mem;
	token_t *gold;
	unsigned errors = 0;

	if (DMA_WORD_PER_BEAT(sizeof(token_t)) == 0) {
		in_words_adj = row_size*col_size*height_size;
		out_words_adj = row_size*col_size*height_size;
	} else {
		in_words_adj = round_up(row_size*col_size*height_size, DMA_WORD_PER_BEAT(sizeof(token_t)));
		out_words_adj = round_up(row_size*col_size*height_size, DMA_WORD_PER_BEAT(sizeof(token_t)));
	}
	in_len = in_words_adj * (1);
	out_len = out_words_adj * (1);
	in_size = in_len * sizeof(token_t);
	out_size = out_len * sizeof(token_t);
	out_offset  = in_len;
	mem_size = (out_offset * sizeof(token_t)) + out_size;


	// Search for the device
#ifndef __riscv
	printf("Scanning device tree... \n");
#else
	print_uart("Scanning device tree... \n");
#endif

	ndev = probe(&espdevs, SLD_STENCIL3D_V0, DEV_NAME);
	if (ndev == 0) {
#ifndef __riscv
		printf("stencil3d_v0 not found\n");
#else
		print_uart("stencil3d_v0 not found\n");
#endif
		return 0;
	}

	for (n = 0; n < ndev; n++) {

		dev = &espdevs[n];

		// Check DMA capabilities
		if (ioread32(dev, PT_NCHUNK_MAX_REG) == 0) {
#ifndef __riscv
			printf("  -> scatter-gather DMA is disabled. Abort.\n");
#else
			print_uart("  -> scatter-gather DMA is disabled. Abort.\n");
#endif
			return 0;
		}

		if (ioread32(dev, PT_NCHUNK_MAX_REG) < NCHUNK(mem_size)) {
#ifndef __riscv
			printf("  -> Not enough TLB entries available. Abort.\n");
#else
			print_uart("  -> Not enough TLB entries available. Abort.\n");
#endif
			return 0;
		}

		// Allocate memory
		gold = aligned_malloc(out_size);
		mem = aligned_malloc(mem_size);
#ifndef __riscv
		printf("  memory buffer base-address = %p\n", mem);
#else
		print_uart("  memory buffer base-address = "); print_uart_addr((uintptr_t) mem); print_uart("\n");
#endif
		// Alocate and populate page table
		ptable = aligned_malloc(NCHUNK(mem_size) * sizeof(unsigned *));
		for (i = 0; i < NCHUNK(mem_size); i++)
			ptable[i] = (unsigned *) &mem[i * (CHUNK_SIZE / sizeof(token_t))];
#ifndef __riscv
		printf("  ptable = %p\n", ptable);
		printf("  nchunk = %lu\n", NCHUNK(mem_size));
#else
		print_uart("  ptable = "); print_uart_addr((uintptr_t) ptable); print_uart("\n");
		print_uart("  nchunk = "); print_uart_int(NCHUNK(mem_size)); print_uart("\n");
#endif

#ifndef __riscv
		printf("  Generate input...\n");
#else
		print_uart("  Generate input...\n");
#endif
		init_buf(mem, gold);

		// Pass common configuration parameters

		iowrite32(dev, SELECT_REG, ioread32(dev, DEVID_REG));
		iowrite32(dev, COHERENCE_REG, ACC_COH_NONE);

#ifndef __sparc
		iowrite32(dev, PT_ADDRESS_REG, (unsigned long long) ptable);
#else
		iowrite32(dev, PT_ADDRESS_REG, (unsigned) ptable);
#endif
		iowrite32(dev, PT_NCHUNK_REG, NCHUNK(mem_size));
		iowrite32(dev, PT_SHIFT_REG, CHUNK_SHIFT);

		// Use the following if input and output data are not allocated at the default offsets
		iowrite32(dev, SRC_OFFSET_REG, 0x0);
		iowrite32(dev, DST_OFFSET_REG, 0x0);

		// Pass accelerator-specific configuration parameters
		/* <<--regs-config-->> */
		iowrite32(dev, STENCIL3D_V0_ROW_SIZE_REG, row_size);
		iowrite32(dev, STENCIL3D_V0_HEIGHT_SIZE_REG, height_size);
		iowrite32(dev, STENCIL3D_V0_COEF_1_REG, coef_1);
		iowrite32(dev, STENCIL3D_V0_COL_SIZE_REG, col_size);
		iowrite32(dev, STENCIL3D_V0_COEF_0_REG, coef_0);

		// Flush (customize coherence model here)
		esp_flush(ACC_COH_NONE);

		// Start accelerators
#ifndef __riscv
		printf("  Start stencil3d...\n");
#else
		print_uart("  Start stencil3d...\n");
#endif
		iowrite32(dev, CMD_REG, CMD_MASK_START);

		// Wait for completion
		done = 0;
		while (!done) {
			done = ioread32(dev, STATUS_REG);
			done &= STATUS_MASK_DONE;
		}
		iowrite32(dev, CMD_REG, 0x0);

#ifndef __riscv
		printf("  Done\n");
		printf("  validating stencil3d...\n");
#else
		print_uart("  Done\n");
		print_uart("  validating stencil3d...\n");
#endif

		/* Validation */
		errors = validate_buf(&mem[out_offset], gold);
#ifndef __riscv
		if (errors)
			printf("  ... FAIL\n");
		else
			printf("  ... PASS\n");
#else
		if (errors)
			print_uart("  ... FAIL\n");
		else
			print_uart("  ... PASS\n");
#endif

		aligned_free(ptable);
		aligned_free(mem);
		aligned_free(gold);
	}

	return 0;
}
