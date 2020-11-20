<details>
<summary><font size=5>Table of Contents</font> </summary>

- [1. Introduction](#1-introduction)
- [2. Prerequiesite](#2-Prerequisite)
- [3. PHY Confirm](#3-PHY-Confirm)
- [4. EFR32xG22 project](#4-EFR32xG22-project)
- [5. Conclusion](#5-Conclusion)

</details>

# 1. Introduction
Sometime we got request on configure compatiable PHY for communicate with device from other vendor. Here share the experiece on configure an EFR32xG22 PHY communicate with TI CC2541 [proprietary 2.4G PHY](files/CM-Configure-TI-CC2541-compatible-proprietary-PHY/2M_BS-TX_2.xml). 
To configure the PHY, we need to know the detail parameter on radio configurator.

# 2. Prerequisite 

## 2.1. Hardware Requirement
* 1 [WSTK with EFR32MG22 radio boards(BRD4182A)](https://www.silabs.com/development-tools/wireless/efr32xg22-wireless-starter-kit)
* 1 SmartRF05 EB plus [CC2541EMK](https://www.ti.com/tool/CC2541EMK)
* 1 [Anritsu MS2692A](https://www.anritsu.com/en-US/test-measurement/products/ms2692a)

The BRD4182A radio board supports three wireless protocols. Bluetooth LE/Mesh, Zigbee and Proprietary.
<div align="left">
  <img src="files/CM-Configure-TI-CC2541-compatible-proprietary-PHY/wstk.png">  
</div> 
CC2541EMK is the one to communicate with, we need it for confirm the communication. And confrim some unknown feature from its air data.
<div align="left">
  <img src="files/CM-Configure-TI-CC2541-compatible-proprietary-PHY/SmartRF05EB.png">  
</div> 
Anritsu MS2692A, use for checking the air data on both EFR32xG22 and CC2541.
<div align="left">
  <img src="files/CM-Configure-TI-CC2541-compatible-proprietary-PHY/ms2692a-signalanalyzers.png">  
</div> 


## 2.2. Software Requirement
**Simplicity Studio** is a free Eclipse-based Integrated Development Environment (IDE) and a collection of value-add tools provided by Silicon Labs. Developers can use Simplicity Studio to develop, debug and analyze their applications. Silicon Labs recently released the [Simplicity Studio 5](https://www.silabs.com/products/development-tools/software/simplicity-studio/simplicity-studio-5) which is based on the latest versions of Eclipse and the C/C++ Development Tooling. Builds off the features of the previous version and includes advanced SecureVault technology, a new web-like interface, and solid performance improvements.  

**SmartRF™ Studio** SmartRF™ is a Windows application that helps designers of RF systems to easily evaluate the radio at an early stage in the design process for CC2xxx low-power RF devices. It simplifies generation of the configuration register values and commands, as well as practical testing and debugging of the RF system. We use [SmartRF Studio 7](https://www.ti.com/tool/SMARTRFTM-STUDIO).

## 2.3. Given parameter
Some are provided and some are get from the [xml file](files/CM-Configure-TI-CC2541-compatible-proprietary-PHY/2M_BS-TX_2.xml), finally got below known parameter.
* Modulation: 2GFSK
* Datarate: 2Mbps
* Deviation 500KHz
* Preamble: 10101010 (13 bytes)
* Syncword: 0x29417671
* Payload: 64 bytes variable length
* Endian: MSB_FIRST
* CRC polynomial: CCITT_16(0x1021)
* CRC seed: 0xFFFF
* Whitening polynomial: PN9_BYTE
* Whitening seed: 0xFF

# 3. PHY Confirm
Try to configure a PHY according to above known parameter. The PHY don't work, don't even detect the preamble, so it is worth to annalyze data transmit from CC2541.

## 3.1. Transmit data by SmartRF Studio 7
<div align="left">
  <img src="files/CM-Configure-TI-CC2541-compatible-proprietary-PHY/TICC2541-TX.gif">  
</div> 
For easier figure out the frame, it is worth to disable the whitening and set some special data bit on the frame, like add 0x00/0xFF.
<div align="left">
  <img src="files/CM-Configure-TI-CC2541-compatible-proprietary-PHY/frame.png">  
</div> 

## 3.2. Capture data on MS2692A
To get a readable frame data on MS2692A, we need to configure the frequency, reference power level etc. accordingly.
### 3.2.1 Use "Power vs Time" trace mode to get the TX pluse.
<div align="left">
  <img src="files/CM-Configure-TI-CC2541-compatible-proprietary-PHY/SA-01-pluse.png">  
</div> 

### 3.2.2 Change the "Start Time" to locate one pluse.
<div align="left">
  <img src="files/CM-Configure-TI-CC2541-compatible-proprietary-PHY/SA-02-location.png">  
</div> 

### 3.2.3 Zoom in.
<div align="left">
  <img src="files/CM-Configure-TI-CC2541-compatible-proprietary-PHY/SA-03-zoom-in.png">  
</div> 

### 3.2.4 Use "Frequency vs Time" trace mode, the whole frame will be show up.
Time between Marker1 and Marker 2 is 52us. This should be the preamble time. 13bytes preamble, 13x8x0.5us=52us. As we can see MS2629A only detect 1 byte preamble and got 48us CW signal. Still not know the reason, but checking only TI BLE PHY, it got the same issue.
<div align="left">
  <img src="files/CM-Configure-TI-CC2541-compatible-proprietary-PHY/SA-04-frame.png">  
</div> 

### 3.2.5 Read the frame data.
Once the Preamble can be located, the frame data should be readable, we just try to sample the data bit for each 0.5us interval.
<div align="left">
  <img src="files/CM-Configure-TI-CC2541-compatible-proprietary-PHY/SA-05-syncwords.png">  
</div> 

# 4. EFR32xG22 Project
After checking the frame data, now we can create and configure on our EFR32xG22 project, here we create "Flex (RAIL) - RAILtest" example on SSv5.
<div align="left">
  <img src="files/CM-Configure-TI-CC2541-compatible-proprietary-PHY/railtest.png">  
</div> 

## 4.1. Customize the PHY
Double click "radio_settings.radioconf", select one PHY, then check on "Customized", change frequency to 2466MHz.
<div align="left">
  <img src="files/CM-Configure-TI-CC2541-compatible-proprietary-PHY/customized.png">  
</div> 
Configure according to arget PHY. 2Mbps data rate, 500KHz deviation. Configure flexible payload length, use CCITT_16(0x1021) polynomial and 0xFFFF CRC seed. Enable whitening. The Preamble set 8 bit here.
<div align="left">
  <img src="files/CM-Configure-TI-CC2541-compatible-proprietary-PHY/packet.png">  
</div> 
Uncheck "Number of Symbols in Timing Window" for disable preamble.
<div align="left">
  <img src="files/CM-Configure-TI-CC2541-compatible-proprietary-PHY/timing.png">  
</div> 


Left the other parameter as default, the PHY is done. Save and build, now then software is ready.  
For more infromation on radio configuratore, refer to [AN1253](https://www.silabs.com/documents/public/application-notes/an1253-efr32-radio-configurator-guide-for-ssv5.pdf). 

## 4.2. Confirm the PHY
TI CC2541 RX, EFR32 TX, 20 packets.
<div align="left">
  <img src="files/CM-Configure-TI-CC2541-compatible-proprietary-PHY/EFR32-TX.gif">  
</div> 
TI CC2541 TX, EFR32 RX, 20 packets.
<div align="left">
  <img src="files/CM-Configure-TI-CC2541-compatible-proprietary-PHY/EFR32-RX.gif">  
</div> 

For more information on CLI command, refer to [UG409](https://www.silabs.com/documents/public/user-guides/ug409-railtest-users-guide.pdf).

# 5. Conclusion
By confirm the PHY on Signal Annlyzer and serveral try on CRC and Whitening config, we can get a PHY that both side TX/RX compatible.
