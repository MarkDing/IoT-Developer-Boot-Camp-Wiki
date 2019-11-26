# 1. Introduction
In this worksheet we provide a step-by-step guide to create, build and run ZigBee 3.0 applications based on EmberZNet Stack 6.6.4. If you use a later release in the future, most of the instructions should be still applied, although there could be minor differences not foreseen at the time of this document.  
These exercises help you get familiar with ZigBee 3.0 in the EmberZNet Stack, Simplicity Studio v4 development environment, and the Wireless Start Kit (WSTK) with EFR32MG12 SoC. We assume that you have a WSTK and the following software.  

## 1.1. Application features
The boot camp series hands-on workshop will cover four functionalities below, and the application development is split into four steps respectively to show how an application should be built up from the beginning.  

The exercise in this documentation is the first exercise in the "Zigbee Boot Camp" series.  
-   **In the 1st phase, a basic network forming by the Light, and a joining process by the Switch will be realized.**
-   The 2nd part will prepare the devices to transmit, receive, and process the On-Off commands by using APIs.  
-   At the 3rd step the Switch will have a periodic event to execute any custom code, which will be a LED blinking in our case.  
-   The 4th thing to do is to make the Switch to be able to store any custom data in its flash by using Non-volatile memory.  

## 1.2. Purpose
This tutorial will give an overall knowledge about how to build a Light and Switch device from the scratch. For the end of the Lab, the user will be familiar with the Simplicity Studio, fundamental needs to make an SoC to work, SDK source architecture.  

The network will consist of two devices by using board of BRD4162A (EFR32MG12).  
* One of them is the Light. Since the realized network is centralized, it will work as the Coordinator and Trust Center of the network. This device forms and opens the network, permits other devices to join, and manages the security keys.  
* The other device is the Switch. It joins to the opened network and send On-Off commands to the Light.  

*** 

# 2. Fundamental steps
Before all the individual steps would be performed, it's necessary to check some basics to avoid unwanted issues during the development.  
In fact, the prerequisites of the Zigbee boot camp series training is documented in the [Zigbee Preparatory Course](https://github.com/MarkDing/IoT-Developer-Boot-Camp/wiki/Zigbee-Preparatory-Course), we just highlight some items here again to make sure that the development environment is ready on your side.  

## 2.1. Hardware Requirements
* 2 WSTK Main Development Board  
* 2 EFR32MG12 radio boards (BRD4162A)  

## 2.2. Software Requirements
Make sure you have installed the latest EmberZNet SDK (which is v6.6.4 at the time of this document) and compatible GCC toolchain on your PC.  

### 2.2.1. Check EmberZNet SDK
1. Launch Simplicity Studio v4.  
2. Go to Window -> Preference -> Simplicity Studio -> SDKs, make sure "EmberZNet 6.6.4" is installed.  
It is part of the Gecko SDK Suite 2.6.4, therefore it doesn't appear itself alone. See Figure 2‑1 below.  
<div align="center">
  <img src="./images/check_installed_EmberZNet_SDK.png">  
</div>  
<div align="center">
  <b>Figure 2-1 Check installed EmberZNet SDK</b>
</div>  

### 2.2.2. Check Toolchains
1. Go to Windows -> Preference -> Simplicity Studio -> Toolchains, make sure GCC toolchain is installed.  
It is important to use the same toolchain version when building your project that was used to build the libraries supplied as part of the SDK. The list of the proper toolchain-SDK pairing can be found [here](https://www.silabs.com/community/software/simplicity-studio/knowledge-base.entry.html/2018/08/22/gecko_sdk_suite_tool-qlc4). See Figure 2‑2 below.  

<div align="center">
  <img src="./images/check_toolchain.png">  
</div>  
<div align="center">
  <b>Figure 2‑2 Check the Toolchain</b>
</div>  

### 2.2.3. Using Gecko Bootloader
A bootloader is a program stored in reserved flash memory that can initialize a device, update firmware images, and possibly perform some integrity checks. If the application seems to do not running, always check the bootloader, because lack of it causes program crash.  
For how to add Gecko Bootloader to your Zigbee project, please read the [preparatory course](https://github.com/MarkDing/IoT-Developer-Boot-Camp/wiki/Zigbee-Preparatory-Course#using-gecko-bootloader).  
**Hint**: More information about Gecko Bootloader, please find the documentations below.  
[UG266: Silicon Labs Gecko Bootloader User's Guide](https://www.silabs.com/documents/public/user-guides/ug266-gecko-bootloader-user-guide.pdf)  
[UG103.6: Bootloader Fundamentals](https://www.silabs.com/documents/public/user-guides/ug103-06-fundamentals-bootloading.pdf)  
[AN1084: Using the Gecko Bootloader with EmberZNet and Silicon Labs Thread](https://www.silabs.com/documents/public/application-notes/an1084-gecko-bootloader-emberznet-silicon-labs-thread.pdf)  

*** 

# 3. Create Light application
After the previous steps have been done, it's time to realize the 1st feature of the Light device. As discussed before, the Light should be able to form, and open the network.  
The AppBuilder will be used for creating the application. Appbuilder is an interactive GUI tool that allows developers to create and configure most of their Zigbee application.  
Before the builder would be opened, I recommend to select the target board on the left side of the Launcher view. It helps to the AppBuilder to recognize the target device, thus the proper board related configurations (peripherals, pins) are automatically applied.  

1. Go to File -\> New -\> Project. This will bring up the New Project Wizard. See the Figure 3‑1 below.  
<div align="center">
  <img src="./images/Open_AppBuilder.png">  
</div>  
<div align="center">
  <b>Figure 3‑1 Open AppBuilder</b>
</div>  
</br>  

2. Select "Silicon Labs Zigbee". Click Next. See Figure 3‑2.  
<div align="center">
  <img src="./images/select_the_stack_type.png">  
</div>  
<div align="center">
  <b>Figure 3‑2 Select the stack type</b>
</div>  
</br>  

3. Select "EmberZNet 6.6.x GA SoC 6.6.x.0". Click Next. See Figure 3‑3.  
<div align="center">
  <img src="./images/select_stack_version_and_SoC_application_type.png">  
</div>  
<div align="center">
  <b>Figure 3‑3 Select stack version and SoC application type</b>
</div>  
</br>  

4. Choose the "ZigbeeMinimal" sample application. Click Next. See Figure 3‑4.  
    **ZigbeeMinimal**: This is a Zigbee minimal network-layer application suitable as a starting point for new application development.  
<div align="center">
  <img src="./images/select_ZigbeeMinimal_sample_application.png">  
</div>  
<div align="center">
  <b>Figure 3‑4 Select ZigbeeMinimal sample application</b>
</div>  
</br>  

5. Name your project to "Zigbee_Light_ZC" and then Click Next. See Figure 3‑5.  
<div align="center">
  <img src="./images/name_the_project.png">  
</div>  
<div align="center">
  <b>Figure 3‑5 Name the project</b>
</div>  
</br>  

6.  In next window (Project Setup), double check the board is BRD4162A, if not, you can correct it manually. And also check the compiler is "GNU ARM v7.2.1". Click Finish. See Figure 3‑6.  
<div align="center">
  <img src="./images/check_the_board_and_compiler.png">  
</div>  
<div align="center">
  <b>Figure 3‑6 Check the board and compiler</b>
</div>  
</br>  

At this point the project is placed into the default workspace directory, but most of the source files are missing. These files will be later linked or generated according to the AppBuilder settings.  
To open the AppBuilder, click double to the "Zigbee_Light_ZC.isc" file. There are multiple tabs in the file, let's have a closer look at each tab.  

**General**  
This page gives information about the current project configuration, its path, furthermore, shows the selected toolchain and board. Nothing to do with this tab. 
**Note**: It's important to mention that in case of changing the toolchain or the board, please always create a new project rather than modify the project settings.  

**ZCL Clusters**  
One of the most important setting is the ZCL configurations. The type of the device is based on its clusters and attributes. The Silicon Labs pre-defined most of the available device types. In our tutorial it's a "HA Light On/Off Light" kind of device. To enable all the mandatory clusters and attribute for a Light, click on the "ZCL device type" dropdown menu, then select "HA Light On/Off Light" template. See Figure 3‑7.  

<div align="center">
  <img src="./images/select_ZCL_device_type.png">  
</div>  
<div align="center">
  <b>Figure 3‑7 Select ZCL device type</b>
</div>  

After selecting the template, new enabled clusters and attributes are appeared in the list, moreover, the endpoint configuration is changed. These settings are applied based on the Zigbee Specification.  

**Note**: It's important to mention that the ZCL selection in not strictly mandatory for network creating and opening. This step prepares the device to be able to receive and process the On-Off commands in the 2nd step.  
**Note**: It's not possible to modify these templates, therefore the "ZigBee Custom.." should be used if there is need to add any additional cluster.  

**Zigbee Stack**  
This tab lets to change the device type in network aspect. Since the router device cannot form centralized network, the "Coordinator and Router" type must be selected. The default "Zigbee 3.0 Security" is appropriate. See Figure 3‑8.  
<div align="center">
  <img src="./images/change_device_type_to_coordinator.png">  
</div>  
<div align="center">
  <b>Figure 3‑8 Change device type to Coordinator</b>
</div>  

The rest of the settings should not be modified, because the device operates on Single network with basic clusters.  

**Printing and CLI**  
Usually the default setting is enough in this Lab. The only thing to do is verify the "Enable debug printing" box is enabled, and check-in the "On off cluster" debug prints to get more information later. See Figure 3‑9.  
<div align="center">
  <img src="./images/debug_printing.png">  
</div>  
<div align="center">
  <b>Figure 3‑9 Debug printing</b>
</div>  

**Note**: The "On off cluster" debug print also serves the later implemented features in the second hands-on.  

**HAL**  
This tab is modified quite rarely. It would be possible to use external hardware configurator and change bootloader type, but it's rather exists for legacy purposes. In this lab, there is no need to do anything on this tab.  

**Plugin**  
The plugins are individual software packages which implement a functionality. A plugin can consist of libraries and source files as well. These are collected on this tab, and the selection of device type doesn't filter out the plugins that the device cannot use, thus it must be done manually. For example, this sample application doesn't enable the necessary plugins for network forming/opening, we need to do that manually.  
The below plugins must be added or removed to get a device which can operate as a Coordinator. See the Figure below for how to enable the plugins in Appbuilder.

The **Network Creator** and **Network Creator Security** plugins implement the network forming and opening functionality, therefore these are required to have.  
The **Security Link Keys Library** provides the APS security key management which is key feature in Zigbee 3.0. This plugin serves the Trust Center functionality.  
The **Network Steering** and **Update TC Link Key** can be removed, since the device doesn't intend to joint to any network.  
The **ZigBee PRO Stack Library** includes one of the most complex stack libraries. It contains the routing, networking, scanning, neighbor, child-handler and other functionalities. It's mandatory for Coordinator and Router. The sample application uses this plugin by default.  
The **Security Link Keys library** provides management of APS link keys in the key table. It is used by a trust center (coordinator) to manage link keys of devices in the network, or by non trust center devices wishing to manage partner link keys. Therefore it is required to have.
The **Serial** establishes the Command Line Interface (CLI). This interface lets the user to communicate with the SoC. In case of selecting the correct board at project creation phase, the plugin settings should fit to the pinout of the device, but it is also important to double check the values. This application uses UART0 via USB Mini-B Connector. The WSTK Main board has a Board Controller which does the UART-USB conversion. This is the Virtual COM port, which must be enabled separately out of the plugin. It will be detailed later.  

<div align="center">
  <img src="./images/plugins_enable.png">  
</div>  
</br>

Summarized the above, the following table presents the affected plugins.  

<div align="center">
  <img src="./images/plugins_to_check_light.png">  
</div>  
<div align="center">
  <b>Table 3.1 Plugs to check</b>
</div>  
</br>  

Before going ahead, it's a good place to point how the users can find more information about the plugins. As mentioned above, some plugins have source files, not just pre-built libraries. These files can be examined to find some not detailed information about its internal working. The header, and source files can be found at "C:\\SiliconLabs\\SimplicityStudio\\v4\\developer\\sdks\\gecko_sdk_suite\\v2.6\\protocol\\zigbee\\app\\framework", under "plugin", "plugin-soc" and "plugin-host" folders. This separation is used to identify the commonly used, SoC and Host specific plugins.  

These files are available from the AppBuilder as well, but some extra information can be found, as the implemented, defined callbacks and APIs by the plugin. See Figure 3‑10.  
<div align="center">
  <img src="./images/plugin_details.png">  
</div>  
<div align="center">
  <b>Figure 3‑10 Plugin details</b>
</div>  

**Callbacks**  
The callbacks are a set of functions for the implementation of the application level functionalities. Some of them are related to plugins, while others can be used without any limitation. This tab is dynamically changing based on the previous *Plugins* and *ZCL Clusters* tab. It means some callbacks are visible/usable only if the appropriate plugin or cluster has been enabled.  
It's not necessary to use any callback for the basic network forming and opening functionalities. It will be used later.  

**Includes**  
Project specific macros and include paths are defined here. It should not be modified, unless the user would use any custom token, or event. It will be used later.  

**Other options**  
Advance settings in case of using dual band functionalities. It's not used in this project.  

**Bluetooth GATT**  
The Zigbee-BLE Dynamic Multiprotocol bluetooth side configurator is resided into the AppBuilder. It's not used in this project.  
Note: Some BLE related plugin make it editable.  

After saving the modifications the .isc file ready to generate the project files and link the necessary SDK sources and libraries.  

Press the Generate button on the upper-right of the Appbuilder.  

The "Generation successful" label signs all the required files are created. See Figure 3‑11.  
<div align="center">
  <img src="./images/generation_result.png">  
</div>  
<div align="center">
  <b>Figure 3‑11 Generation result</b>
</div>  

**Hardware configurator**  
The hardware configurator is NOT part of the AppBuilder. It's unique file in the project for generating the "hal-config/hal-config.h" file. This header file contains includes, which will be used by other source files.  
It's important to understand that the here applied settings don't directly mean that the peripheral is initialized, it just provides macros for the proper pin and clock settings. It could happen that the UART peripheral is enabled in this configurator, but not in the Serial plugin. In this case the initializer functions will not be called, thus the UART won't work. However, the other way around, if a plugin refers to a macro which is not defined by the hardware configurator, it causes compiler errors.  

In our project, the VCOM enable pin must be enabled to make the UART-USB converter works. The serial port initializer sets the PA5 to high state. If the board was selected correctly at the beginning, this already has been set. See Figure 3‑12.  

<div align="center">
  <img src="./images/hardware_configurator.png">  
</div>  
<div align="center">
  <b>Figure 3‑12 Hardware configurator</b>
</div>  

The saving of this file re-generates the "hal-config.h" file according to the settings.  
Press the Build button (![](./images/build_project.png)). Upon a successful build, the binary files should be appeared in the "Binaries" directory.  

*** 

# 4. Download and test the Light application
Let's download the *Zigbee_Light_ZC.s37* file to the development kit as shown below. See Figure 4‑1 and Figure 4‑2.  
<div align="center">
  <img src="./images/open_Flash_Programmer.png">  
</div>  
<div align="center">
  <b>Figure 4‑1 Open Flash Programmer</b>
</div>  
</br>  

<div align="center">
  <img src="./images/download_the_image.png">  
</div>  
<div align="center">
  <b>Figure 4‑2 Download the image</b>
</div> 
</br>  

The highlighted "Advanced Settings.." provides possibility to decide how to flash the chip. Here the flash can be merged with new image (Merge Content), partially (Page Erase) or completely (Full Erase) erased before downloading the file.  
Keep in mind that neither erase type clean the bootloader section in EFR32MG12 part, but the Full erase deletes the region of the tokens.  
After the image has been downloaded, it's possible to communicate with the device. For this purpose, open the Launch console, which is a built-in serial port terminal in the Studio. See Figure 4‑3.  

<div align="center">
  <img src="./images/open_Serial_console.png">  
</div>  
<div align="center">
  <b>Figure 4‑3 Open Serial console</b>
</div>  
</br>  

If the serial console is opened, switch to "Serial 1" and press "Enter". See Figure 4‑4.  
<div align="center">
  <img src="./images/select_Serial1_tab.png">  
</div>  
<div align="center">
  <b>Figure 4‑4 Select Serial 1 tab</b>
</div>  
</br>  

The "\\n\\r" characters triggers the project name printing. This basic test shows that the RX and TX of the CLI is working correctly.  
If the same text is printed, put a bit away the Light application and start to create the Switch.  

*** 

# 5. Create Switch application
In this hands-on, the Switch is the device that be able to join to the network what is created and opened by the Light.  

The creating of the project and configuration way of the AppBuilder are the same as in case of the Light application, therefore this chapter will include a bit less figure than the Light.  
The project also based on the "ZigBeeMinimal" sample application, so please  
1. Repeat the step 1-6 of chapter [Create Light application](#3-create-light-application), except name the project to "Zigbee_Switch_ZR".  
2. Open the .isc file of the project.  
   * Go to *ZCL Clusters* tab and choose **HA On/Off Switch** device template.  
   * Go to *Zigbee Stack* tab and select the **Router** device type from　the dropdown menu.  
   * Go to *Printing and CLI* tab and double check the "Enable debug printing" is turned on.  
   * Go to *Plugins* tab and double check the below plugins are enabled  
      -   Serial  
      -   Network Steering  
      -   Update TC Link Key  
      -   Install code library

The major difference between the Light application and Switch application is the selection of the network related plugins. Let's have a closer look at the enabled plugins.  
The **Serial** has already been discussed at the Light. It' required for the CLI.  
The **Network Steering** plugin serves to discover the existing networks on the enabled channels. The device issues a Beacon Request message and listens the responses. If the Beacon response (from ZC or ZR) is received with set "permit association" flag, the device starts the joining process for the network, otherwise continue the scanning. Please see the Table 5.1 below for the recommended and required plugins.  
The **Update TC Link Key** is used to request new APS Link Key from the Trust Center. It should be enabled since the Light (Trust Center) has the Security Link Keys Library.  
The **Install code library** provides an initial link key based upon an install code manufacturering token in the device. The key is hashed according to the ZigBee spec.  

<div align="center">
  <img src="./images/plugins_to_check_switch.png">  
</div>  
<div align="center">
  <b>Table 5.1 Plugs to check</b>
</div>  
</br>  

3. Press *Generate* button  
4. Verify the VCOM enable is enabled in the Hardware Configurator (likewise to **3.12 Hardware configurator**)  
5. Build the project  

*** 

# 6. Download and test the Switch application
Please repeat the steps from the chapter [Download and test the Light application](#4-download-and-test-the-light-application) and test if the Switch application works. See Figure 6‑1.  
<div align="center">
  <img src="./images/CLI_testing.png">  
</div>  
<div align="center">
  <b>Figure 6‑1 CLI testing</b>
</div>  

***

# 7. Establish connection between Light and Switch
This chapter presents how to form a network and join to this. The communication between the devices will be captured by Network Analyzer tool.  
At the beginning, open both Light and Switch serial console, and type the "echo 1" command. It helps to track the released command which makes the log more understandable.  
Please perform the following operations:  

## 7.1. Create a centralized network
Command:  
```plugin network-creator start 1```

**Hint**: More information about the CLI command, please find the online documentation [here](https://docs.silabs.com/zigbee/latest/af/group-plugin-network-creator).  

You will get the output similar as below on the serial console, The result: 0x00 means "EMBER_SUCCESS". Find more status codes [here](https://docs.silabs.com/zigbee/latest/em35x/group-status-codes).  

Result:  
```
NWK Creator: Form: 0x00
Zigbee_Light_ZC>NWK Creator Security: Start: 0x00
EMBER_NETWORK_UP 0x0000
NWK Creator: Form. Channel: 25. Status: 0x00
NWK Creator: Stop. Status: 0x00. State: 0x00
```

## 7.2. Find network and device information
Command:  
```info```

Result:  
```
MFG String: 
AppBuilder MFG Code: 0x1002
node [(>)000B57FFFEDEA657] chan [25] pwr [3]
panID [0xD216] nodeID [0x0000] xpan [0x(>)FD7B0901B45A9C4E]
parentID [0xFFFF] parentRssi [0]
stack ver. [6.6.3 GA build 151]
nodeType [0x01]
Security level [05]
network state [02] Buffs: 75 / 75
Ep cnt: 1
ep 1 [endpoint enabled, device enabled] nwk [0] profile [0x0104] devId [0x0100] ver [0x01]
    in (server) cluster: 0x0000 (Basic)
    in (server) cluster: 0x0003 (Identify)
    in (server) cluster: 0x0004 (Groups)
    in (server) cluster: 0x0005 (Scenes)
    in (server) cluster: 0x0006 (On/off)
Nwk cnt: 1
nwk 0 [Primary (pro)]
  nodeType [0x01]
  securityProfile [0x05]
```

## 7.3. Find the Network key for capturing
Command:  
```
keys print
```

Result:  
```
EMBER_SECURITY_LEVEL: 05
NWK Key out FC: 00000013
NWK Key seq num: 0x00
NWK Key: 3F B8 D4 09 4E AD 0A 83  89 A2 7F 1F C0 03 BF 87  
Link Key out FC: 00000000
TC Link Key
Index IEEE Address         In FC     Type  Auth  Key
-     (>)000B57FFFEDEA657  00000000  L     y     C0 1C 81 33 69 98 A3 21  D4 67 92 29 73 59 D0 8E  
Link Key Table
Index IEEE Address         In FC     Type  Auth  Key
0/6 entries used.
Transient Key Table
Index IEEE Address         In FC     TTL(s) Flag    Key    
0 entry consuming 0 packet buffer.
```

## 7.4. Add network key to Network Analyzer
Copy the network key ```C0 1C 81 33 69 98 A3 21  D4 67 92 29 73 59 D0 8E``` and add it to the Network Analyzer's key storage to be able to decode the messages. See Figure 7‑1.  

1. Open Window-\>Preferences  

<div align="center">
  <img src="./images/open_Security_Keys_tab.png">  
</div>  
<div align="center">
  <b>Figure 7‑1 Open Security Keys tab</b>
</div>  
</br>  

2. Make sure that Network Analyzer is set to decode the correct protocol. Select Window \> Preferences \> Network Analyzer \> Decoding \> Stack Versions, and verify it is set correctly. If you need to change it, click the correct stack, click Apply, and then OK.  
<div align="center">
  <img src="./images/stack_profile.png">  
</div>  
</br>  

3. Navigate to Network Analyzer-\>Decoding-\> Security Keys and add the new key. See Figure 7‑2.  
<div align="center">
  <img src="./images/add_new_Network_Key.png">  
</div>  
<div align="center">
  <b>Figure 7‑2 Add new Network Key</b>
</div>  

## 7.5. Start capturing on Light device
Right click on Adapter name of the Light-\> *Connect* (if not connected yet)-\>*Start capture*. See Figure 7‑3.  
<div align="center">
  <img src="./images/start_capturing.png">  
</div>  
<div align="center">
  <b>Figure 7‑3 Start capturing</b>
</div>  
</br>  

It should change the view to *Network Analyzer* and immediately start capturing. See Figure 7‑4.  
<div align="center">
  <img src="./images/capturing_on_Light.png">  
</div>  
<div align="center">
  <b>Figure 7‑4 Capturing on Light</b>
</div>  
</br>  

The capture file (\*Live) should show the packets on the network.  

## 7.6. Open the network
Go back to the Serial console of the Light and permit devices to join.  

Command:  
```
plugin network-creator-security open-network
```

Result:  
```
NWK Creator Security: Open network: 0x00
Zigbee_Light_ZC>pJoin for 254 sec: 0x00
NWK Creator Security: Open network: 0x00
```

This command triggers the Light to send a "Permit Join Request" broadcast message.  
By default, the network will be opened for 300 seconds.  

## 7.7. Join the Switch
Join to this network with the Switch device with TC Link key update.  

Command:
```
plugin network-steering start 0
```

Result:
```
NWK Steering State: Scan Primary Channels and use Install Code
Error: NWK Steering could not setup security: 0xB5
NWK Steering State: Scan Secondary Channels and use Install Code
Error: NWK Steering could not setup security: 0xB5
NWK Steering State: Scan Primary Channels and Use Centralized Key
Starting scan on channel 19
NWK Steering: Start: 0x00
Zigbee_Switch_ZR>Starting scan on channel 20
Starting scan on channel 24
Starting scan on channel 25
NWK Steering joining 0xD216
EMBER_NETWORK_UP 0xDA42
NWK Steering network joined.
Processing message: len=12 profile=0000 cluster=0013
RX: ZDO, command 0x0013, status: 0x00
Device Announce: 0xDA42
Update TC Link Key: Starting update trust center link key process: 0x00
Processing message: len=17 profile=0000 cluster=8002
RX: ZDO, command 0x8002, status: 0x00
RX: Node Desc Resp, Matches: 0x0000
Update TC Link Key: New key established: 0x03
Partner: 57 A6 DE FE FF 57 0B 00 
NWK Steering: Trust center link key update status: 0x03
Update TC Link Key: New key established: 0x65
Partner: 57 A6 DE FE FF 57 0B 00 
NWK Steering: Trust center link key update status: 0x65
pJoin for 180 sec: 0x00
NWK Steering: Broadcasting permit join: 0x00
NWK Steering Stop.  Cleaning up.
Join network complete: 0x00
```

## 7.8. Joining process in Network Analyzer
Have a look at the Network Analyzer how the joining process works. See Figure 7‑5.  

<div align="center">
  <img src="./images/joining_process_in_Network_Analyzer.png">  
</div>  
<div align="center">
  <b>Figure 7‑5 Joining process in Network Analyzer</b>
</div>  
</br>  

**Note**: Probably a lot of "Many-to-One Route Discovery" appear in the log. The upper green filter box can be used to filter these messages out. Right click on this package and "Show only summary: Many…..", then negate the condition from "==" to "!=".  

***

# 8. Establish connection between Light and Switch with an installation code-derived link key
This chapter presents how to form a network and join to this. The communication between the devices will be captured by Network Analyzer tool. The installation code will be used in this part. 
An installation code is used to create a preconfigured, link key. The installation code is transformed into a link key by using the AES-MMO hash algorithm, and the derived Zigbee link key will be known only by the Trust Center and the joining device. So the Trust Center can use that key to securely transport the Zigbee network key to the device. Once the device has the network key, it can communicate at the network layer to the Zigbee network.  

## 8.1 Programming the Installation Code on a Zigbee Device
### 8.1.1 Format of the Installation Code File
To program the installation code, create a simple text file with the value of the installation code (without the CRC). In these instructions
the file is named ```install-code-file.txt```.  
The format of the file is as follows:  
```
Install Code: <ascii-hex>
```

Here is a sample installation code file. The CRC for that code is 0xB5C3 and is not included in the file.  
**Note: Please don't use this example as your installation code directly if you are attending any training session, you can fill it with any hex.**
```
Install Code: 83FED3407A939723A5C639B26916D505
```

### 8.1.2 Checking the Installation Code on an EFR32 Device
To get started, it is best to verify there is connectivity with the device to be programmed, and what information is currently stored on the node.  
To do this, make sure that only the Switch device is connected to your PC (otherwise a new dialog will pop-up for selecting the right device), and then execute the following command to print all manufacturing token data from an EFR32-based device. The ```tokendump``` command prints manufacturing token data as key-value pairs. Simplicity Commander supports more than one group of tokens. In this example, the token group named "znet" is used.  
```
$ commander tokendump --tokengroup znet
```

You should see the following output if you didn't write the installation code before, where the code in highlighted area below reflects the significant fields related to the installation code:  
**Note**: If the ```commander``` command is not available on your PowerShell console, please just jump to the directory firstly and then execute the command.
```
C:\SiliconLabs\SimplicityStudio\v4\developer\adapter_packs\commander
```

<div align="center">
  <img src="./images/check_install_code.png">  
</div>  
<div align="center">
  <b>Figure 8‑1 Check installation code</b>
</div>  

### 8.1.3 Writing the Installation Code into the Manufacturing Area on an EFR32 Device
To write the installation code into the manufacturing area of the Switch node, execute the following command:  
```
$ commander flash --tokengroup znet --tokenfile install-code-file.txt
```
You should see output similar to the following:  
<div align="center">
  <img src="./images/write_the_installation_code.png">  
</div>  

### 8.1.4 Verifying the Stored Installation Code on an EFR32 Device
After writing the installation code, it is best to verify the information by executing the following command again:  
```
$ commander tokendump --tokengroup znet
```
<div align="center">
  <img src="./images/verify_the_installation_code.png">  
</div>  

### 8.1.5 Erasing the Installation Code
**Note**: By default, you don't need to process this step in this hands-on, except you need to update the programmed installation code.  
If you want to remove the install code from the device you just programmed, simply create an installation code file with the contents as below, and then execute the command to program this file into the target.  
```
Install Code: !ERASE!
```

## 8.2 Form centralized network
### 8.2.1 Form centralized network
Open the .isc file of the Light project, navigate to "Plugins" tab, and enable the "Security Link Keys library" plugin. Generate and build the project again.  
After programming the new image to the Light node, use the command below as what we did before to form a centralized network with Zigbee 3.0 security.  
```
plugin network-creator start 1
```
After that, please check the Pan ID of the network, it will be used to identify the network.  
```
network id
```
<div align="center">
  <img src="./images/check_the_network_id.png">  
</div>  
</br>


### 8.2.2 Derive a link key from the installation code
To derive a link key from the installation code and store that into the link key table on the Light, which acts as the Trust Center for the centralized network, enter the command below:  
```
option install-code <link key table index> {<Joining Node's EUI64>} {<16-byte installation code + 2-byte CRC>}
```
For example:  
```
option install-code 0 {00 0B 57 FF FE 64 8D D8} {83 FE D3 40 7A 93 97 23 A5 C6 39 B2 69 16 D5 05 C3 B5}
```

* The first argument is the link key table index.  
* The next argument is the EUI64 of the joining node (in this example, Switch). You can find this information by running the CLI ```info``` command on the Switch node, and looking for the string similar to ```node [(>)000B57FFFE648DD8]```.  

<div align="center">
  <img src="./images/check_device_EUI.png">  
</div>  
</br>

* The last argument is the installation code with the 2-byte CRC appended at the end. You can calculate the CRC yourself, or you can simply find out from running the Simplicity Commander tokendump command:  

```
$ commander tokendump --tokengroup znet
```
<div align="center">
  <img src="./images/verify_the_installation_code.png">  
</div>  
</br>

The CRC is displayed just below the install code and is printed in little endian format. **Reverse the bytes to big endian before using as an argument with the option install-code CLI**.  

To see if the link key is added successfully, enter the keys print CLI on the Light to see it in the Link Key Table. This shows both the link key derived from the installation code, and the network key.  
<div align="center">
  <img src="./images/check_link_key.png">  
</div>  
</br>

As show above, the derived link key is:  
```
66 B6 90 09 81 E1 EE 3C  A4 20 6B 6B 86 1C 02 BB 
```

### 8.2.3 Open the network with the derived link key
Now set the transient link key (the same link key that you derived from the install code) on the Trust Center and open the network for joining with the joining device's EUI64:  
```
plugin network-creator-security open-with-key {eui64} {linkkey}
```
For example:  
```
plugin network-creator-security open-with-key {00 0B 57 FF FE 64 8D D8} {66 B6 90 09 81 E1 EE 3C A4 20 6B 6B 86 1C 02 BB}
```

### 8.3 Join the network
For using the install code on the Switch node, please make sure that the "Install Code Library" plugin is enabled on the joining device, Generate and build the project again.  
<div align="center">
  <img src="./images/enable_install_code_library.png">  
</div>  
</br>

After programming the new image to the Switch node, enter this CLI to use the Network Steering plugin to join the network:  
```
plugin network-steering start 0
```
And the serial console will output similar as below to indicate that the Switch node has joined the network 0xD31F successfully.  

<div align="center">
  <img src="./images/join_network_successfully.png">  
</div>  
</br>

## 8.4 Capture the Network log
This chapter presents how to capture the communication between the devices by Network Analyzer tool. 

## 8.4.1 Find the Network key for capturing
The network key is necessary for analyzing the network log, you can get the network key on the coordinator side with the command below. 

Command:  
```
keys print
```

Result:  
```
EMBER_SECURITY_LEVEL: 05
NWK Key out FC: 00000013
NWK Key seq num: 0x00
NWK Key: 3F B8 D4 09 4E AD 0A 83  89 A2 7F 1F C0 03 BF 87  
Link Key out FC: 00000000
TC Link Key
Index IEEE Address         In FC     Type  Auth  Key
-     (>)000B57FFFEDEA657  00000000  L     y     C0 1C 81 33 69 98 A3 21  D4 67 92 29 73 59 D0 8E  
Link Key Table
Index IEEE Address         In FC     Type  Auth  Key
0/6 entries used.
Transient Key Table
Index IEEE Address         In FC     TTL(s) Flag    Key    
0 entry consuming 0 packet buffer.
```

## 8.4.2 Add network key to Network Analyzer
Copy the network key ```C0 1C 81 33 69 98 A3 21  D4 67 92 29 73 59 D0 8E``` and add it to the Network Analyzer's key storage to be able to decode the messages. See Figure 7‑1.  

1. Open Window-\>Preferences  

<div align="center">
  <img src="./images/open_Security_Keys_tab.png">  
</div>  
<div align="center">
  <b>Figure 7‑1 Open Security Keys tab</b>
</div>  
</br>  

2. Make sure that Network Analyzer is set to decode the correct protocol. Select Window \> Preferences \> Network Analyzer \> Decoding \> Stack Versions, and verify it is set correctly. If you need to change it, click the correct stack, click Apply, and then OK.  
<div align="center">
  <img src="./images/stack_profile.png">  
</div>  
</br>  

3. Navigate to Network Analyzer-\>Decoding-\> Security Keys and add the new key. See Figure 7‑2.  
<div align="center">
  <img src="./images/add_new_Network_Key.png">  
</div>  
<div align="center">
  <b>Figure 7‑2 Add new Network Key</b>
</div>  

## 8.4.3 Start capturing on Light device
Now the Switch should have joined the network created by the Light, please use the command on the Switch for leaving the network firstly.
```
network leave
```

Right click on Adapter name of the Light-\> *Connect* (if not connected yet)-\>*Start capture*. See Figure 7‑3.  
<div align="center">
  <img src="./images/start_capturing.png">  
</div>  
<div align="center">
  <b>Figure 7‑3 Start capturing</b>
</div>  
</br>  

It should change the view to *Network Analyzer* and immediately start capturing. See Figure 7‑4.  
<div align="center">
  <img src="./images/capturing_on_Light.png">  
</div>  
<div align="center">
  <b>Figure 7‑4 Capturing on Light</b>
</div>  
</br>  

And then repeat the step in [Open the network with the derived link key](#823-open-the-network-with-the-derived-link-key-1) to open the network, and step in [Join the network](#83-join-the-network-1) to join the network.
The capture file (\*Live) should show the packets on the network.  

## 8.4.4 Joining process in Network Analyzer
Have a look at the Network Analyzer how the joining process works. See Figure 7‑5.  

<div align="center">
  <img src="./images/joining_process_in_Network_Analyzer_install_code.png">  
</div>  
<div align="center">
  <b>Figure 7‑5 Joining process in Network Analyzer</b>
</div>  
</br>  

**Note**: Probably a lot of "Many-to-One Route Discovery" appear in the log. The upper green filter box can be used to filter these messages out. Right click on this package and "Show only summary: Many…..", then negate the condition from "==" to "!=".  

***

# 8. Conclusion
In this hands-on, you learned how to create your Zigbee application projects starting with ZigbeeMinimal example. And how to configure your application as different type of Zigbee node (coordinator, Router, etc.), how to enable/disable different plugins for different functionality to meet your needs, and how to form a centralized network and join this network.  
Also demonstrates how to evaluate the data being transmitted in the Zigbee network using the Network Analyzer tool.  
