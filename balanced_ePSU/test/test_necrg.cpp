

#include "../pnmcrg/pnMCRG.h"

using namespace oc;
/*

P0 inputs X, and outputs S;  P1 inputs Y, and outputs T

satisfying: if X[i] != Y[i], then S[i] == T[i]; otherwise S[i] != T[i]

*/
void nECRG_test(u32 idx, u32 logNum, u32 numThreads){

    u32 numElements = 1 << logNum;
    oc::CuckooParam params = oc::CuckooIndex<>::selectParams(numElements, ssp, 0, 3);
    u32 numBins = params.numBins(); // the real num of nECRG that is used in the whole protocol     

    Socket chl;
    chl = coproto::asioConnect("localhost:" + std::to_string(PORT + 101), idx);

    // prepare for test
    PRNG prng(sysRandomSeed());
    std::vector<block> inputSet(numBins);
    std::vector<block> necrg_out;

    u32 equalNum = numBins/2;
    for(u32 i = 0; i < equalNum; ++i){
        inputSet[i] = block(0, i+1);
    }
    for(u32 i = equalNum; i < numBins; ++i){
        inputSet[i] = block(0, idx+i+1);
    }    

    Timer timer;
    timer.setTimePoint("start"); 

    nECRG(idx, inputSet, necrg_out, chl, numThreads);
    timer.setTimePoint("nECRG"); 

    // test time and communication
    if(idx == 0){
        double comm = 0;
        comm += chl.bytesSent() + chl.bytesReceived();
        std::cout << "Comm cost = " << std::fixed << std::setprecision(3) << comm / 1024 / 1024 << " MB" << std::endl;
        std::cout << timer << std::endl;
    }

    // test the functionality
    if(idx == 0){
        coproto::sync_wait(chl.send(necrg_out));
    }
    else if(idx == 1){
        std::vector<block> necrg_out0(numBins);
        coproto::sync_wait(chl.recv(necrg_out0));

        u32 count = 0;
        for(u32 i = 0; i < numBins; ++i){
            if(necrg_out0[i] != necrg_out[i]){
                count += 1;
            }
        }
        if(count == equalNum){
            std::cout << "nECRG functionality test pass!" << std::endl;
        }
        else{
            std::cout << "nECRG functionality test fail!" << std::endl;
        }
        
    }
    coproto::sync_wait(chl.flush());
    coproto::sync_wait(chl.close());

}



int main(int agrc, char** argv){
    
    CLP cmd;
    cmd.parse(agrc, argv);
    u32 nn = cmd.getOr("nn", 14);
    u32 n = cmd.getOr("n", 1ull << nn);
    u32 nt = cmd.getOr("nt", 1);
    u32 idx = cmd.getOr("r", 0);

    bool pecrgTest = cmd.isSet("pecrg");
    bool pmcrgTest = cmd.isSet("pmcrg");
    bool necrgTest = cmd.isSet("necrg");
    bool pnmcrgTest = cmd.isSet("pnmcrg");
    bool psuTest = cmd.isSet("psu");

    bool help = cmd.isSet("h");
    if (help){
        std::cout << "protocol: non-equality conditional randomness generation" << std::endl;
        std::cout << "parameters" << std::endl;
        std::cout << "    -n:           number of elements in each set, default 1024" << std::endl;
        std::cout << "    -nn:          logarithm of the number of elements in each set, default 10" << std::endl;
        std::cout << "    -nt:          number of threads, default 1" << std::endl;
        std::cout << "    -r:           index of party" << std::endl;
        return 0;
    }    

    if ((idx > 1 || idx < 0)){
        std::cout << "wrong idx of party, please use -h to print help information" << std::endl;
        return 0;
    }

    nECRG_test(idx, nn, nt);

    return 0;
}

