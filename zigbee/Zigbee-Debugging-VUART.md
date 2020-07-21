<details>
<summary><font size=5>Table of Contents</font> </summary>
&nbsp;  

- [1. Introduction](#1-introduction)
- [2. Project Settings](#2-project-settings)
  - [2.1. Hardware Configuration](#21-hardware-configuration)
  - [2.2. Make sure the Segger RTT Control Block is 1KB Aligned](#22-make-sure-the-segger-rtt-control-block-is-1kb-aligned)
- [3. Test](#3-test)

</details>
&nbsp; 

# 1. Introduction
When developing Zigbee applications based on EmberZnet, it's recommended to keep the CLI interface as there are many debug commands. The debug commands are very helpful to manipulate the device and also helpful when trouble-shooting. In normal case, the CLI command is input and ouput through UART. In some cases, the UART might be used to communicate with other peripherals. In that case, you can still run CLI commands through VUART. If you are not familiar with VUAR, it's recommended to read [the page about using the VUART](Debugging-With-VUART) first.

# 2. Project Settings
## 2.1. Hardware Configuration
To enable the debug channel through VUART, you need to make some hardware settings.

1. Open the .isc file of your project, turn to "HAL" tab, then hit the button "Open Hardware Configurator", enable "GPIO" and config the corresponding pin. This is the pin for SWO.

<div align="center">
  <img src="files/ZB-Zigbee-Debugging-VUART/hw-config-swo.png">  
</div>  
</br>

2. Enable "Virtual UART". In its properties, select "VUART via SWO" for "VUART Type".

<div align="center">
  <img src="files/ZB-Zigbee-Debugging-VUART/hw-config-vuart.png">  
</div>  
</br>

3. Enable "Serial". In its properties, select "VUART" for "Port for application serial communication".

<div align="center">
  <img src="files/ZB-Zigbee-Debugging-VUART/hw-config-serial.png">  
</div>  
</br>

## 2.2. Make sure the Segger RTT Control Block is 1KB Aligned
When the WSTK tries to connect the device through Segger RTT, it will search the Segger RTT Control Block (The global variable **_SEGGER_RTT**) from the start of the RAM. The step is 1KB. It's mandatory to set it 1KB aligned. It's quite easy to achieve that. You just need to redefine the macro **SEGGER_RTT_ALIGNMENT** to **1024**.

<div align="center">
  <img src="files/ZB-Zigbee-Debugging-VUART/segger-rtt-align.png">  
</div>  
</br>

# 3. Test
1. Save and generate the project, then build it and flash it to the board.
2. Setup the VUART connection through SWD following the steps [here](Debugging-With-VUART).
3. When the device starts up, run commands to test the input and output.
```
ZigbeeMinimalSoc>
ZigbeeMinimalSoc>
ZigbeeMinimalSoc>
ZigbeeMinimalSoc>
ZigbeeMinimalSoc>info
MFG String: 
AppBuilder MFG Code: 0x1002
node [(>)000B57FFFEA8EF42] chan [0] pwr [0]
panID [0x0000] nodeID [0xFFFE] xpan [0x(>)0000000000000000]
parentID [0xFFFF] parentRssi [0]
stack ver. [6.7.6 GA build 327]
nodeType [unknown]
Security level [05]
network state [00] Buffs: 75 / 75
Ep cnt: 1
ep 1 [endpoint enabled, device enabled] nwk [0] profile [0xFFFF] devId [0xFFFF] ver [0x01]
Nwk cnt: 1
nwk 0 [Primary (pro)]
  nodeType [0x02]
  securityProfile [0x05]
ZigbeeMinimalSoc>
```