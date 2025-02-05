/** @file
*****************************************************************************
This is an implementation of balanced enhanced private set union based on pnMCRG and one-time pad.

References:
[TBZCC-USENIX-2025]: Fast Enhanced Private Set Union in the Balanced and Unbalanced Scenarios
Binbin Tu, Yujie Bai, Cong Zhang, Yang Cao, Yu Chen
USENIX Security 2025,

*****************************************************************************
* @author developed by Yujie Bai and Yang Cao (modified by Yu Chen)
*****************************************************************************/

# pragma once
#include "../pnmcrg/pnMCRG.h"

using namespace oc;
/*
bOPRF + OKVS = bOPPRF

pECRG + bOPPRF = pMCRG

pMCRG + nECRG = pnMCRG
*/


// balanced ePSU use pnMCRG and one-time pad
std::vector<block> balanced_ePSU(u32 idx, std::vector<block> &set, u32 numThreads);
