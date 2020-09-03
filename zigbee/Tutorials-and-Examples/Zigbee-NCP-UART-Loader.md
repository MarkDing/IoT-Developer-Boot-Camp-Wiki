<details>
<summary><font size=5>Table of Contents</font> </summary>  

- [NCP UART Loader](#ncp-uart-loader)
	- [1. Summary](#1-summary)
	- [2. Gecko SDK version](#2-gecko-sdk-version)
	- [3. Hardware Required](#3-hardware-required)
	- [4. Connections Required](#4-connections-required)
	- [5. Setup](#5-setup)
	- [6. How It Works](#6-how-it-works)
	- [7. .sls Projects Used](#7-sls-projects-used)
	- [8. How to Port to Another Part](#8-how-to-port-to-another-part)
	- [9. Special Notes](#9-special-notes)
</details>

********

# NCP UART Loader
## 1. Summary ##
When we upgrade Host + NCP platform, we have to make a choice about which one should be upgraded first. In this case, we would suggest you upgrade the NCP first. The reason is that the EZSP version may be upgraded in the newer version. If you upgrade the host first, then you won't be able to connect to the NCP.

But in fact, what customer needs is a more easy-to-use upgrader. It can upgrade NCP firmware regardless of the current version. This example introduces a standalone NCP UART Loader, so that users can upgrade NCP of any version.

## 2. Gecko SDK version ##
Gecko SDK Suite 2.7.

## 3. Hardware Required ##
- BRD4167A

## 4. Connections Required ##
Connect the WSTK to PC through USB cable

## 5. Setup ##
1. Create the Z3GatewayHost sample project, name it with `ncp-uart-loader`.
2. In **"ZCL Clusters"** tab, select endpoint 1, then enable the client side of cluster `Over The Air Bootloading`;
3. Select the following plugins:
	- [x] OTA Bootload Cluster Client
	- [x] OTA Bootload Cluster Client Policy
	- [x] OTA Cluster Platform Bootloader
4. Save and generate the project.
5. Uncompress the [ncp_uart_loader](files/ZB-Zigbee-NCP-UART-Loader/ncp_uart_loader.rar) example in this project, replace the makefile with the one in this example. 
6. Extract the source file `ncp-uart-loader.c`, `ncp-uart-loader.h` and `stub.c` to your project folder.
7. Make and test.


## 6. How It Works ##
1. Use a WSTK as the NCP. Flash a bootloader `bootloader-xmodem-uart` and the NCP-UART firmware;
2. Connect the WSTK to PC, start the `ncp-uart-loader` program on **Cygwin** to test.

	```
	$ ./ncp_uart_loader.exe -p COM28
	ezsp ver 0x8 stack type 0x2 stack ver. [6.7.6 0x41fa49 build 327]
	Found OTA file 'ncp-uart-hw.ota'
	Manufacturer ID: 0x1002
	Image Type ID:   0x0000
	Version:         0x00000014
	Header String:   EBL ncp-uart-hw
	Found 1 files

	Launching standalone bootloader...
	Starting bootloader communications.
	Delaying 4 seconds
	Setting up serial port
	Transferring EBL file to NCP...
	EBL data start: 0x  3E, end: 0x2C72E, size: 182000 bytes
	0x41e266: 0% complete
	0x41e266: 5% complete
	0x41e266: 10% complete
	0x41e266: 15% complete
	0x41e266: 20% complete
	0x41e266: 25% complete
	0x41e266: 30% complete
	0x41e266: 35% complete
	0x41e266: 40% complete
	0x41e266: 45% complete
	0x41e266: 50% complete
	0x41e266: 55% complete
	0x41e266: 60% complete
	0x41e266: 65% complete
	0x41e266: 70% complete
	0x41e266: 75% complete
	0x41e266: 80% complete
	0x41e266: 85% complete
	0x41e266: 90% complete
	0x41e266: 95% complete
	0x41e266: 100% complete
	Transfer completed successfully.
	Delaying 4 seconds
	Rebooting NCP
	Delaying 4 seconds

	Reboot not supported.  Exiting instead.
	```

## 7. .sls Projects Used ##
- [ncp_uart_loader.rar](files/ZB-Zigbee-NCP-UART-Loader/ncp_uart_loader.rar)

## 8. How to Port to Another Part ##
The project is a host program. It can be easily ported to Unix-like systems with the same approaches of porting the Z3GatewayHost sample.

## 9. Special Notes ##
NA
