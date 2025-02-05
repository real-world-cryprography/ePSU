// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "global.h"
#include "cryptoTools/Common/block.h"
#include "cryptoTools/Crypto/AES.h"

inline const size_t HASH_BUFFER_SIZE = 1024*8;
inline const size_t HASH_OUTPUT_LEN = 32;  // hash output = 256-bit string

//#define BasicHash(input, HASH_INPUT_LEN, output) SM3(input, HASH_INPUT_LEN, output)
#define BasicHash(input, HASH_INPUT_LEN, output) SHA256(input, HASH_INPUT_LEN, output)



extern "C"
{
    void x25519_scalar_mulx(uint8_t out[32], const uint8_t scalar[32], const uint8_t point[32]);
}



 
class EC25519Point {
public:
    uint8_t px[32];

    // constructor functions
    EC25519Point(); 
    EC25519Point(const EC25519Point& other);
    
    // Creates an ECPoint object with given x, y affine coordinates.
    EC25519Point(const uint8_t* buffer);

    // EC point group operations
    
    // Returns an ECPoint whose value is (this * scalar).
    EC25519Point Mul(const std::vector<uint8_t> scalar) const;

    // Returns an ECPoint whose value is (this + other).
    EC25519Point XOR(const EC25519Point& other) const;

    // Returns true if this equals point, false otherwise.
    bool CompareTo(const EC25519Point& point) const;

    inline EC25519Point& operator=(const EC25519Point& other) {
        memcpy(this->px, other.px, 32); 
        return *this; 
    }

    inline std::string ToByteString() const;
    
    inline bool operator==(const EC25519Point& other) const{ return this->CompareTo(other); }

    inline bool operator!=(const EC25519Point& other) const{ return !this->CompareTo(other);}

    inline EC25519Point operator*(const std::vector<uint8_t> scalar) const { return this->Mul(scalar); }

    inline EC25519Point operator^(const EC25519Point& other) const { return this->XOR(other); }

    inline EC25519Point& operator*=(const std::vector<uint8_t> scalar) { return *this = *this * scalar; }

    inline EC25519Point& operator^=(const EC25519Point& other) { return *this = *this ^ other; }

    void Print() const;

    void Print(std::string note) const;  


    friend std::ofstream &operator<<(std::ofstream &fout, EC25519Point &A); 

    friend std::ifstream &operator>>(std::ifstream &fin, EC25519Point &A);

// private:
//     // Initialize to neutral element
//     point_t pt_ = { { { { 0 } }, { { 1 } } } }; // { {.x = { 0 }, .y = { 1 } }};
// };                                              // class ECPoint
};

namespace Hash{
    __attribute__((target("aes,sse2")))
    // dedicated CBCAES that hash 32 bytes input to 32 bytes output
    void cbcEnc(oc::block key, oc::block* data, size_t BLOCK_LEN);

    void Dedicated_CBCAES(uint8_t* input, uint8_t* output);

    __attribute__((target("sse2")))
    oc::block StringToBlock(const std::string &str_input);
    __attribute__((target("sse2")))

    oc::block BytesToBlock(const std::vector<uint8_t> &vec_A);



    /* 
    * hash a oc::block to uint8_t[32]
    * must guranttee output[] has at least LEN bytes space 
    */
    int BlockToBytes(const oc::block &var, uint8_t* output, size_t LEN);



}

