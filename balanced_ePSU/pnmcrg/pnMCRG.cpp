
#include "pnMCRG.h"

void genPermutation(u32 size, std::vector<u32> &pi)
{
    pi.resize(size);
    for (size_t i = 0; i < pi.size(); ++i){
        pi[i] = i;
    }
    std::shuffle(pi.begin(), pi.end(), global_built_in_prg2);
    return;
}

void permute(std::vector<u32> &pi, std::vector<block> &data){
    std::vector<block> res(data.size());
    for (size_t i = 0; i < pi.size(); ++i){
        res[i] = data[pi[i]];
    }
    data.assign(res.begin(), res.end());
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

void nECRG(u32 idx, std::vector<block> &input, std::vector<block> &out, Socket &chl, u32 numThreads)
{
    u32 numBins = input.size();
    out.resize(numBins);
    PRNG prng(sysRandomSeed());
    bool isSender = true;
    if(idx == 1) isSender = false;

    BitVector bitV;
    ssPEQT(idx, input, bitV, chl, numThreads);

    AlignedVector<std::array<block, 2>> sMsgs(numBins);
    AlignedVector<block> rMsgs(numBins);

    ssROT(isSender, numBins, chl, bitV, out, prng, numThreads);

    return;
}



void pECRG(u32 isPi, std::vector<block> &set, std::vector<block> &out, std::vector<u32> &pi, Socket &chl, u32 numThreads)
{
    u32 numElements = set.size();
    out.resize(numElements);
    PRNG prng(sysRandomSeed());
    // P1 sample a permutation and a key for pOPRF
    if(isPi){
        // generate permutation pi
        pi.resize(numElements);
        genPermutation(numElements, pi);

        std::vector<EC25519Point> vec_Hash_X(numElements);
        std::vector<EC25519Point> vec_permuted_Fk1_X(numElements);

        // generate key a
        std::vector<u8> keyA(32);
        prng.get(keyA.data(), keyA.size());
        
        // H(x[pi[i]])^a
        #pragma omp parallel for num_threads(numThreads)
        for(u32 i = 0; i < numElements; ++i){
            Hash::BlockToBytes(set[pi[i]], vec_Hash_X[i].px, 32); 
            x25519_scalar_mulx(vec_permuted_Fk1_X[i].px, keyA.data(), vec_Hash_X[i].px); 
        }
        // send H(x[pi[i]])^a
        SendEC25519Points(chl, vec_permuted_Fk1_X, numThreads);

        std::vector<EC25519Point> vec_Fk1_Y(numElements);
        std::vector<EC25519Point> vec_permuted_Fk1k2_Y(numElements);
        // recv H(y[i])^b
        ReceiveEC25519Points(chl, vec_Fk1_Y, numThreads);

        #pragma omp parallel for num_threads(numThreads)
        for(u32 i = 0; i < numElements; ++i){
            x25519_scalar_mulx(vec_permuted_Fk1k2_Y[i].px, keyA.data(), vec_Fk1_Y[pi[i]].px);

            std::vector<u8> outbBytes(32);
            memcpy(outbBytes.data(), vec_permuted_Fk1k2_Y[i].px, 32);
            out[i] = Hash::BytesToBlock(outbBytes);            
        }
    }
    else{

        std::vector<EC25519Point> vec_Hash_Y(numElements);
        std::vector<EC25519Point> vec_Fk1_Y(numElements);

        // generate key b
        std::vector<u8> keyB(32);
        prng.get(keyB.data(), keyB.size());
        
        // H(y[i])^b
        #pragma omp parallel for num_threads(numThreads)
        for(u32 i = 0; i < numElements; ++i){
            Hash::BlockToBytes(set[i], vec_Hash_Y[i].px, 32); 
            x25519_scalar_mulx(vec_Fk1_Y[i].px, keyB.data(), vec_Hash_Y[i].px); 
        }

        // recv H(x[pi[i]])^a
        std::vector<EC25519Point> vec_permuted_Fk1_X(numElements);
        ReceiveEC25519Points(chl, vec_permuted_Fk1_X, numThreads);

        // send H(y[i])^b
        SendEC25519Points(chl, vec_Fk1_Y, numThreads);

        
        // compute H((H(x[pi[i]])^a)^b)
        std::vector<EC25519Point> vec_permuted_Fk1k2_X(numElements);
        #pragma omp parallel for num_threads(numThreads)
        for(u32 i = 0; i < numElements; ++i){
            x25519_scalar_mulx(vec_permuted_Fk1k2_X[i].px, keyB.data(), vec_permuted_Fk1_X[i].px);
            std::vector<u8> outbBytes(32);
            memcpy(outbBytes.data(), vec_permuted_Fk1k2_X[i].px, 32);
            out[i] = Hash::BytesToBlock(outbBytes);
        }        

    }
    return;
}



void pMCRG(u32 idx, u32 numElements, std::vector<block> &set, std::vector<block> &out, std::vector<block> &permutedX0, Socket &chl, u32 numThreads)
{
    oc::CuckooParam params = oc::CuckooIndex<>::selectParams(numElements, ssp, 0, 3);
    u32 numBins = params.numBins();
    out.resize(numBins);
    block cuckooSeed = block(0x235677879795a931, 0x784915879d3e658a); 

    PRNG prng(sysRandomSeed());
    block hashSeed = block(0x12387ab67853d29e, 0x58735185628bfea4);

    Baxos mPaxos;
    mPaxos.init(3 * numElements, binSize, 3, ssp, PaxosParam::GF128, block(0,0));
    u32 okvs_size = mPaxos.size();

    std::vector<block> t_lable(numBins);
    std::vector<block> s_lable(numBins);
    std::vector<block> P(okvs_size); 
    std::vector<u32> pi(numBins);
    
    // P_idx run batch OPPRF with P_oidx
    if(idx == 0){
        // establish cuckoo hash table
        oc::CuckooIndex cuckoo;
        cuckoo.init(numElements, ssp, 0, 3);
        cuckoo.insert(set, cuckooSeed);

        // get mA mC of vole: a+b = c*d
        oc::SilentVoleReceiver<block, block, oc::CoeffCtxGF128> mVoleRecver;
        mVoleRecver.mMalType = SilentSecType::SemiHonest;
        mVoleRecver.configure(numBins, SilentBaseType::Base);
        AlignedUnVector<block> mA(numBins);
        AlignedUnVector<block> mC(numBins);
        coproto::sync_wait(mVoleRecver.silentReceive(mC, mA, prng, chl));


        // establish cuckoo hash table, compute diffC cuckooHashTable
        oc::AES hasher;
        hasher.setKey(cuckooSeed);    
        
        std::vector<block> diffC(numBins); 
        std::vector<block> keys(numBins);
        std::vector<block> values(numBins);
        permutedX0.resize(numBins); //set x||0

        for (u32 i = 0; i < numBins; ++i)
        {
            auto bin = cuckoo.mBins[i];

            if (bin.isEmpty() == false)
            {
                auto j = bin.hashIdx();
                auto b = bin.idx();
                block xj = block(set[b].mData[0], j);//compute x||z             
                keys[i] = xj;  

                permutedX0[i] = block(set[b].mData[0], 1); 
                diffC[i] = xj ^ mC[i];                                                      
            }
            else
            {          	          	
                keys[i] = prng.get(); 
                diffC[i] = mC[i];
            } 
                                             
        }

        coproto::sync_wait(chl.send(diffC));
        coproto::sync_wait(chl.recv(P));      
        mPaxos.decode<block>(keys, values, P, 1);  

        for (u32 i = 0; i < numBins; ++i)
        {
            s_lable[i] = hasher.hashBlock(mA[i]) ^ values[i];       
        }        

        //run pECRG
        pECRG(1, s_lable, out, pi, chl, numThreads);
        permute(pi, permutedX0);

    }
    else if(idx == 1){

    	// establish simple hash table
        volePSI::SimpleIndex sIdx;
    	sIdx.init(numBins, numElements, ssp, 3);
    	sIdx.insertItems(set, cuckooSeed);   

        // get vole : a + b  = c * d
        block mD = prng.get();
        oc::SilentVoleSender<block,block, oc::CoeffCtxGF128> mVoleSender;
        mVoleSender.mMalType = SilentSecType::SemiHonest;
        mVoleSender.configure(numBins, SilentBaseType::Base);
        AlignedUnVector<block> mB(numBins);
        coproto::sync_wait(mVoleSender.silentSend(mD, mB, prng, chl));

        // diffC received
        std::vector<block> diffC(numBins);
        coproto::sync_wait(chl.recv(diffC));
        // set for PRF(k, x||z)    
        oc::AES hasher;
        hasher.setKey(cuckooSeed);


        std::vector<block> keys(numElements * 3);
        std::vector<block> values(numElements * 3);
        u32 countV = 0;
        prng.get(t_lable.data(), numBins);
    	for (u32 i = 0; i < numBins; ++i)
    	{
    	    auto bin = sIdx.mBins[i];
    	    auto size = sIdx.mBinSizes[i];
    	    
    	    for (u32 p = 0; p < size; ++p)
    	    {
    	    	auto j = bin[p].hashIdx();
    	    	auto b = bin[p].idx();
   	    	
                block yj = block(set[b].mData[0], j);//compute y||j
                keys[countV] = yj; 

                yj ^= diffC[i];
                auto tmp = mB[i] ^ (yj.gf128Mul(mD));
                tmp = hasher.hashBlock(tmp);

                values[countV] = tmp ^ t_lable[i];  
                countV += 1;   	    	   	    
    	    }    	        	        	        	
    	} 
        
        // std::vector<block> P(okvs_size);    
        mPaxos.solve<block>(keys, values, P, nullptr, 1);
        coproto::sync_wait(chl.send(P));

        pECRG(0, t_lable, out, pi, chl, numThreads);

    }
    return;
}    

// pnMCRG = pMCRG + nECRG
void pnMCRG(u32 idx, u32 numElements, std::vector<block> &set, std::vector<block> &out, std::vector<block> &permutedX0, Socket &chl, u32 numThreads)
{
    // Timer timer;
    // timer.setTimePoint("start");

    std::vector<block> mcrg_out;
    pMCRG(idx, numElements, set, mcrg_out, permutedX0, chl, numThreads);
    // timer.setTimePoint("pMCRG");

    nECRG(idx, mcrg_out, out, chl, numThreads);
    // timer.setTimePoint("nECRG");
    // if(idx == 1){
    //     std::cout << timer << std::endl;
    // }
}



