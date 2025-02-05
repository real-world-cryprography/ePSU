

#include "../pnmcrg/pnMCRG.h"

using namespace oc;

/*

P0 inputs X, and outputs S, PI;  P1 inputs Y, and outputs T

satisfying: if X[PI[i]] in Y[PI[i]], then S[i] == T[i]; otherwise S[i] != T[i]

*/
void pnMCRG_test(u32 idx, u32 logNum,u32 numThreads){

    u32 numElements = 1 << logNum;
    oc::CuckooParam params = oc::CuckooIndex<>::selectParams(numElements, ssp, 0, 3);
    u32 numBins = params.numBins(); // the real num of pnMCRG that is used in the whole protocol      

    Socket chl;
    chl = coproto::asioConnect("localhost:" + std::to_string(PORT + 101), idx);

    // prepare for test
    PRNG prng(sysRandomSeed());
    std::vector<block> inputSet(numElements);
    std::vector<block> permutedX0;
    std::vector<block> pnmcrg_out;

    u32 equalNum = numElements/2;
    for(u32 i = 0; i < equalNum; ++i){
        inputSet[i] = block(0, i+1);
    }
    for(u32 i = equalNum; i < numElements; ++i){
        inputSet[i] = block(0, idx*numElements +i+1);
    }  

    Timer timer;
    timer.setTimePoint("start"); 

    pnMCRG(idx, numElements, inputSet, pnmcrg_out, permutedX0, chl, numThreads);
    timer.setTimePoint("pnMCRG");     

    // test time and communication
    if(idx == 0){
        double comm = 0;
        comm += chl.bytesSent() + chl.bytesReceived();
        std::cout << "Comm cost = " << std::fixed << std::setprecision(3) << comm / 1024 / 1024 << " MB" << std::endl;
        std::cout << timer << std::endl;
    }

    // test the functionality
    if(idx == 0){
        coproto::sync_wait(chl.send(pnmcrg_out));
    }
    else if(idx == 1){
        std::vector<block> pnmcrg_out0(numBins);
        coproto::sync_wait(chl.recv(pnmcrg_out0));

        u32 count = 0;
        for(u32 i = 0; i < numBins; ++i){
            if(pnmcrg_out0[i] != pnmcrg_out[i]){
                count += 1;
            }
        }
        if(count == equalNum){
            std::cout << "pnMCRG functionality test pass!" << std::endl;
        }
        else{
            std::cout << "pnMCRG functionality test fail!" << std::endl;
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
        std::cout << "protocol: permuted non membership conditional randomness generation" << std::endl;
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
    pnMCRG_test(idx, nn, nt);

    return 0;
}

