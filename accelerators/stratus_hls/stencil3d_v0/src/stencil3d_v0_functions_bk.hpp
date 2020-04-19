#ifndef _STENCIL3D_V0_FUNCTIONS_HPP_
#define _STENCIL3D_V0_FUNCTIONS_HPP_

#include "stencil3d_v0_conf_info.hpp"
#include "stencil3d_v0.hpp"

void boundary_fill(int32_t row_size, int32_t col_size, int32_t height_size, sc_dt::sc_int<DATA_WIDTH> orig[PLM_IN_WORD], sc_dt::sc_int<DATA_WIDTH> sol[PLM_OUT_WORD]) {

    height_bound_col : for(int32_t j=0; j<col_size; j++) {
        height_bound_row : for(int32_t k=0; k<row_size; k++) {
	    int32_t index0 = k + row_size * j; 
 	    int32_t index1 = k + row_size * (j + col_size * (height_size-1)); 
	    sol[index0] = orig[index0];
 	    sol[index1] = orig[index1];
        }
    }
    col_bound_height : for(int32_t i=1; i<height_size-1; i++) {
        col_bound_row : for(int32_t k=0; k<row_size; k++) {
	    int32_t index0 = k + row_size * col_size * i;
	    int32_t index1 = k + row_size * ((col_size-1) + col_size*i);
	    sol[index0] = orig[index0];
	    sol[index1] = orig[index1];
        }
    }
    row_bound_height : for(int32_t i=1; i<height_size-1; i++) {
        row_bound_col : for(int32_t j=1; j<col_size-1; j++) {
	    int32_t index0 = row_size * (j + col_size * i);
	    int32_t index1 = row_size-1 + row_size * (j + col_size * i);
	    sol[index0] = orig[index0];
	    sol[index1] = orig[index1];
        }
    }

}

void stencil_compute(int32_t C0, int32_t C1, int32_t row_size, int32_t col_size, int32_t height_size, sc_dt::sc_int<DATA_WIDTH> orig[PLM_IN_WORD], sc_dt::sc_int<DATA_WIDTH> sol[PLM_OUT_WORD]) {

    // Stencil computation
    loop_height : for(int32_t i = 1; i < height_size - 1; i++){
        loop_col : for(int32_t j = 1; j < col_size - 1; j++){
            loop_row : for(int32_t k = 1; k < row_size - 1; k++){
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
	 	
                // sum0 = orig[INDX(row_size, col_size, k, j, i)];
                // sum1 = orig[INDX(row_size, col_size, k, j, i + 1)] +
                //       orig[INDX(row_size, col_size, k, j, i - 1)] +
                //       orig[INDX(row_size, col_size, k, j + 1, i)] +
                //       orig[INDX(row_size, col_size, k, j - 1, i)] +
                //       orig[INDX(row_size, col_size, k + 1, j, i)] +
                //       orig[INDX(row_size, col_size, k - 1, j, i)];
                int32_t mul0 = sum0 * C0;
                int32_t mul1 = sum1 * C1;
		sol[index0] = mul0 + mul1;
                // sol[INDX(row_size, col_size, k, j, i)] = mul0 + mul1;
            }
        }
    }
}

    
#endif /* _STENCIL3D_V0_FUNCTIONS_HPP_ */