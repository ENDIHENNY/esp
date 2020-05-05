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
static float gen_random_num(float min, float max)
{
    return (max - min) * ((((float) rand()) / (float) RAND_MAX) + min);
}


static int validate_buffer(token_t *out, token_t *gold)
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

static void init_buffer(token_t *in, token_t * gold)
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
