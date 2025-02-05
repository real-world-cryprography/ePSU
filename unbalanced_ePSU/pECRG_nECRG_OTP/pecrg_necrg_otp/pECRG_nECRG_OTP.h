
/** @file
*****************************************************************************
This is an implementation of permuted non equality conditional randomness generation and one-time pad (for unbalanced ePSU).

References:
[TBZCC-USENIX-2025]: Fast Enhanced Private Set Union in the Balanced and Unbalanced Scenarios
Binbin Tu, Yujie Bai, Cong Zhang, Yang Cao, Yu Chen
USENIX Security 2025,

*****************************************************************************
* @author developed by Yujie Bai and Yang Cao (modified by Yu Chen)
*****************************************************************************/

# pragma once
#include "../pnecrg/pnECRG.h"
#include "define.h"
#include <coproto/Socket/AsioSocket.h>
#include <volePSI/config.h>
#include <volePSI/Defines.h>
#include <cryptoTools/Network/Channel.h>
#include <string> 
#include <fstream>
#include <iostream>
#include <istream>

#include <algorithm>

using namespace oc;


void pECRG_nECRG_OTP(u32 isSender, u32 numThreads);
