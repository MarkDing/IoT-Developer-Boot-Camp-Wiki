<details>
<summary><font size=5>Table of Contents</font> </summary>
&nbsp;  

- [1. Introduction](#1-introduction)
- [2. Implement](#2-implement)
  - [2.1. Hardware Configuration](#21-hardware-configuration)
  - [2.2. Debug Print in NCP](#22-debug-print-in-ncp)
- [3. Test](#3-test)


</details>
&nbsp; 

# 1. Introduction
When developing NCP firmware, normally we just need to configure the hardware settings and then generate the project and build it. It's quite simple as there aren't much source codec. Then we just need to connect the NCP to the host, and then start the host program to check if the host program can communicate with the NCP successfully. If the host program failed to start due to the communication failed, it's not easy to find the problems. The first thing we need to do is to make sure the NCP is running normally. Therefore, we need to know how to debug the NCP.

Another issue is that sometimes the NCP may crash due to software problems. On the SoC platform, if the device crashes, there will be a crash info print when the device starts up, like:
```
[ASSERT:packet-buffer.c:483]
Reset info: 0x07 (CRS)
Extended Reset info: 0x0701 (AST)
Thread mode using main stack (20004000 to 20004960), SP = 200047E8
1736 bytes used (72%) in main stack (out of 2400 bytes total)
No interrupts active
Reset cause: Assert packet-buffer.c:483
R0 = 0000E3B0, R1 = 000001E3, R2 = 00000000, R3 = 00000002
R4 = 00000083, R5 = 20007460, R6 = 07FFF800, R7 = 20004FE8
R8 = 2000721C, R9 = 00000008, R10 = 20004FB8, R11 = 00000000
R12 = 00000006, R13(LR) = FFFFFFF9, MSP = 200047E8, PSP = 00000000
PC = 00007B40, xPSR = 41000000, MSP used = 000006C8, PSP used = 00000000
CSTACK bottom = 20004000, ICSR = 00000806, SHCSR = 00070008, INT_ACTIVE0 = 00000000
INT_ACTIVE1 = 00000000, CFSR = 00010000, HFSR = 00000000, DFSR = 00000000
MMAR/BFAR = E000ED34, AFSR = 00000000, Ret0 = 000062D1, Ret1 = 00007B6D
Ret2 = 0000E0B7, Ret3 = 0000E0F3, Ret4 = 0001705F, Ret5 = 00016F89
Dat0 = 0000E3B0, Dat1 = 000001E3
Key Est. Init Success 0x01
```

This is helpful for us to address the issue. We can decode the crash info and then find the last call stack. We have a [KBA](https://www.silabs.com/community/wireless/zigbee-and-thread/knowledge-base.entry.html/2012/06/28/how_do_i_get_detaile-W9KJ) about how to decode this crash info. But for NCP, when the crash or assert happen, there is no such kind of debug info. 

In this page, we will introduce the approach to add debug print in NCP.

# 2. Implement
In this page, we will print the debug info out through SWO pin. Please make sure there is a GPIO reserved for SWO. Actually, we always recommend customers to design the 10-pin-Simplicity-Connector per [AN958](https://www.silabs.com/documents/public/application-notes/an958-mcu-stk-wstk-guide.pdf). the SWO pin is already in this connector.

## 2.1. Hardware Configuration
1. Open the .isc file of your NCP project, turn to "HAL" tab, then hit the button "Open Hardware Configurator", enable "GPIO" and config the corresponding pin. This is the pin for SWO.

<div align="center">
  <img src="files/ZB-Zigbee-Debugging-NCP/hw-config-swo.png">  
</div>  
</br>

2. Select "Virtual UART". In its properties, select "VUART via SWO" for "VUART Type".

<div align="center">
  <img src="files/ZB-Zigbee-Debugging-NCP/hw-config-vuart.png">  
</div>  
</br>

3. Select "Serial", in properties, select "VUART" for "Port for application serial communication".

<div align="center">
  <img src="files/ZB-Zigbee-Debugging-NCP/hw-config-serial.png">  
</div>  
</br>

## 2.2. Debug Print in NCP
1. Here is an off-the-shelf [plugin](files/ZB-Zigbee-Debugging-NCP/ncp-debug-print.rar) to provide debug print info in NCP project. Uncompress the file, copy the folder "ncp-debug-print" to protocol\zigbee\app\ncp\plugin of your SDK.  
e.g.
```
C:\SiliconLabs\SimplicityStudio\v4\developer\sdks\gecko_sdk_suite\v2.7\protocol\zigbee\app\ncp\plugin\ncp-debug-print
```
**Note**: You need to restart Simplicity Studio after you copied the plugin. When it's done, you can see this plugin in AppBuilder.

<div align="center">
  <img src="files/ZB-Zigbee-Debugging-NCP/plugin-ncp-debug-print.png">  
</div>  
</br>

2. Enable this plugin, save and generate your project. Please note that you need to enable the following plugins to satisfy the dependency requirement.
- Ember Minimal Printf
- Serial

# 3. Test
1. Save and generate the NCP project, then build it and flash it to the board.
2. Setup the VUART connection through SWD following the steps [here](Debugging-With-VUART).
3. When the NCP starts up, you will see the reboot cause info printed out.
```
Reset info: 0x06 ( SW)
Extended Reset info: 0x0601 (RBT)
```
4. If there is a crash, the crash info will also be printed out.
```
Reset info: 0x07 (CRS)
Extended Reset info: 0x0701 (AST)
Thread mode using main stack (20001320 to 20001C80), SP = 20001A70
724 bytes used (30%) in main stack (out of 2400 bytes total)
No interrupts active
Reset cause: Assert ../ncp-callbacks.c:29
R0 = 00030A74, R1 = 0000001D, R2 = 00000000, R3 = 000000AA
R4 = 00000001, R5 = 20003A32, R6 = 200038D8, R7 = 00000000
R8 = 00000007, R9 = 20002F7A, R10 = 20003A4C, R11 = 00000000
R12 = 000308D0, R13(LR) = FFFFFFF9, MSP = 20001A70, PSP = 20001C80
PC = 00007608, xPSR = 61000000, MSP used = 000002D4, PSP used = 00000000
CSTACK bottom = 20001320, ICSR = 00000806, SHCSR = 00070008, INT_ACTIVE0 = 00000000
INT_ACTIVE1 = 00000000, CFSR = 00010000, HFSR = 00000000, DFSR = 00000000
MMAR/BFAR = E000ED34, AFSR = 00000000, Ret0 = 00007601, Ret1 = 000153ED
Ret2 = 0000149D, Ret3 = 00002189, Ret4 = 0001A0C3, Ret5 = 000150B1
Dat0 = 00030A74, Dat1 = 0000001D
Reset info: 0x06 ( SW)
Extended Reset info: 0x0601 (RBT)
Reset info: 0x06 ( SW)
Extended Reset info: 0x0601 (RBT)
```
