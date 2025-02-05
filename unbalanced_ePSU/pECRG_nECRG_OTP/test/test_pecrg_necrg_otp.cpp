#include "../pecrg_necrg_otp/pECRG_nECRG_OTP.h"


using namespace oc;


void pECRG_nECRG_OTP_Test(u32 isSender, u32 numThreads)
{
    pECRG_nECRG_OTP(isSender, numThreads);   
    if(!isSender){
        std::cout << "pECRG_nECRG_OTP_Test finished." << std::endl;
    }
}


int main(int agrc, char** argv){
    
    CLP cmd;
    cmd.parse(agrc, argv);
    
    u32 nt = cmd.getOr("nt", 1);
    u32 idx = cmd.getOr("r", 0);
    bool help = cmd.isSet("h");
    
    if (help){
        std::cout << "protocol: permuted non equality conditional randomness generation and one-time pad (for unbalanced ePSU)" << std::endl;
        std::cout << "parameters" << std::endl;
        std::cout << "    -nt:          number of threads, default 1" << std::endl;
        std::cout << "    -r:           index of party" << std::endl;
        return 0;
    }    

    if ((idx > 1 || idx < 0)){
        std::cout << "wrong idx of party, please use -h to print help information" << std::endl;
        return 0;
    }
    pECRG_nECRG_OTP_Test(idx, nt);

    return 0;
}
