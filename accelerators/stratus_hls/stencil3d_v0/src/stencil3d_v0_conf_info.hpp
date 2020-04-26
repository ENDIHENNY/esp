// Copyright (c) 2011-2019 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __STENCIL3D_V0_CONF_INFO_HPP__
#define __STENCIL3D_V0_CONF_INFO_HPP__

#include <systemc.h>
#include <cstdio>
#include <cstdlib>
using namespace std;

//
// Configuration parameters for the accelerator.
//
class conf_info_t
{
public:

    //
    // constructors
    //
    conf_info_t()
    {
        /* <<--ctor-->> */
        this->row_size = 6;
        this->height_size = 10;
        this->coef_1 = -1;
        this->col_size = 6;
        this->coef_0 = 6;
        this->stencil_n = 1;
    }

    conf_info_t(
        /* <<--ctor-args-->> */
        int32_t row_size, 
        int32_t height_size, 
        int32_t coef_1, 
        int32_t col_size, 
        int32_t coef_0,
        int32_t stencil_n
        )
    {
        /* <<--ctor-custom-->> */
        this->row_size = row_size;
        this->height_size = height_size;
        this->coef_1 = coef_1;
        this->col_size = col_size;
        this->coef_0 = coef_0;
        this->stencil_n = stencil_n;
    }

    // equals operator
    inline bool operator==(const conf_info_t &rhs) const
    {
        /* <<--eq-->> */
        if (row_size != rhs.row_size) return false;
        if (height_size != rhs.height_size) return false;
        if (coef_1 != rhs.coef_1) return false;
        if (col_size != rhs.col_size) return false;
        if (coef_0 != rhs.coef_0) return false;
        if (stencil_n != rhs.stencil_n) return false;
        return true;
    }

    // assignment operator
    inline conf_info_t& operator=(const conf_info_t& other)
    {
        /* <<--assign-->> */
        row_size = other.row_size;
        height_size = other.height_size;
        coef_1 = other.coef_1;
        col_size = other.col_size;
        coef_0 = other.coef_0;
        stencil_n = other.stencil_n;
        return *this;
    }

    // VCD dumping function
    friend void sc_trace(sc_trace_file *tf, const conf_info_t &v, const std::string &NAME)
    {}

    // redirection operator
    friend ostream& operator << (ostream& os, conf_info_t const &conf_info)
    {
        os << "{";
        /* <<--print-->> */
        os << "row_size = " << conf_info.row_size << ", ";
        os << "height_size = " << conf_info.height_size << ", ";
        os << "coef_1 = " << conf_info.coef_1 << ", ";
        os << "col_size = " << conf_info.col_size << ", ";
        os << "coef_0 = " << conf_info.coef_0 << ", ";
        os << "stencil_n = " << conf_info.stencil_n << "";
        os << "}";
        return os;
    }

        /* <<--params-->> */
        int32_t row_size;
        int32_t height_size;
        int32_t coef_1;
        int32_t col_size;
        int32_t coef_0;
        int32_t stencil_n;
};

#endif // __STENCIL3D_V0_CONF_INFO_HPP__
