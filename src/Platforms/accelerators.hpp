//////////////////////////////////////////////////////////////////////////////////////
// This file is distributed under the University of Illinois/NCSA Open Source License.
// See LICENSE file in top directory for details.
//
// Copyright (c) 2019 QMCPACK developers.
//
// File developed by: Ye Luo, yeluo@anl.gov, Argonne National Laboratory
//
// File created by: Ye Luo, yeluo@anl.gov, Argonne National Laboratory
//////////////////////////////////////////////////////////////////////////////////////


#ifndef QMCPLUSPLUS_ACCELERATORS_HPP
#define QMCPLUSPLUS_ACCELERATORS_HPP

#include "Message/Communicate.h"

namespace qmcplusplus
{
void assignAccelerators(Communicate& NodeComm);
}
#endif
