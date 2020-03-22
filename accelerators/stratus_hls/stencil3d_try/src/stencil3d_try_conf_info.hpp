// Copyright (c) 2011-2019 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __STENCIL3D_TRY_CONF_INFO_HPP__
#define __STENCIL3D_TRY_CONF_INFO_HPP__

#include <systemc.h>

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
        this->row_size = 16;
        this->height_size = 32;
        this->col_size = 16;
    }

    conf_info_t(
        /* <<--ctor-args-->> */
        int32_t row_size, 
        int32_t height_size, 
        int32_t col_size
        )
    {
        /* <<--ctor-custom-->> */
        this->row_size = row_size;
        this->height_size = height_size;
        this->col_size = col_size;
    }

    // equals operator
    inline bool operator==(const conf_info_t &rhs) const
    {
        /* <<--eq-->> */
        if (row_size != rhs.row_size) return false;
        if (height_size != rhs.height_size) return false;
        if (col_size != rhs.col_size) return false;
        return true;
    }

    // assignment operator
    inline conf_info_t& operator=(const conf_info_t& other)
    {
        /* <<--assign-->> */
        row_size = other.row_size;
        height_size = other.height_size;
        col_size = other.col_size;
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
        os << "col_size = " << conf_info.col_size << "";
        os << "}";
        return os;
    }

        /* <<--params-->> */
        int32_t row_size;
        int32_t height_size;
        int32_t col_size;
};

#endif // __STENCIL3D_TRY_CONF_INFO_HPP__
