/*
 * Golden output computation - Xiaofu Pei
 *
 */

#ifndef TEST_STENCIL3D_H
#define TEST_STENCIL3D_H

#define INDX(_row_size,_col_size,_i,_j,_k) ((_i)+_row_size*((_j)+_col_size*(_k))) 
#define TYPE int32_t

void boundary_fill(TYPE row_size, TYPE col_size, TYPE height_size, TYPE *orig, TYPE *sol); 
void stencil_compute(TYPE C0, TYPE C1, TYPE row_size, TYPE col_size, TYPE height_size, TYPE *orig, TYPE *sol);

#endif


