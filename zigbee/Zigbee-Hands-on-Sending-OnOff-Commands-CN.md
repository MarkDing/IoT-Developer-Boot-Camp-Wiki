[English](Zigbee-Hands-on-Sending-OnOff-Commands) | 中文

<details>
<summary><font size=5>目录</font> </summary>

- [1. 简介](#1-简介)
  - [1.1. 实验内容](#11-实验内容)
  - [1.2. 目的](#12-目的)
- [2. 基本步骤](#2-基本步骤)
  - [2.1. 硬体需求](#21-硬体需求)
  - [2.2. 软件需求](#22-软件需求)
    - [2.2.1. 检查EmberZNet SDK](#221-检查emberznet-sdk)
    - [2.2.2. 检查工具链](#222-检查工具链)
    - [2.2.3. 使用Gecko Bootloader](#223-使用gecko-bootloader)
- [3. 发送On/Off命令](#3-发送onoff命令)
  - [3.1. Light设备上的命令处理](#31-light设备上的命令处理)
  - [3.2. 从Switch设备发送命令](#32-从switch设备发送命令)
- [4. 测试项目](#4-测试项目)
- [5. 结论](#5-结论)

</details>

***
## [点击这里观看视频课程][video-tutorial]

# 1. 简介
## 1.1. 实验内容
Zigbee快速入门——新兵训练营系列培训的实验环节将涵盖以下四个部分。我们通过这四个部分来向大家逐步展示，如何从零开始构建一个Zigbee应用。

本文档中的实验是“Zigbee快速入门——新兵训练营”系列中的第二部分。 
-   第一部分，由Light构建网络，并使用install code将Switch加入到这个网络。
-   **第二部分，在设备上使用API发送，接收和处理On-Off命令。**
-   第三部分，在Switch端用一个周期事件来执行自定义代码，在我们的实验中是控制LED闪烁。
-   第四部分，在Switch端使用非易失性存储器来存储自定义数据。 

## 1.2. 目的
在之前的动手实验“建网入网”中，我们学习了如何构建基本的集中式Zigbee网络并加入该网络。在本动手实验中，我们将演示如何从Switch节点向Light节点发送开关命令，以操控Light节点的LED。   
与以前的动手实验相同，该网络将使用两个设备，即两个BRD4162A（EFR32MG12）开发板。   
下图说明了该实验的流程。  

<div align="center">
  <img src="files/ZB-Zigbee-Hands-on-Sending-OnOff-Commands/sending_on_off_commands_working_flow.png">  
</div>  
</br>  

我们再次强调，在开始实验之前需要作一些基本的确认，保证硬件平台及软件环境都已经准备就绪，以避免在开发过程中出现不必要的问题。

*** 

# 2. 基本步骤
无论在实验中创建的是什么应用程序或设备类型，在开始开发之前都要作以下检查准备工作。

## 2.1. 硬体需求
* 2个无线入门套件 (WSTK) 主板
* 2个EFR32MG12无线板（BRD4162A）  

## 2.2. 软件需求
确保已在PC上安装了最新的EmberZNet SDK（在本文撰写时为v6.6.4）和兼容的GCC工具链。 

### 2.2.1. 检查EmberZNet SDK
1. 启动Simplicity Studio v4。 
2.	转到Windows ->Preference -> Simplicity Studio-> SDK，确保已安装“ EmberZNet 6.6.4”。  
它是Gecko SDK Suite 2.6.4的一部分，因此并不单独出现。请参见下面的图2-1。
<div align="center">
  <img src="files/ZB-Zigbee-Hands-on-Sending-OnOff-Commands/check_installed_EmberZNet_SDK.png">  
</div>  
<div align="center">
  <b>图2-1 检查已安装的EmberZNet SDK</b>
</div>  

### 2.2.2. 检查工具链
1.	转到Windows->Preference -> Simplicity Studio->Toolchains，确保已安装GCC工具链。
在构建项目时所使用的工具链版本应当与构建SDK附带的库文件的工具链版本相同。用户可以在[此处](https://www.silabs.com/community/software/simplicity-studio/knowledge-base.entry.html/2018/08/22/gecko_sdk_suite_tool-qlc4)找到正确的工具链-SDK对应关系。请参见下面的图2-2。

<div align="center">
  <img src="files/ZB-Zigbee-Hands-on-Sending-OnOff-Commands/check_toolchain.png">  
</div>  
<div align="center">
  <b>图2‑2 检查工具链</b>
</div>  

### 2.2.3. 使用Gecko Bootloader
Bootloader是存储在预留的闪存中的一段程序，可以初始化设备，更新固件image并可能执行某些完整性检查。如果怀疑应用程序没有运行，请始终检查Bootloader，因为缺少Bootloader会导致程序无法运行。  
**注意**: 在本系列实验的开始，强烈建议对设备用Gecko SDK随附的预编译的Bootloader image进行烧录。应当用以“ -combined”结尾的image（例如，bootloader-storage-internal-single-combined.s37）烧录，这个image包含Gecko Bootloader的第一和第二阶段。该image可以在如下位置找到```c:\SiliconLabs\SimplicityStudio\v4\developer\sdks\gecko_sdk_suite\v2.6\platform\bootloader\sample-apps\bootloader-storage-internal-single\efr32mg12p332f1024gl125-brd4162a\```  

想知道有关如何将Gecko Bootloader添加到Zigbee项目的更多信息，请阅读[Zigbee预备课程](Zigbee-Preparatory-Course-CN#32-使用gecko-bootloader)。   
**提示**: 有关Gecko Bootloader的更多信息，请参见下面的文档。    
[UG266: Silicon Labs Gecko Bootloader User’s Guide](https://www.silabs.com/documents/public/user-guides/ug266-gecko-bootloader-user-guide.pdf)    
[UG103.6: Bootloader Fundamentals](https://www.silabs.com/documents/public/user-guides/ug103-06-fundamentals-bootloading.pdf)  
[AN1084: Using the Gecko Bootloader with EmberZNet and Silicon Labs Thread](https://www.silabs.com/documents/public/application-notes/an1084-gecko-bootloader-emberznet-silicon-labs-thread.pdf)  

*** 

# 3. 发送On/Off命令
在之前的动手实验中，我们创建了两个项目，Zigbee_Light_ZC和Zigbee_Switch_ZR，这两个设备现在处于同一网络中，并准备好可以在网络上发送和接收数据。  
在本实验中，Switch设备应基于所按下的按钮来发送“打开/关闭”命令，而Light应用程序应根据接收到的命令打开/关闭LED1。

## 3.1. Light设备上的命令处理
为了能从用户应用层接收到命令，应使用回调函数。  
可以在AppBuilder的“Callbacks”选项卡中启用这些功能。  
打开此选项卡，在“General/ OnOff Cluster”菜单下找到并启用“On”“Off”回调。请参阅图3-1。
<div align="center">
  <img src="files/ZB-Zigbee-Hands-on-Sending-OnOff-Commands/onOff_Cluster_callbacks_enabling.png">  
</div>  
<div align="center">
  <b>图3-1 启用On/Off Cluster的回调函数</b>
</div>  
</br>  

然后按*Generate*按钮。

也许注意到 *\<ProjectName\>_callbacks.c* 在重新生成时未被覆盖，但是 *callback-stub.c* 被覆盖。其背后的原因是ZCL或插件定义的所有回调都可以由协议栈调用。即使用户未使用这些回调，也应将它们放在避免编译器错误的位置。该*callback-stub.c*就是为了这个目的。  
启用回调后，应将其从 *callback-stub.c* 中删除，并留在 *\<ProjectName\>_callbacks.c* 中。这意味着，用户需要将所启用的回调函数手动添加到*Zigbee_Light_ZC_callbacks.c*文件，并实现所需的功能。

<div align="center">
  <img src="files/ZB-Zigbee-Hands-on-Sending-OnOff-Commands/project_explorer.png">  
</div> 
<div align="center">
  <b>Figure 3‑2 实现回调函数</b>
</div>  
</br>   

按如下所示实现应用程序代码。

```
// Sending-OnOff-Commands: Step 1
bool emberAfOnOffClusterOnCallback(void){
  emberAfCorePrintln("On command is received");
  halSetLed(1);
}

bool emberAfOnOffClusterOffCallback(void){
  emberAfCorePrintln("Off command is received");
  halClearLed(1);
}

bool emberAfOnOffClusterToggleCallback(void){
  emberAfCorePrintln("Toggle command is received");
  halToggleLed(1);
}
```

## 3.2. 从Switch设备发送命令
首先，应该找到一个地方来存放我们发送命令的代码。为此，我们使用按钮触发回调。  
按钮操作由**Button Interface**插件处理，因此应将其使能。


<div align="center">
  <img src="files/ZB-Zigbee-Hands-on-Sending-OnOff-Commands/plugin_button_interface_enable.png">  
</div> 
<div align="center">
  <b>Figure 3‑3 使能Button Interface插件</b>
</div>  
</br>  

该插件定义了一些回调函数，因此可以在*Callbacks*选项卡中找到这些回调。移动到此处并同时启用**Button0 Pressed Short**和**Button1 Pressed Short**回调函数，分别发送On和Off命令。
<div align="center">
  <img src="files/ZB-Zigbee-Hands-on-Sending-OnOff-Commands/callback_button_pressed.png">  
</div> 
<div align="center">
  <b>Figure 3‑4 使能回调函数</b>
</div>  
</br>  

保存并点击“Generate”生成。 

与[第3.1章在Light设备上的命令处理](#31-Light设备上的命令处理)类似，将功能“ emberAfPluginButtonInterfaceButton0PressedShortCallback（）”和“ emberAfPluginButtonInterfaceButton1PressedShortCallback（）”手动添加到Zigbee_Switch_ZR_callbacks.c文件中。  
保存修改后的.isc文件，然后按*Generate*。  
每个命令在发送之前都存储在缓冲区中。传输的数据缓冲区应按以下方式构建：  
实际的ZCL命令由以下功能发出。将\<\>替换为“On”或“Off”。

```
emberAfFillCommandOnOffCluster<>()
```

必须设置由哪个endpoint发送到哪个endpoint。 
```
emberAfSetCommandEndpoints(emberAfPrimaryEndpoint(), 1);
```

将消息作为单播发送到设备0x0000，然后发送到协调器。 
```
emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, 0x0000);
```

找到步骤2的注释，并实现完整的功能代码，如下所示。
```
// Sending-OnOff-Commands: Step 2
void emberAfPluginButtonInterfaceButton0PressedShortCallback(uint16_t timePressedMs)
{
  emberAfCorePrintln("Button0 is pressed for %d milliseconds",timePressedMs);

  EmberStatus status;

  emberAfFillCommandOnOffClusterOn()
  emberAfCorePrintln("Command is zcl on-off ON");

  emberAfSetCommandEndpoints(emberAfPrimaryEndpoint(),1);
  status=emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, 0x0000);

  if(status == EMBER_SUCCESS){
    emberAfCorePrintln("Command is successfully sent");
  }else{
    emberAfCorePrintln("Failed to send");
    emberAfCorePrintln("Status code: 0x%x",status);
  }
}

void emberAfPluginButtonInterfaceButton1PressedShortCallback(uint16_t timePressedMs)
{
  emberAfCorePrintln("Button1 is pressed for %d milliseconds",timePressedMs);

  EmberStatus status;

  emberAfFillCommandOnOffClusterOff()
  emberAfCorePrintln("Command is zcl on-off OFF");

  emberAfSetCommandEndpoints(emberAfPrimaryEndpoint(),1);
  status=emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, 0x0000);

  if(status == EMBER_SUCCESS){
    emberAfCorePrintln("Command is successfully sent");
  }else{
    emberAfCorePrintln("Failed to send");
    emberAfCorePrintln("Status code: 0x%x",status);
  }
}
```

***

# 4. 测试项目
前2章介绍了如何使设备能够通过某些API发送和接收命令。  
生成应用程序并将输出文件烧录到目标设备。在对设备进行烧录之前，请退出网络数据捕获状态，因为在连接Network Analyzer（或Energy Profiler）时调试器无法访问芯片。  
**注意**：请**不要**在烧录之前擦除设备，否则“znet” token将被删除，并且设备将无法加入网络，只能按照上一个实验中的说明再次[加入网络](Zigbee-Hands-on-Forming-and-Joining-CN#73-将switch路由器设备上加入网络)。  
按下Button0发送ON命令，您将注意到Light上的LED1打开。  
按下Button1发送OFF命令，您会注意到Light 1的LED1熄灭。  
**注意**：默认情况下，Light节点上的LED0用于指示网络活动，因此，发送命令时还能观察light节点上的LED0闪烁。  
同时，请查看设备的CLI。Switch应在串行控制台上打印如下内容：  
```
Button0 is pressed for 161 milliseconds
Command is zcl on-off ON
Command is successfully sent

Button1 is pressed for 121 milliseconds
Command is zcl on-off OFF
Command is successfully sent
```

Light的串行控制台输出为：
```
Processing message: len=3 profile=0104 cluster=0006
T00000000:RX len 3, ep 01, clus 0x0006 (On/off) FC 01 seq 17 cmd 01 payload[]
On command is received

Processing message: len=3 profile=0104 cluster=0006
T00000000:RX len 3, ep 01, clus 0x0006 (On/off) FC 01 seq 18 cmd 00 payload[]
Off command is received
```

在Network Analyzer中也可以观察到以上事务。请参阅图4-1。

<div align="center">
  <img src="files/ZB-Zigbee-Hands-on-Sending-OnOff-Commands/ZCL_OnOff_commands_in_Network_Analyzer.png">  
</div>  
<div align="center">
  <b>图4-1 Network Analyzer中的ZCL On/Off命令</b>
</div>  
</br>  

以on / off命令为例，来详细说明General ZCL Frame的格式，ZCL Frame格式由ZCL头和ZCL有效数据包组成。General ZCL Frame的格式应如下图所示。
<div align="center">
  <img src="files/ZB-Zigbee-Hands-on-Sending-OnOff-Commands/format_of_the_general_ZCL_frame.png">  
</div>  
<div align="center">
  <b>图4-2 常规ZCL帧的格式</b>
</div> 
</br>  

使用Network Analyzer，您可以捕获“打开/关闭”命令的网络跟踪，如下所示。 
<div align="center">
  <img src="files/ZB-Zigbee-Hands-on-Sending-OnOff-Commands/onoff_command_format.png">  
</div> 
<div align="center">
  <b>图4-3 捕获的打开/关闭命令</b>
</div>  

**Frame Control**  
Frame Control字段的长度为8位，并包含定义命令类型和其他控制标志的信息。Frame Control字段的格式如下图所示。

<div align="center">
  <img src="files/ZB-Zigbee-Hands-on-Sending-OnOff-Commands/format_of_the_frame_control_field.png">  
</div>  
<div align="center">
  <b>图4-4 Frame Control字段的格式</b>
</div> 
</br>  

“On/Off”命令中的**Frame type**为0b1，表示该命令是特定或本地cluster的（On/Off cluster）。  
**Manufacturer Specific**子字段的值在On/Off命令中被设置为false，制造商代码字段将不被包括在所述帧ZCL。  
“On/Off”命令中的**Direction**子字段为0b0，表示该命令正在从cluster的客户端（Switch）发送到Cluster的服务器端（Light）。  
“On/Off”命令中的**Disable Default Response**子字段为0b1。这意味着仅在存在错误的情况下（以及在Zigbee cluster库规范记录的指定条件下），才会返回“默认响应”命令。  

**Manufacturer Code** 制造商代码字段的长度为16位，并为专有扩展指定了分配的制造商代码。如果将帧控制字段的“ Manufacturer Specific”子字段设置为1（指示此命令引用了制造商特定扩展名），则该字段仅应包含在ZCL框架中。  
由于“ On/Off”命令框控制字段的“ Manufacturer Specific”子字段设置为0，因此将不包括“Manufacturer Code”。

**Transaction Sequence Number** 事务序列号字段的长度为8位，标识单个transaction。

**Command Identifier** “命令标识符”字段的长度为8位，用于指定要使用的cluster命令。下面列出了On / Off Cluster的部分命令ID。

ID | Description
-|-
0x00 | Off |
0x01 | On |
0x02 | Toggle |

**Frame Payload**
帧有效数据包字段的长度可变，并且包含特定于各个命令类型的信息。“On”和“Off”命令都没有payload。

***

# 5. 结论
在本实验中，学习了如何发送不同的ZCL命令以及如何从用户应用层处理收到的命令。以及如何使能/禁用不同功能的插件来满足不同的需求。  
还学习了如何使用Network Analyzer评估在Zigbee网络中传输的数据。

[video-tutorial]:https://www.bilibili.com/video/BV1Lf4y11783