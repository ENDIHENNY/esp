#ifndef _STENCIL3D_V0_FUNCTIONS_HPP_
#define _STENCIL3D_V0_FUNCTIONS_HPP_
#include "stencil3d_v0.hpp"
#include "stencil3d_v0_directives.hpp"

void stencil_compute(int in_len, int plm_adj, int32_t C0, int32_t C1, int32_t row_size, int32_t col_size, int32_t height_size, sc_dt::sc_int<DATA_WIDTH> orig[PLM_IN_WORD], sc_dt::sc_int<DATA_WIDTH> sol[PLM_OUT_WORD]) {
 
    int32_t map = row_size * col_size;
    int count_compute = 0;

    if (PLM_IN_WORD < row_size * col_size * height_size) {
	    for (int i = 0; i < in_len; i++) {
		
		HLS_COMPUTE_STENCIL;		


		int index0 = i;	
		int index1 = index0 + map;	
		int index2 = index0 - map;	
		int index3 = index0 + row_size;	
		int index4 = index0 - row_size;	
		int index5 = index0 + 1;	
		int index6 = index0 - 1;	

		int abs_idx = plm_adj + i;
		int flag = 0;

	    height_bound_col : for(int32_t j=0; j<col_size; j++) {
		height_bound_row : for(int32_t k=0; k<row_size; k++) {
		    int32_t idx0 = k + row_size * j; 
		    int32_t idx1 = idx0 + map *  (height_size-1); 
		    if (abs_idx == idx0 || abs_idx == idx1) {
			sol[index0] = orig[index0];
		    	flag = 1;
			goto end1;
		    }
		}
	    }

	    end1: if (flag == 1)	continue;

	    col_bound_height : for(int32_t ii=1; ii<height_size-1; ii++) {
		col_bound_row : for(int32_t k=0; k<row_size; k++) {
		    int32_t idx0 = k + map * ii;
		    int32_t idx1 = idx0 + map - row_size;
		    if (abs_idx == idx0 || abs_idx == idx1) {
			sol[index0] = orig[index0];
		    	flag = 1;
			goto end2;
		    }
		}
	    }

	    end2: if (flag == 1)	continue;

	    row_bound_height : for(int32_t ii=1; ii<height_size-1; ii++) {
		row_bound_col : for(int32_t j=1; j<col_size-1; j++) {
		    int32_t idx0 = row_size * j + map * ii;
		    int32_t idx1 = idx0 + row_size - 1 ; 
		    if (abs_idx == idx0 || abs_idx == idx1) {
			sol[index0] = orig[index0];
		    	flag = 1;
			goto end3;
		    }
		}
	    }
	    end3: if (flag == 1)	continue;
			
		
		
		if (index1 < PLM_IN_WORD && index2 >= 0) {
		    	#if (TYPEDEF == 0)
				int32_t sum0 = orig[index0]; 
				int32_t sum1 = orig[index1] + orig[index2] + orig[index3] + 
				       orig[index4] + orig[index5] + orig[index6];
				
				int32_t mul0 = sum0 * C0;
				int32_t mul1 = sum1 * C1;
				sol[index0] = mul0 + mul1;
    				count_compute++;
    				//cout << "DEBUG Info: Compute " << count_compute << " DONE, sol = " << mul0+mul1 << endl;
				continue;
			
			#elif (TYPEDEF == 1)
				//int2fp(FPDATA temp[PLM_IN_WORD], orig);
				FPDATA sum0 = int2fp<FPDATA, WORD_SIZE>(orig[index0]);
				FPDATA sum1 = int2fp<FPDATA, WORD_SIZE>(orig[index1]) + int2fp<FPDATA, WORD_SIZE>(orig[index2]) +int2fp<FPDATA, WORD_SIZE>(orig[index3]) +int2fp<FPDATA, WORD_SIZE>(orig[index4]) +int2fp<FPDATA, WORD_SIZE>(orig[index5]) +int2fp<FPDATA, WORD_SIZE>(orig[index6]);
				FPDATA mul0 = ((FPDATA) C0) * sum0;
				FPDATA mul1 = ((FPDATA) C1) * sum1;
				sol[index0] = fp2int<FPDATA, WORD_SIZE>(mul0 + mul1);
    				count_compute++;
    				//cout << "DEBUG Info: Compute " << count_compute << " DONE, sol = " << mul0+mul1 << endl;
				continue;
			#endif

		}
    			        //cout << "DEBUG Info: Compute LEFT, abs_idx = " << abs_idx << endl;
    			        //cout << "DEBUG Info: Compute LEFT, index0 = " << index0 << endl;
    			#if (TYPEDEF == 0)
				sol[index0] = INT_MAX;
			#elif (TYPEDEF == 1)
				sol[index0] = 0;
			#endif
	
	    }
    				//cout << "DEBUG Info: Compute time = " << count_compute << endl;
    }
    else {
	    // Stencil computation
	    loop_height : for(int32_t i = 1; i < height_size - 1; i++){
		loop_col : for(int32_t j = 1; j < col_size - 1; j++){
			int32_t index0_share = row_size * j + map * i;
		    loop_row : for(int32_t k = 1; k < row_size - 1; k++){
			int32_t index0 = k + index0_share;
			int32_t index1 = k + index0_share + map;
			int32_t index2 = k + index0_share - map; // -> maximum distance -> 2map = 72
			int32_t index3 = k + index0_share + row_size;
			int32_t index4 = k + index0_share - row_size;
			int32_t index5 = k + 1 + index0_share;
			int32_t index6 = k - 1 + index0_share;
			
			//cout << "index0 for computing : "<< index0 << endl;
			//cout << "index1 for computing : "<< index1 << endl;
			//cout << "index2 for computing : "<< index2 << endl;
			//cout << "index3 for computing : "<< index3 << endl;
			//cout << "index4 for computing : "<< index4 << endl;
			//cout << "index5 for computing : "<< index5 << endl;
			//cout << "index6 for computing : "<< index6 << endl;
			#if (TYPEDEF == 0)
				int32_t sum0 = orig[index0]; 
				int32_t sum1 = orig[index1] + orig[index2] + orig[index3] + 
				       orig[index4] + orig[index5] + orig[index6];
				
				int32_t mul0 = sum0 * C0;
				int32_t mul1 = sum1 * C1;
				sol[index0] = mul0 + mul1;
			
			#elif (TYPEDEF == 1)
				//int2fp(FPDATA temp[PLM_IN_WORD], orig);
				FPDATA sum0 = int2fp<FPDATA, WORD_SIZE>(orig[index0]);
				FPDATA sum1 = int2fp<FPDATA, WORD_SIZE>(orig[index1]) + int2fp<FPDATA, WORD_SIZE>(orig[index2]) +int2fp<FPDATA, WORD_SIZE>(orig[index3]) +int2fp<FPDATA, WORD_SIZE>(orig[index4]) +int2fp<FPDATA, WORD_SIZE>(orig[index5]) +int2fp<FPDATA, WORD_SIZE>(orig[index6]);
				FPDATA mul0 = ((FPDATA) C0) * sum0;
				FPDATA mul1 = ((FPDATA) C1) * sum1;
				sol[index0] = fp2int<FPDATA, WORD_SIZE>(mul0 + mul1);
			#endif
			
		    }
		}
	    }
    }
}

void boundary_fill(int in_rem, int32_t row_size, int32_t col_size, int32_t height_size, sc_dt::sc_int<DATA_WIDTH> orig[PLM_IN_WORD], sc_dt::sc_int<DATA_WIDTH> sol[PLM_OUT_WORD]) {

    int32_t map = row_size * col_size;
    height_bound_col : for(int32_t j=0; j<col_size; j++) {
        height_bound_row : for(int32_t k=0; k<row_size; k++) {
	    int32_t index0 = k + row_size * j; 
 	    int32_t index1 = index0 + map *  (height_size-1); 
	    sol[index0] = orig[index0];
 	    sol[index1] = orig[index1];
	    //cout << "index0 for height filling: "<< index0 << endl;
	    //cout << "index1 for height filling: "<< index1 << endl;
        }
    }
    col_bound_height : for(int32_t i=1; i<height_size-1; i++) {
        col_bound_row : for(int32_t k=0; k<row_size; k++) {
	    int32_t index0 = k + map * i;
	    int32_t index1 = index0 + map - row_size;
	    sol[index0] = orig[index0];
	    sol[index1] = orig[index1];
	    //cout << "index0 for column filling: "<< index0 << endl;
	    //cout << "index1 for column filling: "<< index1 << endl;
        }
    }
    row_bound_height : for(int32_t i=1; i<height_size-1; i++) {
        row_bound_col : for(int32_t j=1; j<col_size-1; j++) {
	    int32_t index0 = row_size * j + map * i;
	    int32_t index1 = index0 + row_size - 1 ; 
	    sol[index0] = orig[index0];
	    sol[index1] = orig[index1];
	    //cout << "index0 for row filling: "<< index0 << endl;
	    //cout << "index1 for row filling: "<< index1 << endl;
        }
    }

}

#endif /* _STENCIL3D_V0_FUNCTIONS_HPP_ */
