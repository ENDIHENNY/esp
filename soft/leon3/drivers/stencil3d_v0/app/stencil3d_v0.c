#include "libesp.h"
#include "cfg.h"
#include <stdlib.h>
#include <time.h>
#define MAX 1000
#define MIN 1

static unsigned in_words_adj;
static unsigned out_words_adj;
static unsigned in_len;
static unsigned out_len;
static unsigned in_size;
static unsigned out_size;
static unsigned out_offset;
static unsigned size;

/* User-defined code */
static int validate_buffer(token_t *out, token_t *gold)
{
	int i;
	int j;
	unsigned errors = 0;

	for (i = 0; i < 1; i++)
		for (j = 0; j < row_size*col_size*height_size; j++)
			if (gold[i * out_words_adj + j] != out[i * out_words_adj + j])
				errors++;

	return errors;
}


/* User-defined code */
static void init_buffer(token_t *in, token_t * gold)
{
    int i;
    int j;
    int k;
    srand(time(0));
    for (i = 0; i < 1; i++) {
	for (j = 0; j < row_size*col_size*height_size; j++) {
		in[i * in_words_adj + j] = rand() % (MAX-MIN) + MIN;
	}

    }
	
    for(j=0; j<col_size; j++) {
        for(k=0; k<row_size; k++) {
	    int32_t index0 = k + row_size * j; 
 	    int32_t index1 = k + row_size * (j + col_size * (height_size-1)); 
	    gold[index0] = in[index0];
 	    gold[index1] = in[index1];

              }
    }
    for(i=1; i<height_size-1; i++) {
        for(k=0; k<row_size; k++) {
	    int32_t index0 = k + row_size * col_size * i;
	    int32_t index1 = k + row_size * ((col_size-1) + col_size*i);
	    gold[index0] = in[index0];
	    gold[index1] = in[index1];
        }
    }
    for(i=1; i<height_size-1; i++) {
        for(j=1; j<col_size-1; j++) {
	    int32_t index0 = row_size * (j + col_size * i);
	    int32_t index1 = row_size-1 + row_size * (j + col_size * i);
	    gold[index0] = in[index0];
	    gold[index1] = in[index1];
        }
    }

    int32_t sum0 = 0;
    int32_t sum1 = 0;
    int32_t mul0 = 0;
    int32_t mul1 = 0;

    // Stencil computation
    for(i = 1; i < height_size - 1; i++){
        for(j = 1; j < col_size - 1; j++){
            for(k = 1; k < row_size - 1; k++){
		int32_t index0 = k + row_size * (j + col_size * i);
		int32_t index1 = k + row_size * (j + col_size * (i + 1));
		int32_t index2 = k + row_size * (j + col_size * (i - 1));
		int32_t index3 = k + row_size * (j + 1 + col_size * i);
		int32_t index4 = k + row_size * (j - 1 + col_size * i);
		int32_t index5 = k + 1 + row_size * (j + col_size * i);
		int32_t index6 = k - 1 + row_size * (j + col_size * i);
		
		int32_t sum0 = in[index0];
		int32_t sum1 = in[index1] + in[index2] + in[index3] + 
		       in[index4] + in[index5] + in[index6];
		int32_t mul0 = sum0 * coef_0;
                int32_t mul1 = sum1 * coef_1;
		gold[index0] = mul0 + mul1;

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
}


/* User-defined code */
static void init_parameters()
{
	if (DMA_WORD_PER_BEAT(sizeof(token_t)) == 0) {
		in_words_adj = row_size*col_size*height_size;
		out_words_adj = row_size*col_size*height_size;
	} else {
		in_words_adj = round_up(row_size*col_size*height_size, DMA_WORD_PER_BEAT(sizeof(token_t)));
		out_words_adj = round_up(row_size*col_size*height_size, DMA_WORD_PER_BEAT(sizeof(token_t)));
	}
	in_len = in_words_adj * (1);
	out_len =  out_words_adj * (1);
	in_size = in_len * sizeof(token_t);
	out_size = out_len * sizeof(token_t);
	out_offset = in_len;
	size = (out_offset * sizeof(token_t)) + out_size;
}


int main(int argc, char **argv)
{
	int errors;

	token_t *gold;
	token_t *buf;

	init_parameters();

	buf = (token_t *) esp_alloc(size);
	gold = malloc(out_size);

	init_buffer(buf, gold);

	printf("\n====== %s ======\n\n", cfg_000[0].devname);
	/* <<--print-params-->> */
	printf("  .row_size = %d\n", row_size);
	printf("  .height_size = %d\n", height_size);
	printf("  .coef_1 = %d\n", coef_1);
	printf("  .col_size = %d\n", col_size);
	printf("  .coef_0 = %d\n", coef_0);
	printf("\n  ** START **\n");

	esp_run(cfg_000, NACC);

	printf("\n  ** DONE **\n");

	errors = validate_buffer(&buf[out_offset], gold);

	free(gold);
	esp_cleanup();

	if (!errors)
		printf("+ Test PASSED\n");
	else
		printf("+ Test FAILED\n");

	printf("\n====== %s ======\n\n", cfg_000[0].devname);

	return errors;
}
