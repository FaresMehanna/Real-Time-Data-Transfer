#!/bin/bash

sysctl -w net.core.default_qdisc=fq
sysctl -w net.ipv4.tcp_congestion_control=cubic
sysctl -w net.core.wmem_max=8361383
sysctl -w net.core.rmem_max=8361383
sysctl -w net.ipv4.tcp_wmem="4096 65536 8361383"
sysctl -w net.ipv4.tcp_rmem="4096 65536 8361383"
sysctl -w net.ipv4.tcp_window_scaling=1