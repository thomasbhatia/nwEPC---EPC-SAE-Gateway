
:: Introduction 
==================================================================================

This note describes a brief tutorial on nwEPC software usage.


:: Hardware Requirements
==================================================================================

1. Any x86 based RHEL 5.x or Fedora Core 14 machine or server with three NICs.
2. One Layer 3 Switch.


:: Software Requirements 
==================================================================================

1. RHEL 5.x or Fedora Core 14 and above.
2. libevent-1.4 package.
3. Wireshark for network captures.
4. iperf for throughput testing.
5. root privileges.


:: Installation
==================================================================================

1. Untar the package and enter the directory.

 $ tar -xvzf nwepc-0.x.tar.gz
 $ cd tar -xvzf nwepc-0.x

2. Execute following commands:

 $ ./configure
 $ make
 $ make install

If all goes well, this shall install nwLteSaeGw and nwLteMmeEmu on the system.


:: IP Addressing 
==================================================================================

IPv6 is NOT supported as yet.
One IPv4 address is required for each of the following interfaces :

1. SGW-S11  - Control Plane
2. SGW-S5   - Control Plane
3. PGW-S5   - Control Plane
4. S1/S5/S8 - User Plane

Same IPv4 address interface is used for S1, S5 and S8 user planes. One of the 
control plane IPv4 address MAY be reused for S1/S5/S8 user plane as the GTP-U udp 
port(2152) is different than GTP-C udp port(2123).

Other than these, an IPv4 address range for UEs will also be required.


:: Running the Software
==================================================================================

1. To run SAE-GW software select a RHEL5.x/Fedora Core 14 based x86 target machine 
   with preferably with three NICs(Network Interface Cards) one for each 
   S1-U/S5-U/S8-U, S11-C/S5-C/S8-C,and SGi interfaces. 

  Sample illustration:

  +------------------------------------------------+
  |                                                | 
  |     Combined SGW/PGW Application Framework     |
  |            Control Plane/Data Plane            |
  |                                                | 
  +------------------------------------------------+
  |                                                |
  |             FedoraCore14/RHEL5.x               |
  |                                                |
  +------------+----+------------+----+------------+
  | NIC1(eth0) |    | NIC2(eth1) |    | NIC3(eth2) |
  +------------+    +------------+    +------------+
         ^                 ^ ^               ^ 
         |                 | |               |                      __
         |                 | |               |                   __(  )__
         | S1-U      S11-C | | S5/S8-C       |       SGi        (        )  
         | S5-U            | |               +------------+--->( Internet )
         | S8-U            | |                            |     (__    __)
         |                 | +---------------+            |        (__)
         |                 |                 |            |
         |                 |                 |            | SGi
         v                 v                 v            |
  +------------+    +------------+    +------------+      |
  | eNodeB/PGW |    |    MME     |    | Other PGWs |------+
  +------------+    +------------+    +------------+


2. Create virtual IP address on NIC1 for data plane interface SGW/PGW for 
   S1-U/S5-U interface. Both SGW and PGW shall share the same data plane. 
   Lets assume the IP address is x.y.z.101. PLease note that one of the control 
   plane IP address may be reused for this purpose.

3. Create three (virtual) IP addresses on NIC2 for control plane IP addresses of 
   GTP-C entities for SGW-S11-C, SGW-S5-C and PGW-S5-C interfaces.
   Lets say the IP addresses are x.y.z.101, x.y.z.103 and x.y.z.104 respectively.

4. Connect the NIC3 card to internet router. This NIC may have internet
   routable address assigned to it. Lets assume this NIC is eth2.

5. Assuming that the UE IP address range starts with 10.10.0.0 and subnet mask 
   255.255.0.0, run the software application using following command:

   $ nwLteSaeGw --gtpu-ip x.y.z.101 --sgw-s11-ip x.y.z.102 --sgw-s5-ip x.y.z.103\
     --pgw-s5-ip x.y.z.104 --sgi-if eth2 --ippool-subnet 10.10.0.0              \
     --ippool-mask 255.255.0.0

6. You may have to configure static routes for the UE IP address range at the 
   internet edge router and vice versa.

7. Use help for more details.

  $ nwLteSaeGw -h

  Supported command line arguments are:

  +----------------------+-------------+-------------------------------------+
  | ARGUMENT             | PRESENCE    | DESCRIPTION                         |
  +----------------------+-------------+-------------------------------------+
  | --sgw-s11-ip | -ss11 | MANDATORY   | S11 control IP address of the SGW.  |
  | --sgw-s5-ip  | -ss5  | MANDATORY   | S5 control IP address of the SGW.   |
  | --pgw-s5-ip  | -ps5  | MANDATORY   | S5 control IP address of the PGW.   |
  | --gtpu-ip    | -gi   | MANDATORY   | IP address for the GTPU User Plane. |
  | --apn        | -ap   | MANDATORY   | Access Point Name to be served.     |
  | --ippool-subnet| -is | MANDATORY   | IPv4 address pool for UEs.          |
  | --ippool-mask | -im  | MANDATORY   | IPv4 address pool for UEs.          |
  | --sgi-if     | -si   | OPTIONAL    | Network interface name for the SGi. |
  | --max-ue     | -mu   | OPTIONAL    | Maximum number of UEs to support.   |
  | --combined-gw | -cgw | OPTIONAL    | Combine SGW and PGW funtions.       |
  +----------------------+-------------+-------------------------------------+


:: Workarounds
==================================================================================

1. If you have only two NIC cards, combine NIC1 and NIC2 shown in figure above 
   into one.
2. If you have only one NIC card you may still follow the same procedure. Software
   may face some performance degradation.
3. Increase/decrease the log level by exporting shell variable NW_LOG_LEVEL to 
   either DEBG, INFO, NOTI, WARN ERRO, CRIT, EMER or ALER. For example: 
   $ export NW_LOG_LEVEL=ERRO


:: A Simple Use Case - PING
==================================================================================

1. Objective

Ping an IP address in PDN from UE and vice-versa.

2. Contraints and Assumptions

The assumptions are non-availability of eNodeB and UE. The setup uses MME-emulator
and simple linux machine to emulate behaviour of UE sending ping data. 

For this, MME-emulator behaves as L3 switch which takes data from the LAN and sends
it over S1-U interface to PDN. When a call is established via MME-emulator, it 
emulates as L3 switch which has establish route for the UE IPv4 address.

3. Hardware requirements

FedoraCore14/RHEL5.x Server/Machines
with at least two NICs each             - 2
FedoraCore14/RHEL5.x Server/Machines
with one NICs each                      - 2
L3 Switch                               - 2

Note: If L3 switch is not available you may use another two linux machines.

4. Logical requirements

IPv4 address range for UEs. This range should be routable in PDN. Similary, a few 
PDN IP address are required.

5. Set-up                             
                                   __
                                __(  )__
  +=============+......        (        )        ......+=============+
  ||  MME-EMU  || NIC1 |<=====( IP Cloud )=====>| NIC1 ||  SAE-GW   ||
  +=============+``````        (__    __)        ``````+=============+
     | NIC 2 |                    (__)                    | NIC 2 |
      ```+```                                              ```+```
         |                                                    |    
     +---+---+                                            +---+---+
     | L3 SW |                                            | L3 SW |
     +---+---+                                            +---+---+
         |                                                    |
    +----+----+                                          +----+----+
    | E-UTRAN |                                          |   PDN   |
    +---------+                                          +---------+


Install nwEPC software on two of the servers/machines with two NICs each 
mentioned in 2 above. We will use one as SAE-GW and other as MME-emulater.

For SAE-GW one NIC shall interface with PDN while other will be used for
S11 and S5/S8 (both control and data). Similarly, for MME-emulator one NIC 
shall interface with E-UTRAN while other will be used for S11 and S5/S8 
(both control and data).

We will configure one of the remaning servers/machines to reside in PDN while 
other as UE in E-UTRAN.

On the machine which lies in PDN add a default route to SAE-GW SGi interface. 
This ensures all packet from PDN are routed to PGW(SAE-GW) data-plane.

On the machine in E-UTRAN add a default route to MME tun-if. This will make 
sure all packets from E-UTRAN are received at MME-emulater which is emulating 
S1-U interface.

Please note that MME-emulater will not install the ip-addresses for UEs. It 
is assumed that all UE IP address are already presnt in E-UTRAN. The MME-UE 
will sniff the packets and transport them over S1-U to PDN. 

On SAE-GW machine add default route to L3-Switch for all traffic. And similary 
on MME-EMU machine add default route to L3-Switch, it is connected to, for all 
traffic.

6. After the setup is ready ,start SAE-GW and establish sessions using 
MME-emulator. 

Supported command line arguments for MME-emulator are:

+---------------------------+---------------+---------------------------------------+
| ARGUMENT                  | PRESENCE      | DESCRIPTION                           |
+---------------------------+---------------+---------------------------------------+
| --mme-ip | -mi            | MANDATORY     | IP address for the MME control plane. |
| --sgw-ip | -si            | MANDATORY     | IP address of the target SGW.         |
| --pgw-ip | -pi            | MANDATORY     | IP address of the target PGW.         |
| --gtpu-ip | -gi           | MANDATORY     | IP address for the MME user plane.    |
| --num-of-ue | -nu         | OPTIONAL      | Number of UEs to simulate.            |
| --session-timeout | -st   | OPTIONAL      | UE session timeout in seconds.        |
| --reg-per-sec| -rps       | OPTIONAL      | Session registrations per second.     |
| --tun-if  | -si           | OPTIONAL      | Network interface name for tunnel-if. |
+---------------------------+---------------+---------------------------------------+

Example Usage: 
$ nwLteMmeEmu --mme-ip 10.0.0.1 --sgw-ip 10.0.0.2 --pgw-ip 10.0.0.3 --gtpu-ip 10.0.0.1 \
  --tun-if eth1 -nu 50000 -st 120 -rps 100

7. Create a virtual IP assigned to any of UE on E-UTRAN machine. 

8. Ping IP address on the machine in PDN from the machine EUTRAN using IP
   address installed in 7.

9. If all goes well, you will see ping response.

10. Check routes, ip addresses and connectivity for trouble shooting.

11. Run iperf to find throughput bandwidth provided by SAE-GW.


:: Licence Information
==================================================================================

Copyright (c) 2010-2011 Amit Chawre <http://www.amitchawre.net/contact.html>
All rights reserved.              

Redistribution and use in source and binary forms, with or without          
modification, are permitted provided that the following conditions          
are met:                                                                    

1. Redistributions of source code must retain the above copyright           
notice, this list of conditions and the following disclaimer.            
2. Redistributions in binary form must reproduce the above copyright        
notice, this list of conditions and the following disclaimer in the      
documentation and/or other materials provided with the distribution.     
3. The name of the author may not be used to endorse or promote products    
derived from this software without specific prior written permission.    

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR        
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES   
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.     
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,            
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT    
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,   
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY       
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT         
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF    
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.           


==================================================================================

DISCLAIMER : THIS INFORMATION IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS 
OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, 
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
INFORMATION, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 

==================================================================================

