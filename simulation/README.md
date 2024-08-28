# NS-3 Simulation of InvisiFlow

## Traffic generator
The source code for the traffic generator is located in the `scratch/` directory. This code is adapted from [*HPCC-PINT*](https://github.com/ProbabilisticINT/HPCC-PINT). 

## Simulation
The simulator is based on ns-3.37. Most modifications have been made in the `src/point-to-point/` directory. For instance, we added `collector-node.cc/h` for InvisiFlow collectors and `switch-node.cc/h` for InvisiFlow switches.