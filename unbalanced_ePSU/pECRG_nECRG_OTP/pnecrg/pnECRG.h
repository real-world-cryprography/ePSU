#pragma once

#include "Circuit.h"
#include "define.h"
#include "curve25519.h"
#include <cryptoTools/Crypto/PRNG.h>
#include <volePSI/GMW/Gmw.h>
#include <cryptoTools/Network/Channel.h>
#include <cryptoTools/Common/Timer.h>
#include <cryptoTools/Common/BitVector.h>
#include "cryptoTools/Common/CLP.h"
#include <libOTe/Vole/Silent/SilentVoleReceiver.h>
#include <libOTe/Vole/Silent/SilentVoleSender.h>
#include <libOTe/Base/BaseOT.h>
#include <libOTe/TwoChooseOne/SoftSpokenOT/SoftSpokenShOtExt.h>
#include <coproto/Socket/AsioSocket.h>


using namespace oc;

constexpr uint64_t fieldBits = 5;



void genPermutation(u32 size, std::vector<u32> &pi);

// softspoken OT
void softSend(u32 numElements, Socket &chl, PRNG& prng, AlignedVector<std::array<block, 2>> &sMsgs, u32 numThreads = 1);
void softRecv(u32 numElements, BitVector bitV, Socket &chl, PRNG& prng, AlignedVector<block> &rMsgs, u32 numThreads = 1);

void SendEC25519Points(Socket &chl, std::vector<EC25519Point> &vecA, u32 numThreads = 1);
void ReceiveEC25519Points(Socket &chl, std::vector<EC25519Point> &vecA, u32 numThreads = 1);

// pECRG: permuted equality conditional randomness generation
void pECRG(u32 isPI, Socket &chl, std::vector<block> &matrix, u32 rowNum, u32 colNum, std::vector<u32> &pi, std::vector<block> &out, u32 numThreads = 1);

//pnECRG: permuted non equality conditional randomness generation
void pnECRG(u32 isPI, Socket &chl, std::vector<block> &matrix, u32 rowNum, u32 colNum, std::vector<u32> &pi, std::vector<block> &out, u32 numThreads = 1);