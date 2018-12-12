# How to test

- Clone the repo to both devices, the camera and the receiver.
- Run tcp_tuning_{os}.sh from /scripts/ folder in both your devices using sudo.
- It's recommended to run "shield_cpu0.sh" on the camera baord and you have to install cpuset to run that shell file.
- Note: all the script files are not permanent and need to be executed after each restart to the device.

## Camera steps:

- Go to /src/ directory, create a build directory and run "cmake .." then "make".
- Go to /src/build/tests and run real_time_system_test with sudo.
- Usage: "sudo ./real_time_system_test  port_number  frequency tolerance_time_in_ms  single_message_size_in_bytes  [zerocopy|nozerocopy]_for_zerocopy_sender".
- Example: "sudo ./real_time_system_test 7575 30 33 300000 nozerocopy".
- If you used "shield_cpu0.sh" you should run it as: "sudo cset shield --exec ./real_time_system_test 7575 30 33 300000 nozerocopy".

## Receiver steps:

- In /others/ directory a receiver_example.c file is existed.
- Compile the file with "gcc receiver_example.c -o receiver_example".
- Run the executable with sudo after running the test program on the camera.
- Usage: "sudo ./receiver_example IP PORT"
- Example: "sudo ./receiver_example 192.168.1.245 7575"

After that the client will successfully connect to the camera and the camera will start to stream useless data. You can monitor the stream rate on the client side. and the dropped frames or the tolerance time used on the camera side.