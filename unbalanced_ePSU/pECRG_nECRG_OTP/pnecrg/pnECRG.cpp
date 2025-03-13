#include "pnECRG.h"

void genPermutation(u32 size, std::vector<u32> &pi)
{
    pi.resize(size);
    for (size_t i = 0; i < pi.size(); ++i){
        pi[i] = i;
    }
    std::shuffle(pi.begin(), pi.end(), global_built_in_prg);
    return;
}


void softSend(u32 numElements, Socket &chl, PRNG& prng, AlignedVector<std::array<block, 2>> &sMsgs, u32 numThreads)
{
    SoftSpokenShOtSender<> sender;
    sender.init(fieldBits, true);
    const size_t numBaseOTs = sender.baseOtCount();
    PRNG prngOT(prng.get<block>());
    AlignedVector<block> baseMsg;
    // choice bits for baseOT
    BitVector baseChoice;

    // OTE's sender is base OT's receiver
    DefaultBaseOT base;
    baseMsg.resize(numBaseOTs);
    // randomize the base OT's choice bits
    baseChoice.resize(numBaseOTs);
    baseChoice.randomize(prngOT);
    // perform the base ot, call sync_wait to block until they have completed.
    coproto::sync_wait(base.receive(baseChoice, baseMsg, prngOT, chl));

    sender.setBaseOts(baseMsg, baseChoice);
    // perform random ots

    sMsgs.resize(numElements);
    coproto::sync_wait(sender.send(sMsgs, prngOT, chl));

}

void softRecv(u32 numElements, BitVector bitV, Socket &chl, PRNG& prng, AlignedVector<block> &rMsgs, u32 numThreads)
{

    SoftSpokenShOtReceiver<> receiver;
    receiver.init(fieldBits, true);
    const size_t numBaseOTs = receiver.baseOtCount();
    AlignedVector<std::array<block, 2>> baseMsg(numBaseOTs);
    PRNG prngOT(prng.get<block>());

    // OTE's receiver is base OT's sender
    DefaultBaseOT base;
    // perform the base ot, call sync_wait to block until they have completed.
    coproto::sync_wait(base.send(baseMsg, prngOT, chl));

    receiver.setBaseOts(baseMsg);

    rMsgs.resize(numElements);
    coproto::sync_wait(receiver.receive(bitV, rMsgs, prngOT, chl));

}

void SendEC25519Points(Socket &chl, std::vector<EC25519Point> &vecA, u32 numThreads) 
{
    u32 size = vecA.size();
    std::vector<u8> buffer(32 * size);	

    #pragma omp parallel for num_threads(numThreads)
    for(auto i = 0; i < size; i++) {
        memcpy(buffer.data() + i * 32, vecA[i].px, 32);  
    }
    coproto::sync_wait(chl.send(buffer));
}

void ReceiveEC25519Points(Socket &chl, std::vector<EC25519Point> &vecA, u32 numThreads) 
{
    u32 size = vecA.size();
    std::vector<u8> buffer(32 * size);
    coproto::sync_wait(chl.recv(buffer));

    #pragma omp parallel for num_threads(numThreads)
    for(u32 i = 0; i < size; ++i)
    {
        memcpy(vecA[i].px, buffer.data() + i * 32, 32);  
    }  
}




// only for subprotocol test
void pECRG(u32 isPI, Socket &chl, std::vector<block> &matrix, u32 rowNum, u32 colNum, std::vector<u32> &pi, std::vector<block> &out, u32 numThreads){

    u32 len = matrix.size();
    assert(len == rowNum * colNum);
    out.resize(len);
    PRNG prng(sysRandomSeed()); 

    if(isPI){
        genPermutation(rowNum, pi);

        std::vector<EC25519Point> vec_Hash_X(len);
        std::vector<EC25519Point> vec_permuted_Fk1_X(len);

        // generate key a
        std::vector<u8> keyA(32);
        prng.get(keyA.data(), keyA.size());

        // H(x[pi[i]])^a
        #pragma omp parallel for num_threads(numThreads)
        for(u32 i = 0; i < len; ++i){
            u32 permuted_i = pi[i % rowNum] + (i/rowNum)* rowNum;
            Hash::BlockToBytes(matrix[permuted_i], vec_Hash_X[i].px, 32); 
            x25519_scalar_mulx(vec_permuted_Fk1_X[i].px, keyA.data(), vec_Hash_X[i].px); 
        }
        // send H(x[pi[i]])^a
        SendEC25519Points(chl, vec_permuted_Fk1_X, numThreads);

        std::vector<EC25519Point> vec_Fk1_Y(len);
        std::vector<EC25519Point> vec_permuted_Fk1k2_Y(len);
        // recv H(y[i])^b
        ReceiveEC25519Points(chl, vec_Fk1_Y, numThreads);

        // std::vector<block> pECRG_out(len);
        #pragma omp parallel for num_threads(numThreads)
        for(u32 i = 0; i < len; ++i){
            u32 permuted_i = pi[i % rowNum] + (i/rowNum)* rowNum;
            x25519_scalar_mulx(vec_permuted_Fk1k2_Y[i].px, keyA.data(), vec_Fk1_Y[permuted_i].px);

            std::vector<u8> outbBytes(32);
            memcpy(outbBytes.data(), vec_permuted_Fk1k2_Y[i].px, 32);
            out[i] = Hash::BytesToBlock(outbBytes);            
        }    
    } 
    else{

        std::vector<EC25519Point> vec_Hash_Y(len);
        std::vector<EC25519Point> vec_Fk1_Y(len);

        // generate key b
        std::vector<u8> keyB(32);
        prng.get(keyB.data(), keyB.size());
        
        // H(y[i])^b
        #pragma omp parallel for num_threads(numThreads)
        for(u32 i = 0; i < len; ++i){
            Hash::BlockToBytes(matrix[i], vec_Hash_Y[i].px, 32); 
            x25519_scalar_mulx(vec_Fk1_Y[i].px, keyB.data(), vec_Hash_Y[i].px); 
        }

        // recv H(x[pi[i]])^a
        std::vector<EC25519Point> vec_permuted_Fk1_X(len);
        ReceiveEC25519Points(chl, vec_permuted_Fk1_X, numThreads);

        // send H(y[i])^b
        SendEC25519Points(chl, vec_Fk1_Y, numThreads);
       
        // compute H((H(x[pi[i]])^a)^b)
        std::vector<EC25519Point> vec_permuted_Fk1k2_X(len);
        // std::vector<block> pECRG_out(len);
        #pragma omp parallel for num_threads(numThreads)
        for(u32 i = 0; i < len; ++i){
            x25519_scalar_mulx(vec_permuted_Fk1k2_X[i].px, keyB.data(), vec_permuted_Fk1_X[i].px);
            std::vector<u8> outbBytes(32);
            memcpy(outbBytes.data(), vec_permuted_Fk1k2_X[i].px, 32);
            out[i] = Hash::BytesToBlock(outbBytes);
        }   
    }
}

// matrix[i] and matrix[i+rowNum] are in the same row
void pnECRG(u32 isPI, Socket &chl, std::vector<block> &matrix, u32 rowNum, u32 colNum, std::vector<u32> &pi, std::vector<block> &out, u32 numThreads){

    u32 len = matrix.size();
    assert(len == rowNum * colNum);
    out.resize(rowNum);
    PRNG prng(sysRandomSeed()); 

    
    u64 keyBitLength = 40 + oc::log2ceil(len);  
    u64 keyByteLength = oc::divCeil(keyBitLength, 8);      

    if(isPI){
        genPermutation(rowNum, pi);

        std::vector<EC25519Point> vec_Hash_X(len);
        std::vector<EC25519Point> vec_permuted_Fk1_X(len);

        // generate key a
        std::vector<u8> keyA(32);
        prng.get(keyA.data(), keyA.size());


        // H(x[pi[i]])^a
        #pragma omp parallel for num_threads(numThreads)
        for(u32 i = 0; i < len; ++i){
            u32 permuted_i = pi[i % rowNum] + (i/rowNum)* rowNum;
            Hash::BlockToBytes(matrix[permuted_i], vec_Hash_X[i].px, 32); 
            x25519_scalar_mulx(vec_permuted_Fk1_X[i].px, keyA.data(), vec_Hash_X[i].px); 
        }
        
        // send H(x[pi[i]])^a
        SendEC25519Points(chl, vec_permuted_Fk1_X, numThreads);

        std::vector<EC25519Point> vec_Fk1_Y(len);
        std::vector<EC25519Point> vec_permuted_Fk1k2_Y(len);
        // recv H(y[i])^b
        ReceiveEC25519Points(chl, vec_Fk1_Y, numThreads);


        std::vector<block> pECRG_out(len);
        #pragma omp parallel for num_threads(numThreads)
        for(u32 i = 0; i < len; ++i){
            u32 permuted_i = pi[i % rowNum] + (i/rowNum)* rowNum ;
            x25519_scalar_mulx(vec_permuted_Fk1k2_Y[i].px, keyA.data(), vec_Fk1_Y[permuted_i].px);

            std::vector<u8> outbBytes(32);
            memcpy(outbBytes.data(), vec_permuted_Fk1k2_Y[i].px, 32);
            pECRG_out[i] = Hash::BytesToBlock(outbBytes);            
        }   


        // nECRG: ssPEQT + ROT
        oc::Matrix<u8> mLabel(len, keyByteLength);
        for(u32 i = 0; i < len; ++i){
            memcpy(&mLabel(i,0), &pECRG_out[i], keyByteLength);
        }    
        BetaCircuit cir = isZeroCircuit(keyBitLength);
        volePSI::Gmw cmp;
        cmp.init(mLabel.rows(), cir, numThreads, 0, prng.get());
        cmp.implSetInput(0, mLabel, mLabel.cols());
        coproto::sync_wait(cmp.run(chl));

        oc::Matrix<u8> mOut;
        mOut.resize(len, 1);
        cmp.getOutput(0, mOut);
        // bitV[i] = mOut[i*colNum] ^ mOut[i*colNum + 1] ... ^ mOut[(i+1) * colNum - 1]  
        BitVector bitV(rowNum);

        for(auto i = 0; i < rowNum; ++i){
            for(auto j = 0; j < colNum; ++j){
                bitV[i] ^= (mOut(j * rowNum + i, 0) & 1); 
            }
        }

        AlignedVector<std::array<block, 2>> sMsgs(rowNum);
        softSend(rowNum, chl, prng, sMsgs, numThreads);

        for(u32 i = 0; i < rowNum; ++i){
            out[i] = sMsgs[i][bitV[i]];
        }  

    } 
    else{

        std::vector<EC25519Point> vec_Hash_Y(len);
        std::vector<EC25519Point> vec_Fk1_Y(len);

        // generate key b
        std::vector<u8> keyB(32);
        prng.get(keyB.data(), keyB.size());
        
        // H(y[i])^b
        // #pragma omp parallel for num_threads(numThreads)
        for(u32 i = 0; i < len; ++i){
            Hash::BlockToBytes(matrix[i], vec_Hash_Y[i].px, 32); 
            x25519_scalar_mulx(vec_Fk1_Y[i].px, keyB.data(), vec_Hash_Y[i].px); 
        }

        // recv H(x[pi[i]])^a
        std::vector<EC25519Point> vec_permuted_Fk1_X(len);
        ReceiveEC25519Points(chl, vec_permuted_Fk1_X, numThreads);

        // send H(y[i])^b
        SendEC25519Points(chl, vec_Fk1_Y, numThreads);
       
        // compute H((H(x[pi[i]])^a)^b)
        std::vector<EC25519Point> vec_permuted_Fk1k2_X(len);
        std::vector<block> pECRG_out(len);
        #pragma omp parallel for num_threads(numThreads)
        for(u32 i = 0; i < len; ++i){
            x25519_scalar_mulx(vec_permuted_Fk1k2_X[i].px, keyB.data(), vec_permuted_Fk1_X[i].px);
            std::vector<u8> outbBytes(32);
            memcpy(outbBytes.data(), vec_permuted_Fk1k2_X[i].px, 32);
            pECRG_out[i] = Hash::BytesToBlock(outbBytes);
        }   

        // nECRG: ssPEQT + ROT
        oc::Matrix<u8> mLabel(len, keyByteLength);
        for(u32 i = 0; i < len; ++i){
            memcpy(&mLabel(i,0), &pECRG_out[i], keyByteLength);
        }    
        BetaCircuit cir = isZeroCircuit(keyBitLength);
        volePSI::Gmw cmp;
        cmp.init(mLabel.rows(), cir, numThreads, 1, prng.get());
        cmp.setInput(0, mLabel);
        coproto::sync_wait(cmp.run(chl));

        oc::Matrix<u8> mOut;
        mOut.resize(len, 1);
        cmp.getOutput(0, mOut);
        // bitV[i] = mOut[i*colNum] ^ mOut[i*colNum + 1] ... ^ mOut[(i+1) * colNum - 1]  
        BitVector bitV(rowNum);
        
        for(auto i = 0; i < rowNum; ++i){
            for(auto j = 0; j < colNum; ++j){
                bitV[i] ^= (mOut(j * rowNum + i, 0) & 1); 
            }
        }

        AlignedVector<block> rMsgs(rowNum);
        softRecv(rowNum, bitV, chl, prng, rMsgs, numThreads);
        memcpy(out.data(), rMsgs.data(), rowNum * sizeof(block));

    }
    return;
}
























