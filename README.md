# balanced_ePSU

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



# unbalanced_ePSU

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

For the version of libOTe we are working with, it is necessary to modify line 8 in the file `libOTe/out/coproto/thirdparty/getBoost.cmake` to the following:

```
set(URL "https://archives.boost.io/release/1.84.0/source/boost_1_84_0.tar.bz2")
```

Similarly, for the version of volepsi we are using, adjust line 8 in the file `volepsi/out/coproto/thirdparty/getBoost.cmake` to:

```
set(URL "https://archives.boost.io/release/1.86.0/source/boost_1_86_0.tar.bz2")
```

### Installation

```shell
#first download the project
cd unbalanced_ePSU/

#in unbalanced_ePSU
git clone https://github.com/microsoft/vcpkg
cd vcpkg/
./bootstrap-vcpkg.sh
./vcpkg install seal[no-throw-tran]
./vcpkg install kuku
./vcpkg install openssl
./vcpkg install log4cplus
./vcpkg install cppzmq
./vcpkg install flatbuffers
./vcpkg install jsoncpp
./vcpkg install tclap

#in unbalanced_ePSU
git clone --recursive https://github.com/osu-crypto/libOTe.git
cd libOTe
git checkout b216559
git submodule update --recursive --init 
python3 build.py --all --boost --relic
git submodule update --recursive --init 
sudo python3 build.py -DENABLE_SODIUM=OFF -DENABLE_MRR_TWIST=OFF -DENABLE_RELIC=ON --install=/usr/local/libOTe

#in unbalanced_ePSU
git clone https://github.com/openssl/openssl.git
cd openssl/ 
#download the latest OpenSSL from the website, to support curve25519, modify crypto/ec/curve25519.c line 211: remove "static", then compile it:
./Configure no-shared enable-ec_nistp_64_gcc_128 no-ssl2 no-ssl3 no-comp --prefix=/usr/local/openssl
make depend
sudo make install
```

### Compile eAPSU

**Hint: When you encounter a hash mismatch error with Boost, you can refer to the "Notes for Errors on Boost" section.**

```shell
#in unbalanced_ePSU/MCRG
mkdir build
cd build
cmake .. -DLIBOTE_PATH=/usr/local/ -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake 
cmake --build .

#in unbalanced_ePSU/pECRG_nECRG_OTP
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

**Step 1:**

```shell
#in unbalanced_ePSU
#print help information
python3 test.py -h
```

#### Flags:

    usage: test.py [-h] [-pecrg] [-pnecrg] [-pnecrgotp] -cn CN [-nt NT] [-nn NN]
    
    options:
      -h, --help  show this help message and exit
    
    Protocol:
      Select one of the following protocols:
    
      -pecrg      Enable the pECRG protocol
      -pnecrg     Enable the pnECRG protocol
      -pecrg_necrg_otp Enable the pECRG_nECRG_OTP protocol
    
    Parameters:
      Configuration for protocol execution:
    
      -cn CN      If the number of elements in each set less than 2^20, set to 1; otherwise, set to 2.
      -nt NT      Number of threads, default 1
      -nn NN      Logarithm of set size, default 12

#### Examples: 



``` bash
#Run MCRG + pECRG_nECRG_OTP with set size `2^12`:
python3 test.py -pecrg_necrg_otp -cn 1 -nt 1 -nn 12

#Run MCRG + pECRG with set size `2^12`:
python3 test.py -pecrg -cn 1 -nt 1 -nn 12

#Run MCRG + pnECRG with set size `2^12`:
python3 test.py -pnecrg -cn 1 -nt 1 -nn 12
```

### Acknowledgments

This project leverages several third-party libraries, some of which have been modified to better suit the needs of this project. Specifically:

**[APSU]** (https://github.com/real-world-cryprography/APSU.git)

- Modifications
  - Remove Kunlun and OSN related codes.
  - Write the immediate values into specified files.

**[OPENSSL]** (https://github.com/openssl/openssl.git)

- Modifications
  - Remove "static" of crypto/ec/curve25519.c line 211 to support curve25519.

**[Kunlun]**  (https://github.com/yuchen1024/Kunlun.git)

- Modifications
  - Tailor curve25519 to support pnMCRG.



































