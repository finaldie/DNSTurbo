[![Build Status](https://travis-ci.org/finaldie/DNSTurbo.svg?branch=master)](https://travis-ci.org/finaldie/DNSTurbo)
![Docker Pulls](https://img.shields.io/docker/pulls/finaldie/dnsturbo.svg)
![license](https://img.shields.io/github/license/finaldie/dnsturbo.svg)
![GitHub release](https://img.shields.io/github/release/finaldie/dnsturbo.svg)
![GitHub tag](https://img.shields.io/github/tag/finaldie/dnsturbo.svg)

# DNSTurbo
A middleware between browser and local DNS client/server, to speed up the Internet according to the network speed. It optimizes the A/AAAA records by latency detection mechanism, offer the best records the user wanted.

Why the GEO-based DNS is not enough? As we know, there are a lot of GEO-based DNS services, like openDNS, google DNS, etc. They can calculate the distance of the user location to the target host, to return the nearest records, but unfortunately it's hard to say the returned records are always the best ones, since the real user experience depends on the user local network quality, for the different time, different ISP and different website, the results are totally different.

So, it's better to have another program to detect the quality of the DNS records from user's point of view, then filter the bad ones out, and keep the good ones as much as possible. **DNSTurbo** was born under this situation, plug it into the DNS pipeline, always keep the records as good as possible for the users.

**DNSTurbo** do the _Latency Detection_, and does not do the _Bandwidth Detection_, because it's highly depend on the **Congestion Control**(For TCP connections), different congestion module would give the different results, and the _Bandwidth Detection_ is not only very heavy especially for the mobile device in _LTE_ network, but also would compete with _Browser_ to make the network worse than ever. And another hand, all the records returned should have the very similar network quality, so to maximize the _Bandwidth_, we should rely on the target program or *TCP Congestion Control* (Like Linux-4.9, the new **CC** module would try the best to use the _Bandwidth_ by the new algo)

## Trailer
* <a href="https://www.youtube.com/watch?v=2u_-Wl7sDdA" target="_blank">Click here to watch the Trailer on Youtube</a>
* <a href="http://v.youku.com/v_show/id_XMTk3NDE2NDUwOA==.html?spm=a2hww.20023042.uerCenter.5!2~5~5!2~5~DL~DD~A.m37E3S" target="_blank">Click here to watch the Trailer on Youku</a>

![DNSTurbo Logo](https://github.com/finaldie/DNSTurbo/wiki/images/Dnsturbo-logo.jpg)

## Architecture

![DNSTurbo-arch](https://github.com/finaldie/DNSTurbo/wiki/images/DNSTurbo_arch_2.png)

## Compatible Environments
 - [x] Linux
 - [x] 32/64 bit compatible
 - [x] x86/ARM compatible

## Features
* Builtin
 - [x] Optimized A Record
 - [x] Optimized AAAA Record
 - [x] Http Latency Detection
 - [x] [Docker Image][3]

* Coming Soon:
 - [ ] ICMP Latency Detection

## Install Dependencies
```console
# Install System Dependencies
sudo apt-get install autoconf valgrind expect libyaml-dev python3-dev python3-pip libc-ares-dev;
sudo pip3 install PyYAML pympler WebOb dnslib;

# Fork/Clone and Build Project Dependencies (For example: project folder is 'DNSTurbo')
cd DNSTurbo
git submodule update --init --recursive
make dep; sudo make install-dep; make skull; sudo make install-skull;
```

## Build
```console
skull build && skull deploy
```

## Start
```console
sudo skull start -D
```

## Docker Images
The [**_Docker images_**][3] are ready now, if people who don't want to waste time to build it from scratch, it's super easy to run it via docker image.

Assume that we've already [installed _Docker_][1], then apply the below commands:
```console
bash$> docker pull finaldie/dnsturbo:0.7
bash$> docker run -p 53:53/udp finaldie/dnsturbo:0.7
```

![DNSTurbo-Docker][2]

## Setup
Now, the DNSTurbo is all set, let's chain it in DNS pipe. For example, the DNSTurbo is set up in `192.168.31.221`

* **For Mac:**
Open `System Preferences` -> `Network` -> `Advanced` -> `DNS`, then add `192.168.31.221` in the left window.

![DNSTurbo-mac-setting](https://github.com/finaldie/DNSTurbo/wiki/images/mac_dns_setting.png)

* **For Windows7/10:**
Open `Network Apaptor` -> `Property` -> `TCP/IPv4`, then select `Manual DNS` option, and add `192.168.31.221` there.

After that, enjoy the new experience :)

## Environment Variables
Sometimes we don't want to use the default upstream DNS from `resolv.conf`, then there is an `Shell Environment Variable` can override it, just define an ENV var **_SKULL_DNS_NS_**, e.g.:
```console
export SKULL_DNS_NS=192.168.1.1
export SKULL_DNS_NS=8.8.8.8
```

[1]: https://docs.docker.com/engine/installation/linux/docker-ce/ubuntu/
[2]: https://github.com/finaldie/DNSTurbo/wiki/images/dnsturbo-docker.png
[3]: https://hub.docker.com/r/finaldie/dnsturbo/
