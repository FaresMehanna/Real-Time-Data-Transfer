# Real-Time Data Transfer In Linux
The general problem I try to solve is the following:
Some given data is available for Y time and should be sent within X time with sending time tolerance Z time. where X+Z <= Y.

This is mainly written for Axiom cameras and will be optimized for them.

## Solution
- System that runs in given frequency, the system will query the user provided function for new data - packets - with the exact frequency.
- High resolution timers that will take care of sleeping the process until the next tick.
- Optimized Senders that will take care of sending the data within the time slot.
- Tolerance time is used to compensate - if any - sending delays in the current packets. The data must be available for the whole duration.
- Techniques such as CPU isolation, CPU shielding, changing process priority to RT and changing process scheduler policy to FIFO is recommended to be used for better timers accuracy and data throughput.
- Techniques such as increasing buffers, kernel TCP tuning and Ethernet interrupter handler affinity to shielded CPU are recommended for better sustained Ethernet throughput.
- Techniques such as zero-copy and DMA usage are recommended for efficient CPU usage.

## Architecture
The final application mainly consists of four components.
- Timers: there work is to provide accurate sleeping for the calling process.
- Senders: there work is to provide efficient data transmission by using worker thread.
- User Provided Function: which will be called periodically to get a list of packets to be sent.
- System: that have certain characteristic to combine the sender and the timer and the user provided function to run them.

The current implementation provide three timers and TCP sender.
You can test the current implementation by running CMakeLists.txt and execute the tests in the "src/tests" directory.
You will need to run the TCP tuning scripts on both the sender and receiver devices and it's recommended to shield a CPU on the sender device. Also make sure the receiver increase the receiver buffer for this socket, example of the receiver in "tests/receiver_example.c".
