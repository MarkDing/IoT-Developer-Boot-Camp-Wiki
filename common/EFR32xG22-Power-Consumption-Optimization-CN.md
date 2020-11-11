# EFR32xG22功耗优化
 
[English](EFR32xG22-Power-Consumption-Optimization) | 中文

## 介绍
EFR32xG22最突出的新特性之一就是低功耗，可以在保留32 KB RAM及使用LFRCO的条件下，达到1.40 µA的EM2深度睡眠电流。本文将讨论如何达到EFR32xG22 EM2的最小电流消耗，以及如何降低电流消耗。
 
## 讨论
根据EFR32MG22的数据手册，标准的试验条件为:VREGVDD = 3.0 V，AVDD = DVDD = IOVDD = RFVDD = PAVDD = 1.8 V。EM2模式下选取的DCDC电压缩放电平为VSCALE0时的电压值，且室内温度为 25℃。
 
当创建一个“SoC - Empty”项目时，我们可以测量EM2中的睡眠电流为2.0 µA左右，而不是数据表中提到的1.40 µA。这是由于为了方便开发，在SoC Empty 项目中启用了如VCOM和debug等外围设备。如果不需要，用户可以禁用这些功能来减少功耗。
 
下图显示了在Simplicity Studio中从Energy Profiler测试的结果。从图中，我们可以看到有两个测量结果。左侧为总平均电流，该电流值包括复位时产生的电流。用户应该读取从选择的范围计算出的平均电流。
 
![common](files/CM-Reduce-Current-Consumption/soc-empty-energy-profiler.png) 

根据[UG172](https://www.silabs.com/documents/public/userguides/ug172-brd4320a-user-guide.pdf)“AEM精度和性能”的章节， AEM能够测量0.1 µA到95 mA范围内的电流。对于超过250 µA的电流，AEM精确在0.1 mA之内。当测量电流低于250 µA时，精度增加到1 µA。即使绝对精度是1 µA, 在低于250 µA的范围内，Energy Profiler仍然不够精确以测量低功耗，特别是在深睡眠模式。

此外，从上图可以看出，无线芯片的电压在3.3 V左右，与数据表中提到的3.0 V并不对应。这是因为当使用AEM模式，一个低噪音3.3 V LDO在主板上用来为无线芯片供电。

为了得到更准确的结果，接下来的讨论和测试将严格按照datasheet中的测试条件进行，并使用高精度的直流分析仪代替Energy Profiler。

<div align="center">
<img src="files/CM-Reduce-Current-Consumption/agilent-n6705b.jpg">  
</div>  
</br>

 
本文使用的直流功率分析仪是安捷伦公司的N6705B，其电流表精度可达0.025% + 8 nA。此外，它还提供了数据记录器功能，测量间隔为20 µs到60 s，便于我们得到当前d电流消耗的平均值。 

在下一节中，我们将讨论不同的测试条件和外围设备对电流消耗的影响。


### 输入电压
首先，我们将做一个简单的比较，比较当供电电压为3.0 V和3.3 V时的测试结果。
下图为“SoC Empty Project”在不同电源电压下的测试结果。上面的曲线为供电电压3.0 V时的电流消耗，下面的曲线为供电电压3.3 V时的电流消耗。从下表可以看出，供电电压为3.0 V时的电流消耗要比3.3 V时高。这是因为设备将在EM2中保持恒定的功率。由公式**P = U x I**可知，功率恒定的条件下电压与电流成反比。

![common](files/CM-Reduce-Current-Consumption/InputVoltage_Comparison.png) 
 

### Debugger
可以通过在EMU_CTRL寄存器上设置EM2DBGEN字段来启用调试连接，这将消耗大约0.5 µA的额外电流。在不需要debug功能的条件下，为了减少当前消耗，可以注释下面一行。
```c
//Force PD0B to stay on EM2 entry. This allow debugger to remain connected in EM2
//EMU->CTRL |= EMU_CTRL_EM2DBGEN; 
```


### DCDC
DC-DC降压变换器是一种开关稳压器，能有效地将高输入电压转换为低输出电压，覆盖广泛的负载电流范围，在EM0、EM1、EM2和EM3能量模式下提供高效率。有关DCDC的更多信息，请参阅[AN0948](https://www.silabs.com/documents/public/application-notes/an0948-power-configurations-and-dcdc.pdf).

```c
  // Enable DC-DC converter
  EMU_DCDCInit_TypeDef dcdcInit = EMU_DCDCINIT_DEFAULT;
  EMU_DCDCInit(&dcdcInit);
```

在SoC Empty项目中，DCDC是默认启用的。下图显示了在soc empty项目中禁用Debug模式后，使用DCDC和不使用DCDC的情况下的当前电流曲线比较。从平均电流可以看出，使用DCDC可以节省大量的电流。

![common](files/CM-Reduce-Current-Consumption/DCDC_comparison.png) 


### 外部 flash
BRD4182A radio board中配备的外部flash“MX25R8035F”默认为Standby Mode。EFR32无线电板上使用的MX25R8035F设备在Standby Mode下的典型电流是5µA，这使得观察VS2和VS0电压缩放级别之间的差异非常困难。
幸运的是，JEDEC标准SPI Flash有一个Deep Powerdown Mode，在这种模式下，典型的电流消耗可以达到0.35 µA，通常是0.007 µA。使用下面的命令，MX25将进入Deep Powerdown Mode。

```c
 /* Disable external flash memory*/
  MX25_init();
  MX25_DP();
```  
MX25_init 初始化SPI Flash，调用MX25_DP发送将Flash放入DP模式所需的字节.
 

### Voltage scaling
电压调整通过尽可能在较低的电压下运行系统，优化系统的能源效率。三种电压等级可供使用:

| VSCALE 等级设置 | DECOUPLE Voltage | 使用条件 |
| :-------- | :-------- | :-------- |
| VSCALE2 | 1.1 V | EM0/EM1 下最高可达 80 MHz, EM2 和 EM3 |
| VSCALE1 | 1.0 V | EM0/EM1 下最高可达 40 MHz, EM2 和 EM3 |
| VSCALE0 | 0.9 V | 只能在EM2、EM3下工作  |

EM2和EM3模式下的电压等级可以通过EMU_CTRL_EMU23VSCALE字段设置。将EMU23VSCALE设置为VSCALE0可以使得在EM2和EM3模式下获得最低的睡眠电流。
```c
  EMU_EM23Init_TypeDef vsInit = EMU_EM23INIT_DEFAULT;
  vsInit.vScaleEM23Voltage = emuVScaleEM23_LowPower;
  EMU_EM23Init(&vsInit);
```
在EM2和EM3中有两个电压缩放等级，分别是emuVScaleEM23_LowPower模式 (vscale0) 和emuVScaleEM23_FastWakeup (vscale2)。不同缩放模式下的电流减小将在后面的章节中介绍。
 

### Radio RAM
EFR32xG22设备包含用于各种功能的SRAM块，包括通用数据存储器(RAM)和各种RF子系统RAM (SEQRAM, FRCRAM)。
如果不需要，在EM2/EM3模式下，可以关闭帧速率控制器SRAM(FRCRAM)和序列SRAM(SEQRAM)的所有部分。SYSCFG_RADIORAMRETNCTRL中设置了FRCRAMRETNCTRL和SEQRAMRETNCTRL用于控制这些区域是否保留。
```c
  /* Disable Radio RAM memories (FRC and SEQ) */
  CMU_ClockEnable(cmuClock_SYSCFG, true);
  SYSCFG->RADIORAMRETNCTRL = 0x103UL;
```
**Note** : 上面的命令只能在MCU项目中实现降低功耗。如果FRCRAM和SEQRAM被禁用，wireless stack将无法正常工作。

禁用不同的Radio RAM可以产生不同程度的电流减少。接下来的表格列出了在MCU项目中保留32 KB RAM和不同RADIO RAM的电流消耗结果。

||FRC RETENTION|SEQ RETENTION|BOTH RADIO RAM RETENTION|NO RADIO RAM RETENTION|
|:-----:|:-----:|:-----:|:-----:|:-----:|
|VSCALE2| 2.33 µA| 2.66 µA | 2.75 µA | 2.22 µA |
|VSCALE1| 1.68 µA| 1.89 µA | 1.97 µA| 1.60 µA|
|VSCALE0| 1.24 µA| 1.38 µA | 1.43 µA| 1.19 µA|
  
 
### GPIO
所有未连接的引脚在EFR32上应配置到 Disable模式(高阻抗，无外接电阻)，和reset相关的IO引脚也是禁用的。这可以通过设置 gpioModeDisabled来完成。可参照MX_25deinit函数的设置。
```c
MX25_deinit();
```

如果您使用我们的SDK附带的一些示例 (MCU或无线示例) 来复现EM2当前的功耗测试，请检查VCOM的状态。启用VCOM会增加功耗。在不需要时，禁用VCOM ENABLE引脚以及TX和RX引脚可以降低功耗。
```c
//initVcomEnable();
```


### Peripherals 
EFR32xG22设置了不同的power domain，在外设不使用时，会被降至最低的供应电流。Power domain由EMU自动管理。它包括最低能量功率域(PDHV)、低功率域(PD0)、低功率域A (PD0A)和辅助PD0功率域(PD0B、PD0C等)。当进入EM2或EM3时，如果辅助低功率域中的任何外设(PD0B、PD0C等)被启用，则辅助低功率域中的任何外设将被通电，导致较高的电流。否则，辅助电源域将被关闭。
 
如果在EM2/EM3模式下启用了PD0B中的任何模块，那么整个PD0B将在EM2/EM3中继续运行。因此，在进入EM2时，请确保禁用高功率的外设。

 
### 温度
注意，温度对功耗有**很大**的影响。此测试的推荐环境温度为25℃，如数据手册所示。
 

##### 另外，如果不需要遵循保留32 KB RAM及LFRCO的情况下，您还可以禁用部分RAM或使用其他振荡器代替LFRCO来降低功耗。

 
### SRAM Retention
RAM可以被分为24 KB和8 KB的bank，分别从地址0x20000000和0x20006000开始。默认情况下，EM2/EM3中保留了这两个bank。在不需要用到32 KB RAM的情况下，可以通过关闭其中的bank减少电流消耗。RETNCTRL寄存器控制这两种bank的保留。
```c
/* Disable MCU RAM retention */
// EMU_RamPowerDown(SRAM_BASE, SRAM_BASE + SRAM_SIZE);
/* Power down BLK0 0x20000000 - 0x20006000: 0x01; BLK1 0x20006000 - 0x20008000: */
CMU_ClockEnable(cmuClock_SYSCFG, true);
SYSCFG->DMEM0RETNCTRL = 0x01UL;
```
禁用不同的RAM可以产生不同程度的电流减少。接下来的表格列出了在MCU项目中不保留RADIO RAM和保留部分（或全部）RAM的电流消耗结果。

 ||32 KB RAM|24 KB RAM|8 KB RAM|
|:-----:|:-----:|:-----:|:-----:|
|VSCALE2| 2.22 µA | 2.20 µA | 1.66 µA |
|VSCALE1| 1.60 µA | 1.59 µA | 1.24 µA |
|VSCALE0| 1.19 µA | 1.19 µA | 0.98 µA|

**Note**:完全移除32 KB RAM是没有意义的(可以实现，但是会导致唤醒失败)。

### 低频振荡器设置
LFRCO是芯片内部集成的32.768 kHz RC振荡器，用于不使用外部晶振的低功耗模式。部分系列芯片的LFRCO可以提供精确模式，精确模式下的LFRCO（PLFRCO）在温度变化时可以通过使能硬件，周期性地参照38.4 MHz HFXO进行校准，提供32.768 kHz和+/- 500ppm精度的时钟源。在温度变化时，由于PLFRCO频繁地进行自动校准，往往会增大电流的消耗。

LFXO使用外部32.768 kHz晶振，提供准确的低频时钟。使用LFXO代替PLFRCO作为振荡器，能够降低电流消耗
```c
CMU_LFXOInit_TypeDef lfxoInit = CMU_LFXOINIT_DEFAULT;
CMU_LFXOInit(&lfxoInit);
CMU_OscillatorEnable(cmuOsc_LFRCO, false, false);
CMU_OscillatorEnable(cmuOsc_LFXO, true, true);
CMU_ClockSelectSet(cmuClock_LFXO, cmuSelect_LFXO);
```
根据EFR32xG22的数据手册，使用DC-DC 3.0V 电源供电时，MCU 在 EM2 及 VSCALE0模式下的电流消耗如下表所示：

![common](files/CM-Reduce-Current-Consumption/datasheet-MCU-current-consumption.png) 
 
**Note**: 重置设备后立即进入EM2模式，可能会让debugger不能再被连接。
为了解决这个问题，请将电池盒旁边的WSTK开关设置为USB(关闭EFR电源)。执行简单命令命令行参数"./command .exe device recover"后，立即将开关移动到AEM位置。
 

# 测试实例
实例项目采用了上述大部分策略来降低能耗。由于在MCU方案和无线方案中所涉及的低功耗方法有很大的不同。实验将分别在这两个领域进行介绍。

**Hardware Environment**
    1个WSTK主开发板
    1个EFR32xG22 2.4GHz 6 dBm Radio Board(BRD4182A Rev B04)

**Software Environment**
    Simplicity Studio SV4.0.0.0
    Gecko SDK v2.7.6

**Note**:本文重点讨论降低电流消耗的策略。本文中所提及的降低功耗的方法也可以通过Gecko SDK 3.0.0在Simplicity Studio SV5.0.0.0中实现。
 

### BLE项目示例

#### 实验示例
 
1.使用版本2.13.6或更新版本的Bluetooth SDK创建一个新的SoC - Empty应用程序项目。  

2.打开app.c并注释掉system_boot中的代码禁止广播，以便我们可以测量EM2中的睡眠电流。

```c
      case gecko_evt_system_boot_id:

//        bootMessage(&(evt->data.evt_system_boot));
//        printLog("boot event - starting advertising\r\n");
//
//        /* Set advertising parameters. 100ms advertisement interval.
//         * The first parameter is advertising set handle
//         * The next two parameters are minimum and maximum advertising interval, both in
//         * units of (milliseconds * 1.6).
//         * The last two parameters are duration and maxevents left as default. */
//        gecko_cmd_le_gap_set_advertise_timing(0, 160, 160, 0, 0);
//
//        /* Start general advertising and enable connections. */
//        gecko_cmd_le_gap_start_advertising(0, le_gap_general_discoverable, le_gap_connectable_scannable);
        break;
```

3.注释掉init_mcu.c中的EMU_CTRL_EM2DBGEN，以禁用EM2中的debug功能。

```c
  //EMU->CTRL |= EMU_CTRL_EM2DBGEN;
```

4.注释掉main.c中的VcomEnable以关闭VCOM。
 
 ```c
  //initVcomEnable();
 ```
 
5.编译项目并烧录到 radio board xg22。  
 

#### 实验结果
实验结果显示两分钟内睡眠电流消耗情况。从底部的表中我们可以看到整体的统计数据，平均电流消耗约为1.65 µA。
 
![common](files/CM-Reduce-Current-Consumption/soc-empty-disable-debug.png) 

因为我们是在一个无线BLE项目(SoC Empty Project)中进行测试，所以即使在EM2中也应该保留Radio RAM (FRC和SEQ)，它消耗了大约0.25 µA的额外供电电流。因此，测试结果将会高于1.4 µA。在不需要无线功能的情况下，xG22在MCU项目中可以达到低于1.4 µA的电流消耗。


### MCU项目示例

#### 实验示例

1. 从github示例中导入一个MCU项目。选择“File -> import”然后浏览以导入下面的项目。
```C:\SiliconLabs\SimplicityStudio\v4\developer\sdks\gecko_sdk_suite\v2.7\peripheral_examples\series2\emu\em23_voltage_scaling\SimplicityStudio```

2. 将main.c用本文附带的文件替换。
   
3. 编译并将其下载到你的radio board xg22。


#### 实验结果
从MCU项目的测试结果可以看出，MCU项目可以达到低于1.4 µA的电流消耗。

![common](files/CM-Reduce-Current-Consumption/mcu-noradioram-32ram-v0.png) 


## 总结
从这个实验中我们可以发现启用或禁用不同的外设会对当前消耗产生不同的影响。为了减少电流消耗，建议采用调整电压来优化系统的能源效率。此外，用户应根据自己的需求采取不同的策略，以达到自己情况下的最小消耗。

**Note**:虽然Energy Profiler对低功率测量不够精确，但它可以检测到小到100nA的当前电流消耗变化。如果条件允许，建议使用精度较高的设备。
 
 
## 参考
* [github peripheral example](https://github.com/SiliconLabs/peripheral_examples/tree/master/series2/emu/em23_voltage_scaling)
* [AN969: Measuring Power Consumption on Wireless Gecko Devices](https://www.silabs.com/documents/public/application-notes/an969-measuring-power-consumption.pdf)
* [Enabling sleep mode of the MX25 series SPI flash](https://www.silabs.com/community/wireless/zigbee-and-thread/knowledge-base.entry.html/2018/12/10/enabling_sleep_mode-V2wx)
 
 
### 附加文件
[main.c](files/CM-Reduce-Current-Consumption/main.c)

