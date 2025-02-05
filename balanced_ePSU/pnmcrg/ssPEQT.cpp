#include "ssPEQT.h"

BetaCircuit isOneCircuit(u64 bits)
{
    BetaCircuit cd;

    BetaBundle a(bits);

    cd.addInputBundle(a);

    //for (u64 i = 1; i < bits; ++i)
    //    cd.addGate(a.mWires[i], a.mWires[i], oc::GateType::Nxor, a.mWires[i]);
    for (u64 i = 0; i < bits; ++i){
        //cd.addCopy(a.mWires[i]);
    }
    

    u64 step = 1;

    while (step < bits)
    {
        //std::cout << "\n step " << step << std::endl;
        for (u64 i = 0; i + step < bits; i += step * 2)
        {
            //std::cout << "a[" << i << "] &= a[" << (i + step) << "]" << std::endl;
            cd.addGate(a.mWires[i], a.mWires[i + step], oc::GateType::And, a.mWires[i]);
        }

        step *= 2;
    }
    //cd.addOutputBundle()
    a.mWires.resize(1);
    cd.mOutputs.push_back(a);

    cd.levelByAndDepth();

    return cd;
}

void ssPEQT(u32 idx, std::vector<block> &input, BitVector &out, Socket& chl, u32 numThreads)
{
    u32 numBins = input.size();
    u64 keyBitLength = ssp + oc::log2ceil(numBins);  
    u64 keyByteLength = oc::divCeil(keyBitLength, 8);    
    PRNG prng(sysRandomSeed());

    mMatrix<u8> mLabel(numBins, keyByteLength);
    for(u32 i = 0; i < numBins; ++i){
        memcpy(&mLabel(i,0), &input[i], keyByteLength);
    }

    // call gmw
    auto cir = volePSI::isZeroCircuit(keyBitLength);
    
    // volePSI::BetaCircuit cir = volePSI::isZeroCircuit(keyBitLength);
    volePSI::Gmw cmp;
    cmp.init(mLabel.rows(), cir, numThreads, idx, prng.get());

    if(idx == 1){
        cmp.setInput(0, mLabel);
    }else{
        cmp.implSetInput(0, mLabel, mLabel.cols());
    }

    coproto::sync_wait(cmp.run(chl));
     
    mMatrix<u8> mOut;
    mOut.resize(numBins, 1);
    cmp.getOutput(0, mOut);   
       
    // get the final output
    out.resize(numBins);
    for (u32 i = 0; i < numBins; ++i){
        out[i] = mOut(i, 0) & 1;
    }    
    return;
}



