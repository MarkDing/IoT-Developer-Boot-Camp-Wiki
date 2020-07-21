
<details>
<summary><font size=5>Table of Contents</font> </summary>
&nbsp;  

- [1. What's VUART](#1-whats-vuart)
- [2. Setup VUART Connection](#2-setup-vuart-connection)
  - [2.1. Connect the J-Link debugger with USB](#21-connect-the-j-link-debugger-with-usb)
  - [2.2. Connect the J-Link debugger with IP](#22-connect-the-j-link-debugger-with-ip)
  - [2.3. Connect the J-Link debugger with USB (Without Simplicity Studio)](#23-connect-the-j-link-debugger-with-usb-without-simplicity-studio)

</details>  

********

# 1. What's VUART
The VUART is short for Virtual UART. It's actually implemented by the SWD debug interface. To use this interface, you just need to connect the four pins of the SWD interface, which are SWCLK, SWDIO, SWO and the GND. All these four pins are included in the 10-pin Mini-Simplicity Connector. Therefore, it's highly recommended to reserve this connector in your custom board. 

In the output direction, debug info will be output through SWO pin from the **ITM** (Instrumentation Trace Macrocell) module of the ARM Cortex core.  

In the input direction, data are write to the device through Segger RTT. This function must be used together with the **WSTK** (Wireless Starter Kit).

For more details about the debug interface, please refer to [AN0043](https://www.silabs.com/documents/public/application-notes/AN0043.pdf). 


# 2. Setup VUART Connection
There are two approaches to setup the VUART connection.

## 2.1. Connect the J-Link debugger with USB
1. Connect your WSTK to PC with USB cable.
2. Select your kit in "Debug Adapters" window, right click and select "Launch Console".
3. Turn to "Serial0" tab, press enter, and you will send the command line prompt.

<div align="center">
  <img src="files/CM-Debugging-With-VUART/usb-debug-info-serial0.png">  
</div>  
</br>

## 2.2. Connect the J-Link debugger with IP
1. On our WSTK, there is a RJ45 ethernet connector on the up-left corner of the main board. You can connect the WSTK to internet with this connector. After that, the main board will get an IP address through DHCP. The IP address will be displayed on the LCD screen.

2. Make sure the WSTK and your PC are in the same subnet. Open Simplicity Studio, then open the preferences through menu **"Window"-->"Preferences"**. Please check the option "Include all network interfaces" if you have multiple network interface. You can also specify the subnet to make the adapter easier to be discovered.

<div align="center">
  <img src="files/CM-Debugging-With-VUART/tcpip-adapters-settings.png">  
</div>  
</br>

3. After that, the WSTK should be discovered automatically. 

<div align="center">
  <img src="files/CM-Debugging-With-VUART/ip-adapters.png">  
</div>  
</br>

4. Select the adapter, right click and then select "Launch console". Choose tab "Serial 0", the debug info from VUART will be displayed here.

<div align="center">
  <img src="files/CM-Debugging-With-VUART/debug-info-in-serial0.png">  
</div>  
</br>

If your application allows input, you can also input commands here.

## 2.3. Connect the J-Link debugger with USB (Without Simplicity Studio)
1. If you don't want to Simplicity Studio built-in console, there is another way.

2. Start a CMD windows, change to **C:\SiliconLabs\SimplicityStudio\v4\developer\adapter_packs\silink**(This path may differ according to your Simplicity Studio installation).

3. Run the following command to map the J-Link debugger to a TCP port.
```
silink -sn 440088231 -automap 4900 -trace=false -polldelay=5000
```

4. Use the terminal tool (i.e. [Teraterm](https://tera-term.en.lo4d.com/windows)) to open the TCP port 4900. **"File"-->"New Connection"**. 

<div align="center">
  <img src="files/CM-Debugging-With-VUART/new-connection.png">  
</div>  
</br>

5. After that, click **OK** to open. Then in the terminal, you can input command and also get output.

<div align="center">
  <img src="files/CM-Debugging-With-VUART/terminal.png">  
</div>  
</br>
