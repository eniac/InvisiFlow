# InvisiFlow

## Introduction

**InvisiFlow** is a communication substrate designed to silently collect network telemetry data. Unlike traditional systems that push telemetry packets to collectors along the shortest path, InvisiFlow dynamically identifies and utilizes spare network capacity. By leveraging opportunistic sending and congestion gradients, it effectively reduces both the telemetry data loss rate and the overhead on user traffic.

[*Our paper*](https://yindazhang.github.io/files/InvisiFlow.pdf) will appear in NSDI 2025.

## About this repo

- `similation/`: Contains code for the ns-3 simulations of InvisiFlow.
- `testbed/`: Contains code for the implementation of InvisiFlow.
- More details can be found within the respective folders.
