
# Smart Home Network Automation System
> **Team Project (3 members)**

My primary responsibility was designing the network architecture, including VLSM subnetting, IP addressing, router interface planning, and network documentation. The TCP client-server implementation and device simulation were developed collaboratively by the team.


## Overview
- Team project completed in a Data Communications and Networking course (3 members).
- Developed a smart home automation system in C++ using TCP socket programming.
- Implemented a multithreaded client-server architecture to support multiple simultaneous client connections.
- Simulated smart home devices, including lighting, thermostat, and security systems.
- Designed a segmented network using VLSM subnetting and static routing to improve network organization and scalability.

## Final Network Architecture
I designed the network architecture shown below.

<img width="942" height="442" alt="Network Architecture" src="https://github.com/user-attachments/assets/ca3cd7ad-0ef3-4a92-a1e6-d5540f6ec194" />


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
<img width="1500" height="2000" alt="1" src="https://github.com/user-attachments/assets/0041e5af-bb87-4dc1-8eae-378d98e4c853" />

I started with a private IPv4 network (192.168.1.0/24) because it is commonly used for local area networks. I estimated the number of devices required for each category (lighting, thermostat, and security) before designing the subnet layout. This planning helped determine the appropriate subnet sizes while leaving room for future expansion.

**Step 2 - Designing the Subnets**
<img width="1500" height="2000" alt="2" src="https://github.com/user-attachments/assets/37c664c5-38a7-4e37-b6a3-c908e21c3754" />

I separated the network into multiple subnets based on device functions instead of placing every device in the same network. This approach improves network organization, reduces unnecessary broadcast traffic, and makes it easier to manage each device group independently.


**Step 3 - VLSM Calculation**
<img width="1500" height="2000" alt="3" src="https://github.com/user-attachments/assets/ee5bf21d-a1b0-4099-8994-947d4c5a851d" />

After estimating the required number of hosts, I used Variable Length Subnet Masking (VLSM) to allocate different subnet sizes. The lighting network received a /26 subnet because it required the largest number of hosts, while the thermostat and security networks used /27 subnets. This allowed efficient use of the available IP address space.


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
