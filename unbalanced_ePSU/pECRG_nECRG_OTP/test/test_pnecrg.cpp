#include "../pnecrg/pnECRG.h"
#include "../pnecrg/define.h"
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



/*

P0 inputs X[i][j], and outputs S[i], PI;  P1 inputs Y[i][j], and outputs T[i]

satisfying: if exits X[PI[i]][j] == Y[PI[i]][j], then S[i] != T[i]; otherwise S[i] == T[i]

*/
void pnECRG_test(u32 idx, u32 colNum, u32 numThreads){
    u32 rowNum = 1638;
    u32 numElements = rowNum * colNum;    

    Socket chl;
    chl = coproto::asioConnect("localhost:" + std::to_string(PORT + 101), idx);

    // prepare for test
    PRNG prng(sysRandomSeed());
    std::vector<block> matrix(numElements);
    std::vector<u32> pi;
    std::vector<block> pnecrg_out;
    // prng.get(matrix.data(), matrix.size());

    // generate inputSet
    u32 equalNum = 10;
    for(u32 i = 0; i < equalNum; ++i){
        for(u32 j = 0; j < colNum; ++j){
            matrix[i] = block(0, i*colNum +j +1);//equal
        }
        
    }
    for(u32 i = equalNum; i < rowNum; ++i){
        for(u32 j = 0; j < colNum; ++j){
            matrix[i] = block(0, i*colNum+j+idx+1);//unequal
        };
    } 

    Timer timer;
    timer.setTimePoint("start"); 

    pnECRG(idx, chl, matrix, rowNum, colNum, pi, pnecrg_out, numThreads);
    timer.setTimePoint("pnECRG"); 

    if(idx == 0){
        double comm = 0;
        comm += chl.bytesSent() + chl.bytesReceived();
        std::cout << "pnECRG comm cost = " << std::fixed << std::setprecision(3) << comm / 1024 / 1024 << " MB" << std::endl;
        std::cout << timer << std::endl;

    }

    if(idx == 0){
        coproto::sync_wait(chl.send(pnecrg_out));
    }
    else if(idx == 1){
        std::vector<block> pnecrg_out0(rowNum);
        coproto::sync_wait(chl.recv(pnecrg_out0));

        u32 count = 0;
        for(u32 i = 0; i < rowNum; ++i){
            if(pnecrg_out0[i] != pnecrg_out[i]){
                count += 1;
            }
        }
        if(count == equalNum){
            std::cout << "pnECRG functionality test pass!" << std::endl;
        }
        else{
            std::cout << "pnECRG functionality test fail!" << std::endl;
        }
        
    }

    coproto::sync_wait(chl.flush());
    coproto::sync_wait(chl.close());
}


int main(int agrc, char** argv){
    
    CLP cmd;
    cmd.parse(agrc, argv);
    
    u32 nt = cmd.getOr("nt", 1);
    u32 idx = cmd.getOr("r", 0);

    u32 colNum = cmd.getOr("cn", 1);
    bool pnECRGTest = cmd.isSet("pnecrg");

    bool help = cmd.isSet("h");
    
    if (help){
        std::cout << "protocol: permuted non equality conditional randomness generation" << std::endl;
        std::cout << "parameters" << std::endl;
        std::cout << "    -colNum:      column number of matrix from MCRG, default 1" << std::endl;
        std::cout << "    -nt:          number of threads, default 1" << std::endl;
        std::cout << "    -r:           index of party" << std::endl;
        return 0;
    }    

    if ((idx > 1 || idx < 0)){
        std::cout << "wrong idx of party, please use -h to print help information" << std::endl;
        return 0;
    }

    pnECRG_test(idx, colNum, nt);
   
    return 0;
}
