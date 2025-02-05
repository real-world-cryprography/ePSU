#pragma once

#include <volePSI/Defines.h>
#include <volePSI/config.h>
#include <volePSI/Paxos.h>
#include <volePSI/SimpleIndex.h>

#include <cryptoTools/Crypto/PRNG.h>
#include <cryptoTools/Network/Channel.h>
#include <cryptoTools/Common/CuckooIndex.h>
#include <cryptoTools/Common/Timer.h>
#include <cryptoTools/Common/BitVector.h>

#include <libOTe/Vole/Silent/SilentVoleReceiver.h>
#include <libOTe/Vole/Silent/SilentVoleSender.h>


#include <string> 
#include <fstream>

#include "ssPEQT.h"
#include "ssROT.h"
#include "curve25519.h"


#include <algorithm>

using namespace oc;

inline std::random_device rd2;
inline std::mt19937 global_built_in_prg2(rd2());


void genPermutation(u32 size, std::vector<u32> &pi);

// permute data according to pi
void permute(std::vector<u32> &pi, std::vector<block> &data);


void SendEC25519Points(Socket &chl, std::vector<EC25519Point> &vecA, u32 numThreads); 

void ReceiveEC25519Points(Socket &chl, std::vector<EC25519Point> &vecA, u32 numThreads); 

void nECRG(u32 idx, std::vector<block> &input, std::vector<block> &out, Socket &chl, u32 numThreads);

void pECRG(u32 isPi, std::vector<block> &set, std::vector<block> &out, std::vector<u32> &pi, Socket &chl, u32 numThreads);

// pMCRG = mpOPRF + pECRG
void pMCRG(u32 idx, u32 numElements, std::vector<block> &set, std::vector<block> &out, std::vector<block> &permutedX0, Socket &chl, u32 numThreads);   

// pnMCRG = MCRG + nECRG
void pnMCRG(u32 idx, u32 numElements, std::vector<block> &set, std::vector<block> &out, std::vector<block> &permutedX0, Socket &chl, u32 numThreads);









