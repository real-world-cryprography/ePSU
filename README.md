# ePSU

## Compiling the ePSU Locally

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

and for the version of libOTe we are using, adjust line 8 in the file `libOTe/out/coproto/thirdparty/getBoost.cmake` to:

```
set(URL "https://archives.boost.io/release/1.84.0/source/boost_1_84_0.tar.bz2")
```

### balanced_ePSU

#### Installation

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

#### Compile balanced_ePSU

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

#### Test for balanced_ePSU

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

### unbalanced_ePSU

#### Installation 

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

#### Compile unbalanced_ePSU

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

#### Test for unbalanced_ePSU

```shell
#in unbalanced_ePSU
#print help information
python3 test.py -h
```

##### Flags:

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

##### Examples: 

``` bash
#Run MCRG + pECRG_nECRG_OTP with set size `2^12`:
python3 test.py -pecrg_necrg_otp -cn 1 -nt 1 -nn 12

#Run MCRG + pECRG with set size `2^12`:
python3 test.py -pecrg -cn 1 -nt 1 -nn 12

#Run MCRG + pnECRG with set size `2^12`:
python3 test.py -pnecrg -cn 1 -nt 1 -nn 12
```

## Docker Quick Start

Docker makes it easy to create, deploy, and run applications by using containers. Here are some quick tips to get you started with Docker:

### Prerequisites

- Ensure you have Docker installed on your machine. You can download Docker from the [official website](https://www.docker.com/products/docker-desktop).

### Pulling the docker image

To pull the Docker image, use the following command:

```bash
docker pull kafei2cy/epsu:v1.2.0
```

### Running a Docker Container

To run a Docker container from the image you pulled and access the Container Shell, use the following command:

```sh
docker run -it --name your-container-name kafei2cy/epsu:v1.2.0 /bin/bash

#if you want use tc to change network setting, use the following command
docker run -it --cap-add=NET_ADMIN --name your-container-name kafei2cy/epsu:v1.2.0 /bin/bash
```

- `--name your-container-name` gives your container a name for easier reference.

### Test for ePSU

```bash
#Test for balanced_ePSU
cd /home/ePSU/balanced_ePSU/build

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

#Test for unbalanced_ePSU
cd /home/ePSU/unbalanced_ePSU

python3 test.py -h

#Run MCRG + pECRG_nECRG_OTP with set size `2^12`:
python3 test.py -pecrg_necrg_otp -cn 1 -nt 1 -nn 12

#Run MCRG + pECRG with set size `2^12`:
python3 test.py -pecrg -cn 1 -nt 1 -nn 12

#Run MCRG + pnECRG with set size `2^12`:
python3 test.py -pnecrg -cn 1 -nt 1 -nn 12
```

### Stopping and Removing a Docker Container

To stop a running container, use the following command:

```sh
docker stop your-container-name
```

To remove a stopped container, use the following command:

```sh
docker rm your-container-name
```

## Automated Testing

We also provide automated scripts to test both balanced_ePSU and unbalanced_ePSU, allowing for quick and efficient evaluation across different configurations. If you use the docker image we provide, you just need to choose the network setting and run the script directly in the corresponding folder.

If your host machine is running Windows, note that Docker Desktop on Windows uses WSL2 (Windows Subsystem for Linux) by default (can not use tc). A more effective approach is to leverage application-level tools like **Clumsy** or other proxy-based solutions for network emulation.

### Change The Network Setting

```bash
#If you are using the docker image we provide, you can use the following command without 'sudo'
#change the network setting，for example:
sudo tc qdisc add dev lo root netem delay 80ms rate 400Mbit

#delete the setting
sudo tc qdisc del dev lo root
```

### balanced_ePSU Auto Test

```bash
#in ePSU/auto_test_shell
#copy the script to ePSU/balanced_ePSU/build
cp balancedAutoTest.sh ../balanced_ePSU/build

#in ePSU/balanced_ePSU/build
chmod +x balancedAutoTest.sh

#if you use the docker image 
#run the script, for example:
./balancedAutoTest.sh -protocol psu -start 10 -end 12 -step 2 -nt 4
```

#### Script Parameters Overview

##### 1. `-protocol [NAME]`

- **Purpose**: Select the protocol to test

- **Valid Options**:

  ```bash
  pecrg        # pECRG protocol
  pmcrg        # pMCRG protocol
  necrg        # nECRG protocol
  pnmcrg       # pnMCRG protocol
  psu          # balanced_ePSU protocol
  ```

- **Default**: `psu`

##### 2. `-nt [VALUE]`

- **Purpose**: Set the number of threads
- **Type**: Positive integer
- **Default**: `4`

##### 3. `-start [EXPONENT]`

- **Purpose**: Define the starting exponent for dataset size (2^N)
- **Type**: $\text{Integer} \ge 10$
- **Default**: `10` ($2^{10}$ = 1024 elements)

##### 4. `-end [EXPONENT]`

- **Purpose**: Define the ending exponent for dataset size (2^N)
- **Type**: $\text{start} \le \text{Integer} \le 20$
- **Default**: `20` ($2^{20}$= 1,048,576 elements)

##### **5. `-step [VALUE]`**

- **Purpose**: Set increment step between exponents
- **Type**: Positive integer
- **Default**: `2`

### unbalanced_ePSU Auto Test

```shell
#in ePSU/auto_test_shell
#copy the script to ePSU/unbalanced_ePSU
cp unbalancedAutoTest.sh ../unbalanced_ePSU/

#in ePSU/unbalanced_ePSU
chmod +x unbalancedAutoTest.sh

#run the script, for example:
./unbalancedAutoTest.sh -protocol pecrg_necrg_otp -start 10 -end 12 -step 2 -nt 4
```

#### Script Parameters Overview

##### 1. `-protocol [NAME]`

- **Purpose**: Select the protocol to test

- **Valid Options**:

  ```bash
  pecrg_necrg_otp       # unbalanced_ePSU protocol
  pecrg                 # pECRG protocol
  pnecrg                # pnECRG protocol
  ```

- **Default**: `pecrg_necrg_otp`

##### 2. `-nt [VALUE]`

- **Purpose**: Set the number of threads
- **Type**: Positive integer
- **Default**: `4`

##### 3. `-start [EXPONENT]`

- **Purpose**: Define the starting exponent for dataset size (2^N)
- **Type**: $\text{Integer} \ge 10$
- **Default**: `10` ($2^{10}$ = 1024 elements)

##### 4. `-end [EXPONENT]`

- **Purpose**: Define the ending exponent for dataset size (2^N)
- **Type**: $\text{start} \le \text{Integer} \le 20$
- **Default**: `20` ($2^{20}$= 1,048,576 elements)

##### **5. `-step [VALUE]`**

- **Purpose**: Set increment step between exponents
- **Type**: Positive integer
- **Default**: `2`

## Acknowledgments

This project leverages several third-party libraries, some of which have been modified to better suit the needs of this project. Specifically:

**[APSU]**  (https://github.com/real-world-cryprography/APSU.git)

- Modifications
  - Remove Kunlun and OSN related codes.
  - Write the immediate values into specified files.

**[OPENSSL]**  (https://github.com/openssl/openssl.git)

- Modifications
  - Remove "static" of crypto/ec/curve25519.c line 211 to support curve25519.

**[Kunlun]**   (https://github.com/yuchen1024/Kunlun.git)

- Modifications
  - Tailor curve25519 to support pnMCRG.

