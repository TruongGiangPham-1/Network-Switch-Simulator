# Simulation of a mini DataCenter

## Problem Context
a data Center usually have hundreds to tens of thousands of hosts. Each host have its own CPU, memory, cache, and disk.
Each hosts are often stacked on top of each other in a rack. Furthuremore, there is a packet switch on top of each rack called `Top of Rack (TOR) switch`.
Ultimately, communications between and within racks are managed by TOR switches, ensuring packets from one host are delivered to other hosts in the same rack, or forwarded to other hosts in different racks.  

This project considers a simplified version of a data center network where TOR switches are connected to a centralized controller called `master` switch. 
This architecure is called `software-defined networking`, meaning the control plane and forwarding plane are separated.


