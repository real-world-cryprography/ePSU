

#include "../epsu/balanced_epsu.h"

using namespace oc;


// balanced_ePSU test
void balanced_ePSU_test(u32 idx, u32 numElements, u32 numThreads){

    std::vector<block> set(numElements);


    Timer timer;
    timer.setTimePoint("start");    

    Socket chl;
    chl = coproto::asioConnect("localhost:" + std::to_string(PORT + 101), idx);
    
    // generate set
    for (u32 i = 0; i < numElements; i++)
    {
        set[i] = oc::toBlock(0, idx + i + 1);
    }

    if (idx == 1){
        std::vector<block> out;
        out = balanced_ePSU(idx, set, numThreads);
        u32 UNION_CARDINALITY = numElements + 1;
        if(UNION_CARDINALITY == out.size()){
            std::cout << "Balanced_ePSU functionality test pass! And union size is: " << out.size() << std::endl;
        }
        else
        {
            std::cout << "Failure!  ideal union size: " << UNION_CARDINALITY << std::endl;
            std::cout << "Failure!  real union size: " << out.size() << std::endl;
        }
        timer.setTimePoint("end"); 

    } else {
        balanced_ePSU(idx, set, numThreads);
    }

   
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
        std::cout << "protocol: two-party balanced private set union" << std::endl;
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

    balanced_ePSU_test(idx, n, nt);
    return 0;
}

