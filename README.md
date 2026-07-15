
# Smart Home Network Automation System
> **Team Project (3 members)**

My primary responsibility was designing the network architecture, including VLSM subnetting, IP addressing, router interface planning, and network documentation. The TCP client-server implementation and device simulation were developed collaboratively by the team.


## Overview
- Team project completed in a Data Communications and Networking course (3 members).
- Developed a smart home automation system in C++ using TCP socket programming.
- Implemented a multithreaded client-server architecture to support multiple simultaneous client connections.
- Simulated smart home devices, including lighting, thermostat, and security systems.
- Designed a segmented network using VLSM subnetting and static routing to improve network organization and scalability.

## Technologies

- C++
- TCP/IP
- Socket Programming
- Multithreading
- Subnetting
- VLSM
- Static Routing
- Git


## My Contributions

- Designed the network topology
- Planned VLSM subnetting
- Created the IP addressing scheme
- Assigned router interfaces and gateway addresses
- Documented the network architecture


## Final Network Architecture
I designed the network architecture shown below.

<img width="942" height="442" alt="Network Architecture" src="https://github.com/user-attachments/assets/ca3cd7ad-0ef3-4a92-a1e6-d5540f6ec194" />



## Network Design

This project uses **Variable Length Subnet Masking (VLSM)** to efficiently allocate IP address ranges based on the expected number of devices in each subnet.


### Subnet Allocation

| Subnet | CIDR | Subnet Mask | Network Address | Usable Hosts | Usable IP Range |
|--------|------|-------------|-----------------|-------------:|-----------------|
| Lighting | /26 | 255.255.255.192 | 192.168.1.0 | 62 | 192.168.1.1 – 192.168.1.62 |
| Thermostat | /27 | 255.255.255.224 | 192.168.1.64 | 30 | 192.168.1.65 – 192.168.1.94 |
| Security | /27 | 255.255.255.224 | 192.168.1.96 | 30 | 192.168.1.97 – 192.168.1.126 |
| Management | /28 | 255.255.255.240 | 192.168.1.128 | 14 | 192.168.1.129 – 192.168.1.142 |


### Router Interface Configuration

| Router Interface | Gateway IP | Connected Subnet |
|-----------------|------------|------------------|
| eth0 | 192.168.1.1 | Lighting (192.168.1.0/26) |
| eth1 | 192.168.1.65 | Thermostat (192.168.1.64/27) |
| eth2 | 192.168.1.97 | Security (192.168.1.96/27) |
| eth3 | 192.168.1.129 | Management (192.168.1.128/28) |


### Network Design Decisions

| Design Choice | Reason |
|--------------|--------|
| Base Network | Started with **192.168.1.0/24**, a standard private IPv4 network for LAN environments. |
| Host Planning | Estimated the required number of devices before subnetting. |
| VLSM | Used Variable Length Subnet Masking to allocate IP addresses efficiently. |
| Network Segmentation | Separated Lighting, Thermostat, Security, and Management into different subnets for better organization and reduced broadcast traffic. |
| CIDR Allocation | Used **/26**, **/27**, and **/28** according to the number of required hosts. |
| Management Subnet | Placed the server and client in a dedicated management subnet to isolate management traffic from smart devices. |



## Design Process
### How to design networking
1. Started with the base network 192.168.1.0/24.
2. Estimated the required number of hosts for each subnet.
3. Applied VLSM (Variable Length Subnet Masking) to calculate subnet sizes.
4. Assigned IP address ranges and gateway addresses to each subnet.
5. Created the final network topology in Figma.


**Step 1 - Planning the Network**

<img width="600" alt="1" src="https://github.com/user-attachments/assets/0041e5af-bb87-4dc1-8eae-378d98e4c853" />

I first planned the overall network structure by separating the smart home system into multiple subnets. The management subnet contains the server and client, while dedicated subnets were planned for the lighting, thermostat, and security devices. This layout provides better organization and allows each device group to be managed independently.


**Step 2 - Designing the Subnets**

<img width="600" alt="2" src="https://github.com/user-attachments/assets/37c664c5-38a7-4e37-b6a3-c908e21c3754" />

After planning the network structure, I calculated the subnet sizes using VLSM. I estimated the required number of hosts for each device group and selected the appropriate CIDR prefix for every subnet to efficiently utilize the available IP address space.


**Step 3 - VLSM Calculation**

<img width="600" alt="3" src="https://github.com/user-attachments/assets/ee5bf21d-a1b0-4099-8994-947d4c5a851d" />

After calculating the subnet sizes, I assigned IP address ranges to each subnet using VLSM. I verified the network address, usable host range, and broadcast address for each subnet to ensure efficient IP address allocation and proper network segmentation.


## Repository Structure

| Folder | Description |
|---------|-------------|
| include | Header files |
| src/client | Client implementation |
| src/server | Multithreaded TCP server |
| src/network | Network configuration and subnet management |
| src/devices | Smart device simulation |
| src/protocol | HTTP-like communication protocol |
| tests | Unit tests |


## Reflection
This was my first time designing a network architecture for a practical project. At first, I found it difficult to divide the original IP address range into multiple subnets while making sure that each device group had enough usable host addresses.

However, as I worked through the calculations step by step, I began to understand how the subnet sizes, network addresses, usable host ranges, and broadcast addresses were connected. When I finally completed the IP addressing plan and saw the full network structure come together, I felt a strong sense of clarity and achievement.

This experience showed me that network design can feel complicated at first, but breaking the problem into smaller steps makes it much easier to understand.


## What I Learned
Through this project, I learned how to plan and allocate IP addresses based on the number of devices required in each network segment. I also developed a clearer understanding of VLSM, including what it is, why it is used, and how it improves the efficiency of IP address allocation.

Instead of assigning the same subnet size to every device group, I used different CIDR prefixes based on the expected number of hosts. This allowed the lighting network to receive a larger subnet, while the thermostat, security, and management networks received smaller subnets that matched their needs.

I also learned that drawing the network architecture helps make the design easier to understand. Creating the topology by hand first allowed me to organize my ideas, check the relationships between the router, server, client, and device subnets, and identify mistakes before creating the final diagram in Figma.

This project made me more interested in network architecture because I enjoyed turning calculations into a clear and organized network design.


## Skills Demonstrated

- Network Architecture Design
- IPv4 Addressing
- VLSM (Variable Length Subnet Masking)
- Subnetting
- CIDR Notation
- IP Address Planning
- TCP/IP Networking
- Static Routing
- Network Topology Design
- Figma (Network Diagram)
- C++
- TCP Socket Programming
- Multithreaded Client-Server Architecture
- Git & GitHub


## Future Improvements

In future projects, I would like to design a larger network that integrates cloud services such as AWS. I also want to gain more experience with routing, VLANs, firewalls, and hybrid cloud networking to build more realistic and scalable network architectures.
