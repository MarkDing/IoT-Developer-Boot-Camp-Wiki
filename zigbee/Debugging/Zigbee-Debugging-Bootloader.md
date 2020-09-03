<details>
<summary><font size=5>Table of Contents</font> </summary>
&nbsp;  

- [1. Introduction](#1-introduction)
- [2. Project Settings](#2-project-settings)
  - [2.1. Plugin "Debug"](#21-plugin-debug)
  - [2.2. Hardware Configuration](#22-hardware-configuration)
- [3. Test](#3-test)


</details>
&nbsp; 

# 1. Introduction
In this page, we will introduce an approach to add debug print in the Gecko Bootloader. The debug info will be output through VUART. It's recommended to read [the page about using the VUART](Debugging-With-VUART) first.


# 2. Project Settings
## 2.1. Plugin "Debug"
In Gecko Bootloader, there is already a plugin named "Debug". It implemented the debug print interface through SWO.  

<div align="center">
  <img src="files/ZB-Zigbee-Debugging-Bootloader/plugin-debug.png">  
</div>  
</br>

Please enable the two options:
- Debug prints
- Debug asserts

In the Gecko Bootloader project, it already defined a macro to print debug info.
```
BTL_DEBUG_PRINTLN(str)
```

You can use the macro to add your own debug info.

## 2.2. Hardware Configuration
To enable the debug channel through SWO, you need to make some hardware settings.

1. Open the .isc file of your project, turn to "HAL" tab, then hit the button "Open Hardware Configurator", enable "GPIO" and config the corresponding pin. This is the pin for SWO.

<div align="center">
  <img src="files/ZB-Zigbee-Debugging-Bootloader/hw-config-swo.png">  
</div>  
</br>

2. Enable "Virtual UART". In its properties, select "VUART via SWO" for "VUART Type".

<div align="center">
  <img src="files/ZB-Zigbee-Debugging-Bootloader/hw-config-vuart.png">  
</div>  
</br>


# 3. Test
1. Save and generate the Gecko Bootloader project, then build it and flash it to the board.
2. Setup the VUART connection through SWD following the steps [here](Debugging-With-VUART).
3. When the Gecko Bootloader starts up, you will see the debug info printed out.
```
BTL entry
```

