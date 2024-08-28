# Implementation of InvisiFlow

## Server Implementation
The source code for the InvisiFlow servers (e.g., collectors) can be found in the `server/` directory. These servers have been tested with DPDK 21.11. They are responsible for continuously sending pull requests and processing telemetry packets from the network.

## Switch Implementation
The source code for the InvisiFlow switches is located in the `switch/` directory. This code is based on the P4-16 version of [*OrbWeaver*](https://github.com/eniac/OrbWeaver/tree/main/p4v16). The setup and execution process follows the same steps as outlined for [*OrbWeaver*](https://github.com/eniac/OrbWeaver/tree/main).