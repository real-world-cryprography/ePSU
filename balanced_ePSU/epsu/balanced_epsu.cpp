

#include "balanced_epsu.h"

using namespace oc;
/*
bOPRF + OKVS = bOPPRF

pECRG + bOPPRF = pMCRG

pMCRG + nECRG = pnMCRG
*/

// balanced ePSU use pnMCRG and one-time pad
std::vector<block> balanced_ePSU(u32 idx, std::vector<block> &set, u32 numThreads){
    
    u32 numElements = set.size();
    oc::CuckooParam params = oc::CuckooIndex<>::selectParams(numElements, ssp, 0, 3);
    u32 numBins = params.numBins();    
  
    std::vector<block> permutedX(numBins);
    std::vector<block> pnMCRG_out(numBins);// use pnMCRG_out as one-time pad
    std::vector<block> vecOTP_out(numBins);

    Timer timer;
    timer.setTimePoint("start");    

    Socket chl;
    chl = coproto::asioConnect("localhost:" + std::to_string(PORT + 101), idx);
    

    if (idx == 0){
        // run cuckoo hash, and save permuted cuckoo hash table(as x||1) in permutedX0
        pnMCRG(idx, numElements, set, pnMCRG_out, permutedX, chl, numThreads);
        // one-time pad
        for(u32 i = 0; i < numBins; ++i){
            vecOTP_out[i] = pnMCRG_out[i] ^ permutedX[i];
        }

        coproto::sync_wait(chl.send(vecOTP_out));
        timer.setTimePoint("end"); 

    } else {
        pnMCRG(idx, numElements, set, pnMCRG_out, permutedX, chl, numThreads);
        coproto::sync_wait(chl.recv(vecOTP_out));
        std::vector<block> setUnion(set);

        for(u32 i = 0; i < numBins; ++i){
            // one-time pad
            vecOTP_out[i] ^= pnMCRG_out[i];
            if(vecOTP_out[i].mData[0] == 1){
                setUnion.emplace_back(vecOTP_out[i].mData[1]);
            }
        }

        timer.setTimePoint("end"); 
        
        double comm = 0;
        comm += chl.bytesSent() + chl.bytesReceived();

        std::cout << "Comm cost = " << std::fixed << std::setprecision(3) << comm / 1024 / 1024 << " MB" << std::endl;

        std::cout << " " << std::endl;

        std::cout << timer << std::endl;
        return setUnion;

    }
    coproto::sync_wait(chl.flush());
    coproto::sync_wait(chl.close());
    return std::vector<block>(); 
}

