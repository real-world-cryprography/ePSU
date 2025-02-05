// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

// STD
#include <algorithm>
// #include <functional>

// APSU
#include "curve25519.h"
// #include "apsu/util/utils.h"

// FourQ
// #include "apsu/fourq/FourQ_api.h"
// #include "apsu/fourq/FourQ_internal.h"
// #include "apsu/fourq/random.h"

// SEAL
// #include "seal/randomgen.h"
// #include "seal/util/blake2.h"

// using namespace std;
// using namespace seal;





// initialize as a all zero byte array
EC25519Point::EC25519Point(){
    memset(this->px, '0', 32); 
}

EC25519Point::EC25519Point(const EC25519Point& other){
    memcpy(this->px, other.px, 32);
}

EC25519Point::EC25519Point(const uint8_t* buffer){
    memcpy(this->px, buffer, 32);
}

EC25519Point EC25519Point::Mul(const std::vector<uint8_t> scalar) const {
    EC25519Point result; 
    x25519_scalar_mulx(result.px, scalar.data(), this->px); 
    return result;
}

EC25519Point EC25519Point::XOR(const EC25519Point& other) const {  
    EC25519Point result;
    int thread_num = omp_get_thread_num();
    #pragma omp parallel for num_threads(NUMBER_OF_THREADS)
    for(auto i = 0; i < 32; i++){
        result.px[i] = this->px[i]^other.px[i];
    }
    return result; 
}

bool EC25519Point::CompareTo(const EC25519Point& other) const{
    return std::equal(this->px, this->px+32, other.px, other.px+32);
}


// convert an EC25519 Point to byte string
std::string EC25519Point::ToByteString() const
{
    std::string ecp_str(32, '0'); 
    memcpy(&ecp_str[0], this->px, 32); 
    return ecp_str; 
}

// void EC25519Point::Print() const
// { 
//     for(auto i = 0; i < 32; i++) {
//         std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(this->px[i]);
//     } // print in hex string
//     std::cout << std::dec << std::endl; 
// }

// print an EC point with note
// void EC25519Point::Print(std::string note) const
// { 
//     std::cout << note << " = "; 
//     this->Print(); 
// }

std::ofstream &operator<<(std::ofstream &fout, EC25519Point &A)
{ 
    fout.write(reinterpret_cast<char *>(A.px), 32); 
    return fout;            
}

std::ifstream &operator>>(std::ifstream &fin, EC25519Point &A)
{ 
    fin.read(reinterpret_cast<char *>(A.px), 32); 
    return fin;            
}

class EC25519PointHash{
public:
    size_t operator()(const EC25519Point& A) const
    {
        return std::hash<std::string>{}(A.ToByteString());
    }
};

auto EC25519Point_Lexical_Compare = [](EC25519Point A, EC25519Point B){ 
    return A.ToByteString() < B.ToByteString(); 
};



void Hash::cbcEnc(oc::block key, oc::block* data, size_t BLOCK_LEN){
    
    oc::AES aes;
    aes.setKey(key);
    aes.ecbEncBlock(data[0]);
    for (auto i = 1; i < BLOCK_LEN; i++)
    {
        data[i] = _mm_xor_si128(data[i], data[i-1]);
        aes.ecbEncBlock(data[i]);
    }    
}

void Hash::Dedicated_CBCAES(uint8_t* input, uint8_t* output) 
{
    oc::block data[2];
    memcpy(data, input, 2 * sizeof(oc::block));

    oc::block fixed_enc_key = oc::block(0x234876543381cb45,0x2746392748636dba);
    oc::AES aes;
    aes.setKey(fixed_enc_key);
    data[0] = aes.ecbEncBlock(data[0]);
    data[1] ^= data[0];
    data[1] = aes.ecbEncBlock(data[1]);

    memcpy(output, data, 2 * sizeof(oc::block));

}

oc::block Hash::StringToBlock(const std::string &str_input) 
{
    unsigned char output[HASH_OUTPUT_LEN];
    BasicHash(reinterpret_cast<const unsigned char*>(str_input.c_str()), str_input.length(), output); 
    oc::block outblock;
    memcpy(&outblock, output, sizeof(oc::block));
    return outblock;
}

oc::block Hash::BytesToBlock(const std::vector<uint8_t> &vec_A) 
{
    unsigned char output[HASH_OUTPUT_LEN];
    BasicHash(vec_A.data(), vec_A.size(), output); 
    return ((oc::block*)&output[0])[0];
    // return _mm_load_si128((oc::block*)&output[0]);
}



/* 
* hash a oc::block to uint8_t[32]
* must guranttee output[] has at least LEN bytes space 
*/
int Hash::BlockToBytes(const oc::block &var, uint8_t* output, size_t LEN)
{
    if(HASH_OUTPUT_LEN < LEN){
        std::cerr << "digest is too short for desired length" << std::endl;
        return 0;     
    }

    memset(output, 0, 32);
    memcpy(output, &var, 16); // set the block as input of hash
    Dedicated_CBCAES(output, output); 
    //BasicHash(output, 16, output); // compute the hash value

    return 1; 
}