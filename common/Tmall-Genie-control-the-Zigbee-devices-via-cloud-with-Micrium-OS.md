# Zigbee Router Bridge连接到阿里云

<details>   
<summary><font size=5>目录</font> </summary>

- [1. 使用Silicon Labs公司的Simplicity Studio创建工程](#1-使用silicon-labs公司的simplicity-studio创建工程)
    - [1.1 创建工程](#11-创建工程)
    - [1.2 配置工程](#12-配置工程)
- [2. 移植阿里物联生活平台设备侧SDK](#2-移植阿里物联生活平台设备侧sdk)
    - [2.1 下载SDK](#21-下载sdk)
    - [2.2 配置SDK](#22-配置sdk)
    - [2.3 根据配置提取SDK代码](#23-根据配置提取sdk代码)
    - [2.4 编译Zigbee工程](#24-编译zigbee工程)
    - [2.5 实现wrapper.c中的适配接口](#25-实现wrapperc中的适配接口)
    - [2.6 参照阿里云物联网SDK的网关参考代码实现云对接的功能](#26-参照阿里云物联网sdk的网关参考代码实现云对接的功能)
- [3. 使用WGM110模组连接网络](#3-使用wgm110模组连接网络)
- [4. Zigbee 部分的功能实现](#4-zigbee-部分的功能实现)
    - [4.1 周期性轮询设备](#41-周期性轮询设备)

</details>

***


本项目的主要目的开发一个Zigbee的router，通过这个router将Zigbee设备连接到阿里云，从而实现使用天猫精灵来控制Zigbee网络中的设备。项目的示意图如下：

<div align="center">
<img src="files/CM-Smart-Speaker/arch.png">
</div>

此外，在本项目中，使用Amazon的Echo Plus作为Zigbee的coordinator，首先将Zigbee设备加入由echo plus创建的Zigbee网络中，通过echo plus可以直接控制。另外一方面，本项目开发的router bridge也是加入到echo plus的网络中，然后通过这个bridge将zigbee网络中的设备加入阿里云。最终可以实现同时使用天猫精灵和echo plus来控制。

项目中涉及的平台和组件包括：

*   &nbsp;[天猫精灵平台](https://open.aligenie.com/)，只需购买一台天猫精灵智能音箱，使用淘宝账号登录即可；
*   &nbsp;[阿里物联生活平台](https://living.aliyun.com/)，需要注册帐号，并创建相关产品。
*   &nbsp;Zigbee router, 即本项目开发的目标。由WiFi和Zigbee两部分组成，用UART接口连接二者。

    *   &nbsp;Zigbee部分，基于Silicon Labs公司的EFR32MG12以及EmberZnet SDK开发；
    *   &nbsp;WiFi部分，基于Silicon Labs公司的WGM110模组开发；
*   &nbsp;[Amazon Echo Plus](https://www.amazon.com/All-new-Echo-Plus-2nd-built/dp/B0794W1SKP)，用户需要在手机端下载Alexa APP来控制echo plus 

涉及的资源和资料链接:

*   &nbsp;[阿里物联生活平台](https://living.aliyun.com/doc#index.html)
*   &nbsp;[阿里物联生活平台设备侧SDK](https://github.com/aliyun/iotkit-embedded)
*   &nbsp;[Silicon Labs公司Zigbee产品已经开发工具](https://www.silabs.com/products/wireless/mesh-networking/zigbee)
*   &nbsp;[Amazon alexa开发](https://developer.amazon.com/alexa)

* * *

## 1. 使用Silicon Labs公司的Simplicity Studio创建工程

Silicon Labs公司的Simplicity Studio可以从[官网](https://www.silabs.com/products/development-tools/software/simplicity-studio)下载，建议安装到默认的路径(C盘预留20GB以上的空间)。由于需要下载Zigbee协议SDK, 需要购买[Zigbee开发套件](https://www.silabs.com/products/development-tools/wireless/mesh-networking)才能拥有下载权限。本项目基于EmberZnet SDK 6.5.5版本开发。此外，此项目中还是用到了Micrium OS操作系统，也需要从Simplicity Studio中下载安装。最后由于EmberZnet中支持Micrium OS的插件现在只支持使用IAR来编译，因此需要安装IAR。

### 1.1 创建工程

主要步骤如下：

*   &quot;File&quot;--&gt;&quot;New&quot;--&gt;&quot;Project&quot;;
*   选择&quot;Silicon Labs AppBuilder Project&quot;, 然后点击&quot;Next&quot;继续;
*   选择&quot;ZCL Application Framework V2&quot;, 然后点击&quot;Next&quot;继续;
*   选择&quot;EmberZnet 6.5.5.0 GA Soc 6.5.5.0&quot;, 然后点击&quot;Next&quot;继续;
*   在对话框的左下方, 勾上&quot;Start with a blank application&quot;, 然后点击&quot;Next&quot;继续;
*   输入工程名称, 然后点击&quot;Next&quot;继续;
*   在&quot;Boards&quot;列表框, 选择&quot;BRD4164A&quot;， 然后&quot;Part&quot;框会自动更新成对应的芯片型号。在编译器中选择IAR，最后点击&quot;Finish&quot;完成。 上述步骤完成后，Simplicity Studio会自动打开锁创建的工程对应的isc文件。

### 1.2 配置工程

在打开的isc界面上有多个tab，每个tab都包含不同的配置项目。这里主要的配置包括：

*   &quot;ZCL global&quot; tab，配置其中的&quot;Manufacture&quot;，这里直接选择&quot;Silicon Laboratories 0x1049&quot;;
*   &quot;ZCL Clusters&quot; tab, 首先配置endpoint，为了简便起见，只用一个endpoint。然后选中这个endpoint，将&quot;ZCL device type&quot;设置为&quot;Zigbee Custom&quot;--&gt;&quot;HA devices&quot;--&gt;&quot;HA on/off switch&quot;。接下来勾选上其他一些需要的cluster，例如level control， color control等。
*   &quot;Znet stack&quot; tab，由于本项目是开发一个网关，所以将&quot;Zigbee Device Type&quot;设置为&quot;Coordinator or Router&quot;;
*   &quot;Plugins&quot; tab，勾选一些需要的plugin，具体如下：

    *   NVM3 Library，需要同时去掉Simulated EEPROM version 1 library，并选中Simulated EEPROM version 2 to NVM3 Upgrade Library。
    *   Serial，在右边的属性设置页面，使能USART3，这个将用来跟WiFi模组WGM110通信；
    *   device table;
    *   Network Steering和Update TC Link key，这两个plugin将用于控制router加入Zigbee网络;
    *   支持Micrium OS，由于需要支持WiFi模组以及集成阿里物联网平台的SDK，需要有OS的支持。这里涉及几个plugin的替换： * 取消Simple Main，使能Micrium RTOS * 取消Zigbee PRO Stack Library，使能Zigbee PRO MbedTls Library for TRNG * 使能mbed TLS Common和mbed TLS TRNG Configuration
*   &quot;HAL Configurat&quot; tab，点击按钮&quot;Hardware Configurator&quot;，然后勾选上Virtual Serial Port
*   &quot;Callbacks&quot; tab，使能如下几个callback:

    *   emberAfMainInitCallback 用于完成一些初始化的工作
    *   emberAfHalButtonIsrCallback 用于配置按键来触发WiFi配网
    *   emberAfPluginMicriumRtosAppTask1InitCallback 完成task1的一些初始化工作
    *   emberAfPluginMicriumRtosAppTask1MainLoopCallback task1的主循环
    *   emberAfPluginDeviceTableNewDeviceCallback 新设备入网时，触发动作通知云端
    *   emberAfPluginDeviceTableDeviceLeftCallback 设备离网，触发动作通知云端
*   &quot;Includes&quot; tab, 添加如下几个event

    *   addSubDevEventControl 在网关中设备加网离网时，向云端增加删除设备
    *   clearWiFiEventControl 由按钮触发，清除当前的WiFi密码，进入WiFi配网模式
    *   commissionEventControl 启动时自动搜索并加入网络
    *   pollAttrEventControl 周期性轮询设备的属性，同时上报给云端

完成上述动作之后，保存，然后&quot;Generate&quot;生成代码，这样整个工程就建立完成了。编译工程，确保编译通过。 接下来就需要逐步增加功能，并完善。

* * *

## 2. 移植阿里物联生活平台设备侧SDK

参考[阿里物联生活平台的开发指南](https://living.aliyun.com/doc#index.html)，首先下载最新版本的[SDK(V3.01)](https://github.com/aliyun/iotkit-embedded), 参考SDK的[用户手册](https://code.aliyun.com/edward.yangx/public-docs/wikis/user-guide/Linkkit_User_Manual)进行配置和移植。

### 2.1 下载SDK

    https://github.com/aliyun/iotkit-embedded.git

### 2.2 配置SDK

以windows为例，运行config.bat脚本，会弹出如下配置窗口：

<div align="center">
<img src="files/CM-Smart-Speaker/iotsdk_config.png">
</div>

用方向键进行控制，空格键或者Enter进行选择或者取消，ESC键返回。
在默认配置的基础上，需要配置的选项有：

*   &nbsp;&quot;PLATFORM HAS OS&quot;
*   &nbsp;&quot;FEATURE DYNAMIC REGISTER&quot;
*   &nbsp;&quot;Device Model Configuration&quot; 菜单下：

    *   &nbsp;&quot;FEATURE DEVICE MODEL GATEWAY&quot;
*   &nbsp;&quot;FEATURE DEV RESET&quot;
*   &nbsp;&quot;FEATURE WIFI PROVISION ENABLED&quot;，然后子菜单下：

    *   &nbsp;&quot;FEATURE AWSS SUPPORT DEV AP&quot;
*   &nbsp;&quot;FEATURE DEV BIND ENABLED&quot;
保存配置退出。

### 2.3 根据配置提取SDK代码

配置完成后，用户的配置保存在make.settings文件中。运行extract.bat脚本，自动提取相应的SDK源码，放置在output/eng文件夹下面。将eng文件夹拷贝到前面创建的Zigbee工程中，将eng文件夹名称修改为iotkit-embeded-sdk。

### 2.4 编译Zigbee工程

编译Zigbee工程，会有一些编译错误。主要原因是提取的SDK的头文件路径没有包含到工程中。
选择工程，右键，&quot;Properties&quot;，将SDK各个部分的路径添加到Include中，如下图：

<div align="center">
<img src="files/CM-Smart-Speaker/project_include.png">
</div>

反复检查修改，直至编译通过。

### 2.5 实现wrapper.c中的适配接口

wrapper.c中有一些接口是需要根据各自平台的具体情况来实现。这里我们使用的是运行在EFR32(ARM Cortex-M4)上的Micrium OS, IP通信依赖WGM110模组来实现。
wrapper.c中的接口主要分成几类：

*   IP 通信相关接口，包括：

    *   TCP 连接以及读写
    *   UDP 读写
    *   TLS 连接以及读写
    *   读取本地IP以及MAC
*   操作系统相关接口：

    *   定时器Timer
    *   内存分配Memory (malloc/free)
    *   信号量Semaphore
    *   互斥量Mutex
    *   任务管理Task
*   AES加密解密
*   WiFi配网相关接口
*   其他平台接口，包括：

    *   打印和格式化
    *   获取运行tick
    *   随机数生成
*   对接阿里物联网平台所需的接口，包括：

    *   读取设备名称和产品key
    *   读取和保存设备secret
    *   读取和保存平台特有的key-value对

其中OS相关的接口可以参考[Micrium OS的文档和手册](https://doc.micrium.com/pages/viewpage.action?pageId=10753180)来逐步实现。

### 2.6 参照阿里云物联网SDK的网关参考代码实现云对接的功能

主要参考linkkit_example_gateway.c，将其修改为动态注册，以便支持一型一密。
设备名称采用Eui64+Endpoint的方式来格式化，例如Eui64为000D6F00056AE60A的设备，有两个endpoint，1和2，因此需要要云端添加两个子设备，子设备名称分别是000D6F00056AE60A_1和000D6F00056AE60A_2。这样做的好处是解析云端的命令时，可以直接根据云端的devid提取设备名称，进而知道要操作的Zigbee设备的Eui64和endpoint。

* * *

## 3. 使用WGM110模组连接网络

开发WGM110的驱动软件，主要参考[API手册](https://www.silabs.com/documents/login/reference-manuals/wgm110-api-rm.pdf)。需要注意的是，EFR32与WGM110是通过UART连接的，速率115200，因此报文收发速度有限。另外，发送比较大的包时，需要分割成128字节的片来发送，否则容易出错。 WGM110驱动实现主要包括以下几个部分，详细的情况直接参考[代码](https://github.com/Jim-tech/Iot/blob/RouterBridge/Z3Aliyun3011/wifi)

*   串口初始化和读写
*   WiFi 消息接收任务
*   所有WiFi相关的接口都用的是同步方式实现，即发送命令给WiFi模组后，同步等待回应直至超时
*   分别实现TCP/UDP的数据接收队列

## 4. Zigbee 部分的功能实现

### 4.1 周期性轮询设备

目前主要通过广播查询设备属性，通过判断是否能接收到响应消息来判断设备是否在线。如果设备有回应，进行device table的发现过程，从而获取该设备支持的endpoint以及cluster信息。 然后向云端添加设备。
