[![Build Status](https://travis-ci.org/finaldie/DNSTurbo.svg?branch=master)](https://travis-ci.org/finaldie/DNSTurbo)

# DNSTurbo
A middleware between browser and local DNS client/server, to speed up the Internet according to the network speed. It optimizes the A/AAAA records by latency detection mechanism, offer the best records the user wanted.

Why not geo based DNS is not enough? As we know, there are a lot of geo based DNS services, like openDNS, google DNS, etc, but unfortunately, they can calculate the distance of the user location to the target host, but it's hard to say the returned records are always the best ones, since the real user experience depends on the user local network quality, different time, different website, the results are totally different.

So, it's better to have another program to detect the quality of the DNS records from user's point of view, then filter the bad ones out, and keep the good ones as much as possible. **DNSTurbo** was born under this situation, plug it into the DNS pipeline, always keep the records as good as possible for the users.

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
Now, the DNSTurbo is all set, let's chain it in DNS pipe. For example, the DNSTurbo is set up in `192.168.31.221`

* **For Mac:**
Open `System Preferences` -> `Network` -> `Advanced` -> `DNS`, then add `192.168.31.221` in the left window.

![DNSTurbo-mac-setting](https://github.com/finaldie/DNSTurbo/wiki/images/mac_dns_setting.png)

* **For Windows7/10:**
Open `Network Apaptor` -> `Property` -> `TCP/IPv4`, then select `Manual DNS` option, and add `192.168.31.221` there.

After that, enjoy the new experience :)
