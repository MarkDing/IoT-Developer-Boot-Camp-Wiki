[English](Zigbee-Hands-on-Using-Event) | 中文

<details>
<summary><font size=5>目录</font> </summary>

- [1. 简介](#1-简介)
  - [1.1. 实验内容](#11-实验内容)
  - [1.2. 目的](#12-目的)
- [2. 事件的使用](#2-事件的使用)
- [3. 测试项目](#3-测试项目)
- [4. 结论](#4-结论)

</details>

***
## [点击这里观看视频课程][video-tutorial]

# 1. 简介

## 1.1. 实验内容
Zigbee快速入门——新兵训练营培训的实验环节将涵盖以下四个部分。我们通过这四个部分来向大家逐步展示，如何从零开始构建一个Zigbee应用。

本文档中的实验是“Zigbee快速入门——新兵训练营”系列中的第三部分。 
-   第一部分，由Light构建网络，并使用install code将Switch加入到这个网络。
-   第二部分，在设备上使用API发送，接收和处理On-Off命令。
-   **第三部分，在Switch端用一个周期事件来执行自定义代码，在我们的实验中是控制LED闪烁。**
-   第四部分，在Switch端使用非易失性存储器来存储自定义数据。 

## 1.2. 目的
在之前的实验“建网入网”和“发送On/Off命令”中，我们学习了如何组建基本的集中式Zigbee网络和入网，以及如何控制网络中的Switch节点向Light节点发送on-off命令。   
在本实验中，我们将提供详细的步骤，以演示如何利用Zigbee协议栈的事件机制在Switch节点上设定事件。  
下图列出了该动手实验的流程。  

<div align="center">
  <img src="files/ZB-Zigbee-Hands-on-Using-Event/using_event_working_flow.png">  
</div>  
</br>  

**注意**:实验前，请确保硬件和软件均已准备就绪，可以进行开发。请参阅前两个实验的第二章“基本步骤”，以获取更多详细信息。

# 2. 事件的使用
Zigbee应用框架及其关联的cluster 代码通过利用Zigbee协议栈事件机制来设定事件，从而可以在指定的时间间隔运行某段代码。对于更上层，事件机制提供了一个集中入口，所有周期性的动作都可以被用户输入、无线指令或者设备初始化来触发或者取消。这个机制使得Zigbee应用框架能够准确的知道下一个周期性动作将在何时触发。对于需要准确知道何时必须醒来以执行某些操作的睡眠设备，或者由于某些事件正在进行而无法休眠的睡眠终端设备，尤为重要。使用Zigbee事件机制的另一个好处是减少了RAM和Flash占用空间。

Zigbee应用框架的事件有两种类型：自定义事件和cluster事件。自定义事件由用户创建，并且可以在应用程序中随意使用。cluster事件由Zigbee应用框架插件中的cluster实现方式决定。

自定义事件包括两部分：事件处理函数（在事件触发时调用）和EmberEventControl结构体（用于设定事件）。Zigbee应用程序框架和AppBuilder提供了一个图形化界面，用于创建自定义事件并将其添加到应用程序中。

**步骤1：创建自定义事件**  
AppBuilder提供了一种向应用程序添加任何自定义事件的方法。  
基本来说，需要两点： 
-   事件控制器–事件的结构体 
-   事件处理程序–事件触发函数  

打开*AppBuilder* - > *Includes* 选项卡。将自定义事件命令```ledBlinkingEventControl```和回调函数```ledBlinkingEventHandler```分别添加到 *Event Configuration*窗口。参见下图。
<div align="center">
  <img src="files/ZB-Zigbee-Hands-on-Using-Event/custom_event_adding_in_AppBuilder.png">  
</div>  
<div align="center">
  <b>图2-1 在AppBuilder中添加自定义事件</b>
</div>  
</br>  

**第2步：启用MainInit回调** 
事件应当在代码中的某个位置被启用，我们可以在应用程序开始的位置调用相应的函数将其启用。*Main Init*回调函数将被应用程序的*main()*函数调用，它使应用程序有机会在系统启动时进行所需的任何初始化。可以把它理解为 “ main()”函数里面在while(true) ” 前面的函数。   
双击Zigbee_Switch_ZR.isc文件以使用AppBuilder打开它，然后在AppBuilder的“Callbacks”选项卡中启用此回调。参见下图。

<div align="center">
  <img src="files/ZB-Zigbee-Hands-on-Using-Event/main_init_enabling.png">  
</div>  
<div align="center">
  <b>图2-2 启用Main Init回调函数</b>
</div>  
</br>  

保存并点击”Generate”生成项目。

**步骤3：设定事件**  
如前所述，回调函数```emberAfMainInitCallback()```应被添加到*Zigbee_Switch_ZR_callbacks.c*文件中并设定事件。  
相关代码段应类似于以下内容。有关如何使用API设定事件的更多信息，请参阅[API文档](https://docs.silabs.com/zigbee/latest/em35x/group-event)。

```
// Using-event: Step 3
EmberEventControl ledBlinkingEventControl;

void emberAfMainInitCallback(void)
{
  emberEventControlSetDelayMS(ledBlinkingEventControl, 5000);
}

void ledBlinkingEventHandler(void)
{
  // First thing to do inside a delay event is to disable the event till next usage
  emberEventControlSetInactive(ledBlinkingEventControl);

  halToggleLed(1);

  //Reschedule the event after a delay of 2 seconds
  emberEventControlSetDelayMS(ledBlinkingEventControl, 2000);
}
```

需要注意的一点是，应在事件触发函数开始执行后立即将其设置为非活动状态，并在执行完成后重新设定事件。

# 3. 测试项目
编译应用程序，然后将image烧录到Switch设备。按下WSTK上的Reset（复位）按钮，将会看到板上的LED1在延迟几秒钟后打开，然后以2s的间隔闪烁。

# 4. 结论
在本实验中，学习了如何创建自定义事件，定义事件函数和事件控制结构体，实现了设定LED闪烁事件的事件函数。

[video-tutorial]:https://www.bilibili.com/video/BV1DK4y147uW