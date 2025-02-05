# ePSU

### Environment

This code and following instructions is tested on Ubuntu 22.04, with g++ 11.4.0 and CMake 3.22.1

### Dependencies

```shell
sudo apt-get update
sudo apt-get install build-essential tar curl zip unzip pkg-config libssl-dev libomp-dev libtool
sudo apt install gcc g++ gdb git make cmake
```

### Notes for Errors on Boost

When building libOTe or volepsi using the command `python3 build.py ...`, the following error may occur:

```
CMake Error at /home/cy/Desktop/tbb/eAPSU/pnECRG_OTP/volepsi/out/coproto/thirdparty/getBoost.cmake:67 (file):
  file DOWNLOAD HASH mismatch

    for file: [/home/cy/Desktop/tbb/eAPSU/pnECRG_OTP/volepsi/out/boost_1_86_0.tar.bz2]
      expected hash: [1bed88e40401b2cb7a1f76d4bab499e352fa4d0c5f31c0dbae64e24d34d7513b]
        actual hash: [79e6d3f986444e5a80afbeccdaf2d1c1cf964baa8d766d20859d653a16c39848]
             status: [0;"No error"]
```

This error is associated with issues in the URL used for downloading Boost.

For the version of volepsi we are using, adjust line 8 in the file `volepsi/out/coproto/thirdparty/getBoost.cmake` to:

```
set(URL "https://archives.boost.io/release/1.86.0/source/boost_1_86_0.tar.bz2")
```

### Installation

```shell
#first download the project
cd balanced_ePSU/

#in balanced_ePSU
git clone https://github.com/openssl/openssl.git
cd openssl/ 
#download the latest OpenSSL from the website, to support curve25519, modify crypto/ec/curve25519.c line 211: remove "static", then compile it:
./Configure no-shared enable-ec_nistp_64_gcc_128 no-ssl2 no-ssl3 no-comp --prefix=/usr/local/openssl
make depend
sudo make install
```

### Compile ePSU

**Hint: When you encounter a hash mismatch error with Boost, you can refer to the "Notes for Errors on Boost" section.**

```shell
#in balanced_ePSU
git clone https://github.com/Visa-Research/volepsi.git
cd volepsi
# compile and install volepsi
python3 build.py -DVOLE_PSI_ENABLE_BOOST=ON -DVOLE_PSI_ENABLE_GMW=ON -DVOLE_PSI_ENABLE_CPSI=OFF -DVOLE_PSI_ENABLE_OPPRF=OFF
python3 build.py --install=../libvolepsi
cp out/build/linux/volePSI/config.h ../libvolepsi/include/volePSI/
cd ..
mkdir build
cd build
cmake ..
make
```

### Test

```shell
#in balanced_ePSU/build
#print balanced ePSU test help information
./test_balanced_epsu -h

#for pECRG
./test_pecrg -nn 12 -nt 1 -r 0 & ./test_pecrg -nn 12 -nt 1 -r 1

#for pMCRG
./test_pmcrg -nn 12 -nt 1 -r 0 & ./test_pmcrg -nn 12 -nt 1 -r 1 

#for nECRG
./test_necrg -nn 12 -nt 1 -r 0 & ./test_necrg -nn 12 -nt 1 -r 1 

#for pnMCRG
./test_pnmcrg -nn 12 -nt 1 -r 0 & ./test_pnmcrg -nn 12 -nt 1 -r 1 

#for balanced ePSU test 
./test_balanced_epsu -nn 12 -nt 1 -r 0 & ./test_balanced_epsu -nn 12 -nt 1 -r 1
```

### Acknowledgments

This project leverages several third-party libraries, some of which have been modified to better suit the needs of this project. Specifically:

**[OPENSSL]** (https://github.com/openssl/openssl.git)

- Modifications
  - Remove "static" of crypto/ec/curve25519.c line 211 to support curve25519.

**[Kunlun]**  (https://github.com/yuchen1024/Kunlun.git)

- Modifications
  - Tailor curve25519 to support pnMCRG.



















