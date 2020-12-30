<details>
<summary><font size=5>Table of Contents</font> </summary>

- [1. Introduction](#1-introduction)
- [2. Prerequiesite](#2-Prerequisite)
- [3. PHY Confirm](#3-PHY-Confirm)
- [4. EFR32xG22 project](#4-EFR32xG22-project)
- [5. Conclusion](#5-Conclusion)

</details>

# 1. Introduction
Sometime Bluetooth connection fail happen, to avoid or improve this, it needs to locate the the failaure cause by which side(GATT Client or Server) or stop at which step. The Simplicity Studio got the helpful tool Network Analyzer. Packets over the air can be captured by the WSTK and decoded detailly. The captured data is provided by the EFR32 device RF layer. In some case it is possible that the data don't even cupture by EFR32 device RF layer. So here introduce another powerful tool [Ellisys Bluetooth Tracker](https://www.ellisys.com/products/btr1), it designed to support concurrent capture and analysis of Bluetooth Low Energy and Wi-Fi communications, as well as a wide variety of wired interfaces.

# 2. Prerequisite 

## 2.1. Hardware Requirement
* 1 [WSTK with EFR32MG22 radio boards(BRD4182A)](https://www.silabs.com/development-tools/wireless/efr32xg22-wireless-starter-kit)
* 1 Smart phone, here use [HUAWEI BTV-W09](https://www.amazon.com/MediaPad-Android-Marshmallow-Moonlight-Warranty/dp/B01LB08BH6), Android 7.0
* 1 [Ellisys Bluetooth Tracker](https://www.ellisys.com/products/btr1)

The BRD4182A radio board supports three wireless protocols. Bluetooth LE/Mesh, Zigbee and Proprietary. Here use it on Bluetooth LE.
<div align="center">
  <img src="files/BL-Bluetooth-connection-issue-location/wstk.png">  
</div> 
HUAWEI BTV-W09, Run EFR connect App for connect with EFR32xG22.
<div align="center">
  <img src="files/BL-Bluetooth-connection-issue-location/phone.png">  
</div> 
Ellisys Bluetooth Tracker, capture BLE packet over the air, include traffic between EFR32xG22 and Smart phone.
<div align="center">
  <img src="files/BL-Bluetooth-connection-issue-location/ellisys.png">  
</div> 


## 2.2. Software Requirement
**Simplicity Studio** is a free Eclipse-based Integrated Development Environment (IDE) and a collection of value-add tools provided by Silicon Labs. Developers can use Simplicity Studio to develop, debug and analyze their applications. Use it Network Analyzer for packets capture.  

**Ellisys Bluetooth Analyzer** Support BLE air data concurrent capture, use for compare with datd captured by Network Analyzer.

## 2.3. Data Cupture
Download test software on EFR32xG22. Start both Network Analyzer and Ellisys Bluetooth Analyzer capture. Manually operate connect and disconnect on Smart phone EFR connect. Retry several times then stop the capture and save the data. How to cupture the air data, here will not going into the detail, refer these links for more information, [Network Analyzer](https://www.silabs.com/documents/login/presentations/tech-talk-using%20silabs-network-analyzer.pdf), [Ellisys Bluetooth Analyzer](https://www.ellisys.com/products/download/bta_manual.pdf).

# 3. Analyzer Cuptured Data
After Cuptured, got [.btt](files/BL-Bluetooth-connection-issue-location/src/connecton.btt) file on Ellisys Bluetooth Analyzer and [.isd](files/BL-Bluetooth-connection-issue-location/src/connecton.isd) file on Network Analyzer.

## 3.1. Data on Ellisys Bluetooth Analyzer
Below GIF file show how to open a config file and transmit RF data on SmartRF Studio 7.
<div align="center">
  <img src="files/BL-Bluetooth-connection-issue-location/">  
</div> 
For easier figure out the frame, it is worth to disable the whitening and set some special data bit on the frame, like add 0x00/0xFF.
<div align="center">
  <img src="files/PR-Configure-TI-CC2541-compatible-proprietary-PHY/Frame.png">  
</div> 

## 3.2. Data on Network Analyzer
To get a readable frame data on MS2692A, it needs to configure the frequency, reference power level etc. accordingly.
### 3.2.1 Use "Power vs Time" trace mode to get the TX pulse
<div align="center">
  <img src="files/PR-Configure-TI-CC2541-compatible-proprietary-PHY/SA-01-pulse.png">  
</div> 

### 3.2.2 Change the "Start Time" to locate one pulse
<div align="center">
  <img src="files/PR-Configure-TI-CC2541-compatible-proprietary-PHY/SA-02-location.png">  
</div> 

### 3.2.3 Zoom in
<div align="center">
  <img src="files/PR-Configure-TI-CC2541-compatible-proprietary-PHY/SA-03-zoom-in.png">  
</div> 

### 3.2.4 Use "Frequency vs Time" trace mode, the whole frame will be show up.
Time between Marker1 and Marker2 is 52us. This should be the preamble time. 13bytes preamble, 2Mbps means one bit take 0.5us, 13x8x0.5us=52us. The MS2629A only detect 1 byte preamble and got 48us CW signal. Still not know the reason, but checking only TI BLE PHY, it got the same issue. This's why it has configured according to given parameters but our EFR32 device can not detect the preamble.
<div align="center">
  <img src="files/PR-Configure-TI-CC2541-compatible-proprietary-PHY/SA-04-frame.png">  
</div> 

### 3.2.5 Read the frame data.
Once the Preamble can be located, the frame data should be readable, just try to sampling the data bit for each 0.5us interval.
<div align="center">
  <img src="files/PR-Configure-TI-CC2541-compatible-proprietary-PHY/SA-05-syncwords.png">  
</div> 

# 4. EFR32xG22 Project
After checking the CC2541 frame data, know detail parameters setting on radio configurator, now it can create and configure on our EFR32xG22 project, here use "Flex (RAIL) - RAILtest" example on SSv5.
<div align="center">
  <img src="files/PR-Configure-TI-CC2541-compatible-proprietary-PHY/railtest.png">  
</div> 

## 4.1. Customize the PHY
Double click radio config file -- "radio_settings.radioconf", select one preset PHY(2450M 2GFSK 2Mbps 1M), then check on "Customized", target PHY's center frequency is 2466MHz, first change frequency to 2466MHz.
<div align="center">
  <img src="files/PR-Configure-TI-CC2541-compatible-proprietary-PHY/customized.png">  
</div> 
Configure according to target PHY. 2Mbps data rate, 500KHz deviation. Configure flexible payload length, use CCITT_16(0x1021) CRC polynomial and 0xFFFF CRC seed. Enable whitening, the given whitening seed is 0xFF, but according to test result it should use 0x01FF on our radio configurator. The Preamble set 8 bit here.
<div align="center">
  <img src="files/PR-Configure-TI-CC2541-compatible-proprietary-PHY/packet.png">  
</div> 

It only got 8 bits preamble, according to [AN1253](https://www.silabs.com/documents/public/application-notes/an1253-efr32-radio-configurator-guide-for-ssv5.pdf), timing detect on preamble should be disable.
<div align="center">
  <img src="files/PR-Configure-TI-CC2541-compatible-proprietary-PHY/timing-window.png">  
</div> 
Uncheck "Number of Symbols in Timing Window" for disable RX side preamble detection, use syncword for timing detection.
<div align="center">
  <img src="files/PR-Configure-TI-CC2541-compatible-proprietary-PHY/timing.png">  
</div> 

Left the other parameter as default, the PHY is done. Save and build, now then software is ready.  
For more information on radio configurator, refer to [AN1253](https://www.silabs.com/documents/public/application-notes/an1253-efr32-radio-configurator-guide-for-ssv5.pdf). 

## 4.2. Confirm the PHY
TI CC2541 RX, EFR32 TX, 20 packets.
<div align="center">
  <img src="files/PR-Configure-TI-CC2541-compatible-proprietary-PHY/EFR32-TX.gif">  
</div> 
TI CC2541 TX, EFR32 RX, 20 packets.
<div align="center">
  <img src="files/PR-Configure-TI-CC2541-compatible-proprietary-PHY/EFR32-RX.gif">  
</div> 

For more information on CLI command, refer to [UG409](https://www.silabs.com/documents/public/user-guides/ug409-railtest-users-guide.pdf).

# 5. Conclusion
By checking PHY configuration from CC2541 TX on MS2692A, and tried several times on its CRC and Whitening configuration, know the parameters on radio configurator, it can get a compatible PHY for both sides.
