[English](PCB-Layout-Design-Guide) | 中文

<details>

<summary><font size=5>目录</font> </summary>
 
  - [1. 为什么正确PCB布线是很重要的?](#1.-为什么正确PCB布线是很重要的?)
  - [2. 射频电路部分](#2.-射频电路部分)
    - [2.1. 射频电路的不同部分](#2.1.-射频电路的不同部分)
      - [2.1.1. 射频匹配和滤波网络的作用](#2.1.1.-射频匹配和滤波网络的作用)
      - [2.1.2. VDD电源滤波的作用](#2.1.2.-VDD电源滤波的作用)
    - [2.2. 射频部分的布线](#2.2.-射频部分的布线)  
      - [2.2.1. 匹配网络的布线](#2.2.1.-匹配网络的布线)
  - [3. 布线设计实践](#3.-布线设计实践)
    - [3.1. 2.4GHz匹配网络布线设计实践](#3.1.-2.4GHz匹配网络布线设计实践)
    - [3.2. Sub-G匹配网络布线设计实践](#3.2.-Sub-G匹配网络布线设计实践)
    - [3.3. HFXO布线设计实践](#3.3.-HFXO布线设计实践)
    - [3.4. VDD电源滤波布线设计实践](#3.4.-VDD电源滤波布线设计实践)
    - [3.5  总体布线设计实践](#3.5.-总体布线设计实践)
  - [4. 小测验](#4.-小测验)

</details>

# 概述

本文目的是以实践经验帮助客户用EFR32无线系列芯片设计PCB，以达到较好的射频性能。
PCB基于第一代系列SOC的设计布线要求在[AN928.1: EFR32 Series 1 Layout Design Guide](https://www.silabs.com/documents/public/application-notes/an928.1-efr32-series1-layout-design-guide.pdf)里有详细描述。EFR32第一代系列的模块PCB布线在各个模块[the datasheet of each module](https://www.silabs.com/support/resources.ct-data-sheets.ct-miscellaneous.p-wireless_bluetooth-low-energy_blue-gecko-bluetooth-low-energy-modules.p-wireless_bluetooth-low-energy_blue-gecko-bluetooth-low-energy-modules_bgm11s12f256ga-v2.p-wireless_bluetooth-low-energy_efr32bg1-series-1-modules.p-wireless_bluetooth-low-energy_efr32bg13-series-1-modules.sort=2,asc)里讲述。另外在Simplicity Studio里面可以找到EFR32芯片的参考设计文件。

# 1. 为什么正确PCB布线是很重要的?
如果PCB布线不好，可能引起以下问题：
+ 通信距离变差；
+ 产品违背相关EMC规范（高谐波，毛刺）；
+ 缩短电池寿命。

# 2. 射频电路部分 
## 2.1. 射频电路的不同部分
PCB布线最重要的部分是射频电路部分。
对双频段的RFIC来说，射频部分布线包括：
1. Sub-GHz和2.4GHz射频匹配网络 
2. 电源滤波 
3. 高频晶振

<p align="center">
  "<img src="files/HW-PCB-Layout-Design-Guide/matching network.png">
</p>

<div align="center">

<b>Figure 1. RF section</b> 

</div>

  
### 2.1.1. 射频匹配和滤波网络的作用
射频发射器可以产生基波，谐波和包括射频毛刺的高频高能量的信号。在Silicon Labs参考板里Tx匹配电路的目的就是提供最多的基波能量给到天线，这就要求它提供RFIC和负载之间必要的阻抗变换。另外在匹配电路与天线之间设计一个合适滤波网络可以用来抑制谐波和毛刺的成分。接收匹配网络可以提供负载到RFIC的阻抗匹配。
> **发射匹配** 
> + 传输要求的RF发射能量
> + 阻抗变换
> + 谐波/毛刺抑制

> **接收匹配**  
> + 阻抗变换  


### 2.1.2. VDD电源滤波的作用
电源线及板子上的高速模块和变换器可能会产生额外的射频毛刺，或者导致灵敏度的降低。因此高速模块与射频电路部分间的滤波和隔离是非常重要的。最低值的电容滤除基波成分，100nF左右的电容滤除几十MHz的成分，也即是泄露的时钟毛刺。因这些成分可以在芯片内部与基波混频产生离基波几十MHz远的有害毛刺，所以滤除这些成分是很重要的。最大值的电容滤除几百KHz的类似来自开关电源的干扰。
 
## 2.2. 射频部分的布线    
Silicon Labs建议直接拷贝参考PCB板上的RF部分的走线。如果不可能，请尽量遵循[AN928.1](https://www.silabs.com/documents/public/application-notes/an928.1-efr32-series1-layout-design-guide.pdf)里面讲的布线设计规则和指导。

<p align="center">
  <img src="files/HW-PCB-Layout-Design-Guide/RF part.png">
</p>

<div align="center">

<b>Figure 2. RF part</b> 

</div>



### 2.2.1. 匹配网络的布线  
匹配网络布线对达到要求的功率，最优的灵敏度和电源效率来说是非常关键的。PCB寄生参数很容易引起失调，特别对Rx电路更是如此。
尽管匹配网络尺寸比较小，因频率高，匹配电路对匹配布线的物理参数非常敏感（当然包括器件值外）。
与参考方案布线比较来说：
> - 不同的器件距离和线长会引入不同大小的寄生电感
> - 不同的介质厚度，介电常数和间隙产生不同的寄生电容
> - 不同的器件间距离，放置方向及器件相对放置方向导致不同的器件间的耦合作用
> - 器件大小型号的不同也有不同的器件寄生参数
这些将引起参考匹配和滤波电路或晶振负载电容等失调。可能的影响有：
> - 降低基波发射功率
> - 降低接收灵敏度
> - 增加杂散发射水平  
> - 增加电流消耗
> - 不同板子间的频率偏差


# 3. 布线设计实践 
## 3.1. 2.4GHz匹配网络布线设计实践  


1. 减小相邻元器件之间的距离。匹配网络元器件应相互靠近在一起，而且都靠近RF IC。相邻器件走线线宽与焊盘宽度一样。

<p align="center">
  <img src="files/HW-PCB-Layout-Design-Guide/1.png">
</p>

<div align="center">

<b>Figure 3.</b> 

</div>




2. 匹配网络第一个器件需尽可能靠近RF IC的2G4RF_IOP的管脚放置，这将减小串联寄生电感并避免任何失调效应。

<p align="center">
  <img src="files/HW-PCB-Layout-Design-Guide/2.png">
</p>

<div align="center">

<b>Figure 4.</b> 

</div>


3. 相邻滤波电容的地焊盘之间可能出现耦合（特别在高次谐波处）。这将减弱低通滤波器的滤波效果，而产生能在传导和辐射测试中观察到的更高的高次谐波成分。
为避免出现可能过强的高次谐波成分，建议相邻的滤波电容在射频线的两边连接到地平面。
<p align="center">
  <img src="files/HW-PCB-Layout-Design-Guide/3.png">
</p>

<div align="center">

<b>Figure 5.</b> 

</div>

 注： 对于EFR32xG1x第一代芯片是这样的。但对于EFR32xG2x第二代芯片，滤波电容还是应在射频线的同一边连接到地平面。

4. 在匹配网络区域射频线/焊盘与相邻的铺地铜皮之间的间隔最少要0.5毫米。这将减小寄生电容及减轻失调效应。
<p align="center">
  <img src="files/HW-PCB-Layout-Design-Guide/4.png">
</p>

<div align="center">

<b>Figure 6.</b> 

</div>

5. 在RF芯片及匹配网络的下方区域（对于4层PCB，就是在顶层下的首个内层）应铺上连续的实地铜皮，这将为匹配网络提供良好的参考地平面，而且保证了到RF芯片地的良好的低阻抗的回流路径。板子走线不能放在此区域，以避免与匹配网络的耦合效应。同时建议在匹配网络的地过孔和RF IC地焊盘之间的地回流路径不能被任何东西阻挡；回流电流将会看到一个干净的，无阻挡的到RF IC地的地平面路径。

<p align="center">
  <img src="files/HW-PCB-Layout-Design-Guide/gnd.png">
</p>

<div align="center">

<b>Figure 7.</b>
</div> 


6. 尽可能用50欧的带地平面的共面波导线连接天线或U.FL连接器到匹配电路，这将减少不定PCB厚度对匹配的影响。这也将减小辐射或耦合效应。在靠近共面波导线两边多放上地过孔以进一步减少辐射。

<p align="center">
  <img src="files/HW-PCB-Layout-Design-Guide/IFA.png">
</p>

<div align="center">

<b>Figure 8.</b> 
</div> 

***
## 3.2. Sub-G匹配网络布线设计实践

下图展示了4层EFR32双频段连贯的PCB布线。此讲中各层将用当前显示层的颜色标识。下面就一个一个地看看EFR32的Sub-GHz频段的布线。

<p align="center">
  <img src="files/HW-PCB-Layout-Design-Guide/layers.png">
</p>

<div align="center">

<b>Figure 9.</b> 

</div>


1. 用尽可能最短的线连接第一个Rx匹配元件到芯片的输入管脚。Rx匹配网络对额外的寄生电感和电容非常敏感。这样做对减小失调效应来说是非常重要的。

<p align="center">
  <img src="files/HW-PCB-Layout-Design-Guide/1s_2.png">
</p>

<div align="center">

<b>Figure 10.</b> 

</div>


2. 为了减小到地的寄生电容，建议在接收匹配网络下面各层设置铺地禁止区。

<p align="center">
  <img src="files/HW-PCB-Layout-Design-Guide/2s.png">
</p>

<div align="center">

<b>Figure 11.</b> 

</div>

3. 连接第一个Tx匹配元件到芯片的发射管脚的走线必须放到别的层。下图为例子，芯片Rx管脚在顶层连接到第一个Rx匹配器件。但是连接第一个Tx匹配器件到发射输出脚的走线放在内2层上了。

<p align="center">
  <img src="files/HW-PCB-Layout-Design-Guide/3s.png">
</p>

<div align="center">

<b>Figure 12.</b> 

</div>

4. 在第一内层上，匹配网络的下面以外的区域（巴伦和低通滤波器）需要铺上实地铜皮。在匹配网络的下面的其它内层上可以布线。

<p align="center">
  <img src="files/HW-PCB-Layout-Design-Guide/4s.png">
</p>

<div align="center">

<b>Figure 13.</b> 

</div>

5. 在连接第一个Tx匹配器件到发射管脚的走线下方不能布线。

<p align="center">
  <img src="files/HW-PCB-Layout-Design-Guide/5s.png">
</p>

<div align="center">

<b>Figure 14.</b> 

</div>


6. 建议在2.4GHz匹配网络和Sub-GHz匹配网络之间放带地过孔的隔离地铜皮。

<p align="center">
  <img src="files/HW-PCB-Layout-Design-Guide/6s.png">
</p>

<div align="center">

<b>Figure 15.</b> 

</div>

7. 在匹配网络区域射频线/焊盘与相邻的铺地铜皮之间的间隔最少要0.5毫米。这将减小寄生电容及减轻失调效应。

<p align="center">
  <img src="files/HW-PCB-Layout-Design-Guide/7s.png">
</p>

<div align="center">

<b>Figure 16.</b> 

</div>


8. 尽可能用50欧的带地平面的共面波导线连接天线或U.FL连接器到匹配电路，这将减少PCB厚度变化对匹配的影响。这也将减小辐射或耦合效应。在靠近共面波导线两边多放地过孔以进一步减少辐射。

<p align="center">
  <img src="files/HW-PCB-Layout-Design-Guide/antenna trace.png">
</p>

<div align="center">

<b>Figure 17.</b> 
</div> 


## 3.3. HFXO布线设计实践

1. 晶振需尽可能靠近RF IC放置，以保证走线寄生电容尽可能小；这将减小频率偏差。避免电源线靠近或放在晶振下方，或平行于晶振时钟的走线。

<p align="center">
  <img src="files/HW-PCB-Layout-Design-Guide/crystal.png">
</p>

<div align="center">

<b>Figure 18.</b> 

</div>

2. 用带地过孔的地铜皮连接晶振的外壳到地，以避免未接地器件的辐射。不要留任何不接地或悬空的金属，这些金属或引起不必要的辐射。

<p align="center">
  <img src="files/HW-PCB-Layout-Design-Guide/crystal2.png">
</p>

<div align="center">

<b>Figure 19.</b> 

</div>

3. 在晶振和VDD电源线之间放置隔离地铜皮，以避免电源线引起的晶振的失调效应，也避免了晶振或时钟信号线及其谐波成分向电源线的泄露。

<p align="center">
  <img src="files/HW-PCB-Layout-Design-Guide/crystal3.png">
</p>

<div align="center">

<b>Figure 20.</b> 

</div>

****
## 3.4. VDD电源滤波布线设计实践

1. 最低值的去耦电容应靠VDD管脚最近，而且他们需要在接地端很好地接地（用靠近的地过孔）。最大的滤波电容可放在VDD管脚的远处，在电池供电的情况下大电容不是必需的。

<p align="center">
  <img src="files/HW-PCB-Layout-Design-Guide/VDD.png">
</p>

<div align="center">

<b>Figure 21.</b> 

</div>

***

## 3.5. 总体布线设计实践

1. 用50欧的带地共面波导线连接相距较远的RF器件。在线阻抗计算器可用来计算必要线宽和对地间隙，下图展示了50欧姆带地共面线的典型值。

<p align="center">
  <img src="files/HW-PCB-Layout-Design-Guide/coplanar2.png">
</p>

<div align="center">

<b>Figure 22.</b> 

</div>

<p align="center">
  <img src="files/HW-PCB-Layout-Design-Guide/coplanar.png">
</p>

<div align="center">

<b>Figure 23.</b> 

</div>

2. 应加强电容上热焊盘线的接地效果。而且这些电容接地管脚附近的走线需要变宽，这将减小地铜皮到地管脚之间的串联寄生电感。在靠近电容地管脚加更多的地孔（也即连接到底层或中间层参考地平面）有助于进一步减小这一效应。

<p align="center">
  <img src="files/HW-PCB-Layout-Design-Guide/vias.png">
</p>

<div align="center">

<b>Figure 24.</b> 

</div>

3. EFR32芯片中间的暴露地焊盘需打尽可能多的过孔以保证良好接地和散热能力。参考设计中7X7mm封装中的地焊盘上有25个地过孔，每个地过孔直径为10mil。

<p align="center">
  <img src="files/HW-PCB-Layout-Design-Guide/vias2.png">
</p>

<div align="center">

<b>Figure 25.</b> 

</div>

4. 尽可能多的在地铜皮边（特别是PCB板边和VDD电源线周围）放置接地过孔以减少边缘场引起的谐波辐射。

<p align="center">
  <img src="files/HW-PCB-Layout-Design-Guide/vias3.png">
</p>

<div align="center">

<b>Figure 26.</b> 

</div>


5. 在多于2层的PCB设计中，所有走线应放在内层上，特别是VDD电源走线。同时避免电源线靠近PCB板边放置。顶层或底层的整层应尽可能多的放置连续金属化地，以减少来自信号线带来的谐波和杂散辐射。 

<p align="center">
  <img src="files/HW-PCB-Layout-Design-Guide/layers.png">
</p>

<div align="center">

<b>Figure 27.</b> 

</div>



# 4. 小测验

下面的设计中有几处错误，请尝试找出全部错误。这些错误可导致基波频率失调，低的发射功率，高电流消耗，灵敏度损失和高的谐波成分。（提示：7个错误）。

<p align="center">
  <img src="files/HW-PCB-Layout-Design-Guide/example.png">
</p>

<div align="center">

<b>Figure 28.</b> 

</div>