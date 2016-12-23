[![Build Status](https://travis-ci.org/finaldie/DNSTurbo.svg?branch=master)](https://travis-ci.org/finaldie/DNSTurbo)

# DNSTurbo
DNS Turbo - A middleware between browser and local DNS client/server, to optimize the records based on the network speed.

## Architecture

![DNSTurbo-arch](https://github.com/finaldie/DNSTurbo/wiki/images/DNSTurbo_arch_2.png)

## Environment
 - [x] Linux
 - [x] 32/64 bit compatible
 - [x] x86/ARM compatible

## Features
* Builtin
 - [x] Optimized A Record
 - [x] Optimized AAAA Record
 - [x] Http Latency Detection

* Coming Soon:
 - [ ] ICMP Latency Detection

## Install Dependencies
```console
# Install System Dependencies
sudo apt-get install autoconf valgrind expect libyaml-dev python-dev python-pip libprotobuf-dev protobuf-compiler libprotobuf-c0-dev protobuf-c-compiler
sudo pip install PyYAML protobuf pympler WebOb dnslib

# Fork/Clone and Build Project Dependencies (For example: project folder is 'DNSTurbo')
cd DNSTurbo
git submodule update --init --recursive
make dep
sudo make install-dep
```

## Build
```console
skull build && skull deploy
```

## Start
```console
sudo skull start -D
```

## Setup
Now, the DNSTurbo is all set, let's chain it in DNS pipe. For example, the DNSTurbo is set up in `192.168.1.100`

* **For Mac:**
Open `System Preferences` -> `Network` -> `Advanced` -> `DNS`, then add `192.168.1.100` in the left window.

* **For Windows7:**
Open `Network Apaptor` -> `Property` -> `TCP/IPv4`, then select `Manual DNS` option, and add `192.168.1.100` there.

After that, enjoy the new experience :)
