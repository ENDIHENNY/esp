#include <stdint.h>
#include <sstream>
#include <stdio.h>
#include <iostream>

#include "system.hpp"
#include "stencil3d_test.hpp"

void boundary_fill(int32_t row_size, int32_t col_size, int32_t height_size, int32_t *orig, int32_t *sol) {
    int32_t i = 0;
    int32_t j = 0; 
    int32_t k = 0;
    printf("start boundary filling\n");
    for(j=0; j<col_size; j++) {
        for(k=0; k<row_size; k++) {
	    int32_t index0 = k + row_size * j; 
 	    int32_t index1 = k + row_size * (j + col_size * (height_size-1)); 
	    sol[index0] = orig[index0];
 	    sol[index1] = orig[index1];

              }
    }
    for(i=1; i<height_size-1; i++) {
        for(k=0; k<row_size; k++) {
	    int32_t index0 = k + row_size * col_size * i;
	    int32_t index1 = k + row_size * ((col_size-1) + col_size*i);
	    sol[index0] = orig[index0];
	    sol[index1] = orig[index1];
        }
    }
    for(i=1; i<height_size-1; i++) {
        for(j=1; j<col_size-1; j++) {
	    int32_t index0 = row_size * (j + col_size * i);
	    int32_t index1 = row_size-1 + row_size * (j + col_size * i);
	    sol[index0] = orig[index0];
	    sol[index1] = orig[index1];
        }
    }
    printf("Finish boundary filling\n");

}

void stencil_compute(int32_t C0, int32_t C1, int32_t row_size, int32_t col_size, int32_t height_size, int32_t *orig, int32_t *sol) {
    int32_t i = 0;
    int32_t j = 0;
    int32_t k = 0;

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
		
		int32_t sum0 = orig[index0];
		int32_t sum1 = orig[index1] + orig[index2] + orig[index3] + 
		       orig[index4] + orig[index5] + orig[index6];
		 int32_t mul0 = sum0 * C0;
                int32_t mul1 = sum1 * C1;
		sol[index0] = mul0 + mul1;


                    }
        }
    }
}


