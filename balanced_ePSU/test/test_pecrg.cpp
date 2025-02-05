

#include "../pnmcrg/pnMCRG.h"

using namespace oc;

/*

P0 inputs X, and outputs S;  P1 inputs Y, and outputs T, PI

satisfying: if X[PI[i]] == Y[PI[i]], then S[i] == T[i]; otherwise s_i != t_i

*/

void pECRG_test(u32 idx, u32 logNum, u32 numThreads){
    oc::CuckooParam params = oc::CuckooIndex<>::selectParams(1<<logNum, ssp, 0, 3);
    u32 numBins = params.numBins(); // the real num of pECRG that is used in the whole protocol      

    Socket chl;
    chl = coproto::asioConnect("localhost:" + std::to_string(PORT + 101), idx);

    // prepare for test
    PRNG prng(sysRandomSeed());
    std::vector<block> inputSet(numBins);
    std::vector<block> pecrg_out(numBins);
    std::vector<u32> pi(numBins);

    // generate inputSet
    u32 equalNum = numBins/2;
    for(u32 i = 0; i < equalNum; ++i){
        inputSet[i] = block(0, i+1);
    }
    for(u32 i = equalNum; i < numBins; ++i){
        inputSet[i] = block(0, idx+i+1);
    }  

    Timer timer;
    timer.setTimePoint("start"); 

    pECRG(idx, inputSet, pecrg_out, pi, chl, numThreads);
    timer.setTimePoint("pECRG"); 

    // test time and communication
    if(idx == 0){
        double comm = 0;
        comm += chl.bytesSent() + chl.bytesReceived();
        std::cout << "Comm cost = " << std::fixed << std::setprecision(3) << comm / 1024 / 1024 << " MB" << std::endl;
        std::cout << timer << std::endl;

    }
    // test the functionality
    if(idx == 0){
        coproto::sync_wait(chl.send(pecrg_out));
    }
    else if(idx == 1){
        std::vector<block> pecrg_out0(numBins);
        coproto::sync_wait(chl.recv(pecrg_out0));

        u32 count = 0;
        for(u32 i = 0; i < numBins; ++i){
            if(pecrg_out0[i] == pecrg_out[i]){
                count += 1;
            }
        }
        if(count == equalNum){
            std::cout << "pECRG functionality test pass!" << std::endl;
        }
        else{
            std::cout << "pECRG functionality test fail!" << std::endl;
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

    bool help = cmd.isSet("h");
    if (help){
        std::cout << "protocol: permuted equality conditional randomness generation" << std::endl;
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

    pECRG_test(idx, nn, nt);

    return 0;
}

