#include "pECRG_nECRG_OTP.h"


void pECRG_nECRG_OTP(u32 isSender, u32 numThreads)
{
    u64 item_cnt;
    u64 alpha_max_cache_count;
    std::ifstream randomMFile;
    Timer timer;
    timer.setTimePoint("start");  

    Socket chl;
    chl = coproto::asioConnect("localhost:" + std::to_string(1212 + 101), isSender);

    if(isSender == 1){

        std::string filePath = "../../MCRG/build/randomM/sender_cuckoo";
        if (!filePath.empty()){
            randomMFile.open(filePath, std::ios::binary | std::ios::in);
            if (!randomMFile.is_open()){
                return;
            }
        }
        randomMFile.read((char*)(&item_cnt), sizeof(uint64_t));
        randomMFile.read((char*)(&alpha_max_cache_count), sizeof(uint64_t));
        std::vector<block> decrypt_randoms_matrix(item_cnt * alpha_max_cache_count);
        std::vector<block> cuckoo_item(item_cnt);

        randomMFile.read((char*)decrypt_randoms_matrix.data(), sizeof(block) * decrypt_randoms_matrix.size());
        randomMFile.read((char*)cuckoo_item.data(), sizeof(block) * cuckoo_item.size());
        randomMFile.close();
        std::vector<uint32_t> pi;  
        std::vector<block> pnECRG_out; 
        pnECRG(1, chl, decrypt_randoms_matrix, item_cnt, alpha_max_cache_count, pi, pnECRG_out, numThreads);

        // shuffle cuckoo table and XOR pnECRG_out
        // one time padding
        std::vector<block> shuffle_item(item_cnt);
        for(int i = 0; i < item_cnt; i++){
            if(cuckoo_item[pi[i]] == block(0,0)){
            	shuffle_item[i] = pnECRG_out[i];
            }
            else{
            	shuffle_item[i] = oc::block(cuckoo_item[pi[i]].mData[1], 1) ^ pnECRG_out[i];
            }
            
        }                         
        coproto::sync_wait(chl.send(shuffle_item)); 
        timer.setTimePoint("end"); 
        std::cout << timer << std::endl;

    }else if (isSender == 0){
        std::string filePath = "../../MCRG/build/randomM/receiver_pi";
        if (!filePath.empty()){
            randomMFile.open(filePath, std::ios::binary | std::ios::in);
            if (!randomMFile.is_open()){
                return;
            }
        }
        randomMFile.read((char*)(&item_cnt), sizeof(uint64_t));
        randomMFile.read((char*)(&alpha_max_cache_count), sizeof(uint64_t));

        std::vector<block> random_matrix(item_cnt * alpha_max_cache_count);

        randomMFile.read((char*)random_matrix.data(), sizeof(block) * random_matrix.size());
        randomMFile.close();
        std::vector<uint32_t> pi;  // useless
        std::vector<block> pnECRG_out; 
        pnECRG(0, chl, random_matrix, item_cnt, alpha_max_cache_count, pi, pnECRG_out, numThreads);
        std::vector<oc::block> shuffle_item(item_cnt);
        coproto::sync_wait(chl.recv(shuffle_item));  

        // cause the receiver knows its input set, here we only need to know all the items in X/Y.
        std::ofstream fout;
        fout.open("union.csv",std::ofstream::out);
        for(auto i = 0; i < item_cnt; ++i){
            auto tmp_block = pnECRG_out[i] ^ shuffle_item[i];
            if(tmp_block.mData[0] == 1){
                fout << tmp_block.mData[1] << std::endl;
            }
        }
        fout.close();
        // u32 union_sub_receiver = 0;
        // for(auto i = 0; i < item_cnt; ++i){
        //     auto tmp_block = pnECRG_out[i] ^ shuffle_item[i];
        //     if(tmp_block.mData[0] == 1){
        //         union_sub_receiver += 1;
        //     }
        // }
        // std::cout << "union sub receiver size: " << union_sub_receiver << std::endl;


        timer.setTimePoint("end"); 
        std::cout << timer << std::endl;

        double comm = 0;
        comm += chl.bytesSent() + chl.bytesReceived();

        std::cout << "Comm cost = " << std::fixed << std::setprecision(3) << comm / 1024 / 1024 << " MB" << std::endl;
    }
    coproto::sync_wait(chl.flush());
    coproto::sync_wait(chl.close());    
}

