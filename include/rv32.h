//=============================================================
// 
// Copyright (c) 2021 Simon Southwell. All rights reserved.
//
// Date: 12th July 2021
//
// Contains the class definition for the top level derived class
//
// This file is part of the Zicsr extended RISC-V instruction
// set simulator (rv32).
//
// This code is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This code is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this code. If not, see <http://www.gnu.org/licenses/>.
//
//=============================================================

#ifndef _RV32_H_
#define _RV32_H_

#include "rv32i_cpu.h"

// This class is the top level ISS implementation class, configurable
// to have only those extensions required to be modelled. The
// rv32_inheritance.h file defines which extensions are to be included.
class rv32 : public rv32i_cpu
{
public:
    LIBRISCV32_API rv32(FILE* dbg_fp = stdout) : rv32i_cpu(dbg_fp)
    {
    };

    /*****************************************************/
    /* Add customisations and additional extensions here */
    /*****************************************************/
};

#endif
