#!/bin/bash
# OSX default of 3 is not big enough 
sysctl -w net.inet.tcp.win_scale_factor=8
# increase OSX TCP autotuning maximums from 1048576
sysctl -w net.inet.tcp.autorcvbufmax=8361383
sysctl -w net.inet.tcp.autosndbufmax=8361383
# socket buffers
sysctl -w net.inet.tcp.recvspace=65536
sysctl -w net.inet.tcp.sendspace=65536
# slow start
sysctl -w net.inet.tcp.slowstart_flightsize=20
