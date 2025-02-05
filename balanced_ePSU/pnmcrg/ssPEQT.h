#pragma once

#include <volePSI/GMW/Gmw.h>
#include <volePSI/Defines.h>
#include <volePSI/config.h>
#include <volePSI/Paxos.h>


#include <cryptoTools/Crypto/PRNG.h>
#include <cryptoTools/Network/Channel.h>
#include <cryptoTools/Common/Timer.h>
#include <cryptoTools/Common/BitVector.h>


#include "Defines.h"
#include "ssROT.h"

#include <string> 
#include <fstream>

#include <cryptoTools/Circuit/BetaCircuit.h>
#include <cryptoTools/Circuit/Gate.h>
#include "Defines.h"


// using BetaCircuit = oc::BetaCircuit;
// using BetaBundle = oc::BetaBundle;
// using GateType = oc::GateType;


using namespace oc;

BetaCircuit isOneCircuit(u64 n);

void ssPEQT(u32 idx, std::vector<block> &input, BitVector &out, Socket& chl, u32 numThreads = 1);








