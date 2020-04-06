
# EFR32 Layout Design Guide

The purpose of this material is to help users design PCBs
for the EFR32 Wireless Gecko Portfolio using design practices
that allow for good RF performance.

PCB layout requirements are described in the application note, [AN928.1: EFR32 Series 1 Layout Design Guide](https://www.silabs.com/documents/public/application-notes/an928.1-efr32-series1-layout-design-guide.pdf),  and the layout design process for EFR32 Series 1 Modules is discussed in [the datasheet of each module](https://www.silabs.com/support/resources.ct-data-sheets.ct-miscellaneous.p-wireless_bluetooth-low-energy_blue-gecko-bluetooth-low-energy-modules.p-wireless_bluetooth-low-energy_blue-gecko-bluetooth-low-energy-modules_bgm11s12f256ga-v2.p-wireless_bluetooth-low-energy_efr32bg1-series-1-modules.p-wireless_bluetooth-low-energy_efr32bg13-series-1-modules.sort=2,asc) and  beside that EFR32 reference design files are available in Simplicity Studio.


## Why is the proper layout design important? 

If the layout is a inappropriate design that can cause 
+ Degradation in the communication distance 
+ The unit can violate the relevant EMC regulation (high harmonics, spur) 
+ The battery life can be reduced

## RF section 


### Different parts of RF section
The most important part of the proper layout design is the RF circuitry itself.

In the case of dual band RFIC, this RF part includes the following:
1. Sub-GHz and 2.4 GHz RF matching newtork  
2. Power supply filterings 
3. High frequency crystal  



<p align="center">
  "<img src="matching network.png">
</p>

<center> 

**Figure 1. RF section** 

</center>

     


#### 1. Function of RF matching  and filtering network

The RF transmitter ICs can synthetize high frequency and high power signals at their TX output: the fundamental, its harmonics and additional spurious content.
On the Silicon Labs’ reference design boards the function of the TX matching is to drive most of the fundamental power into the antenna, thus to provide the required impedance transformation between the RFIC and the load.
Beside that, a proper filtering network should be used between the matching network and the antenna design which is able to suppress harmonics and spurs.


> **TX matching:** 
> + Deliver the required TX RF power
> + Impedance transformation
> + Harmonic/spur suppression 



> **RX matching:**  
> + Impedance match  


#### 2. Function of VDD filterings

Power supplies, high speed blocks and converters mounted on the board can cause extra spurs in the RF spectrum or sensitivity degradation.
So, the filtering and the isolation of these blocks from the RF part is important. The lowest value capacitors filter on the fundamental frequency. The capacitor with the value around 100nF filters the several tenths of MHz frequency range, thus the leaking clock spurs.Filtering these spurs is important as they can be upconverted inside the chip causing unwanted spurs at several 10MHz offset around the carrier. The largest value capacitor filters the several hundred kHz interferences typically coming from a switching power supply. 



****
 
### Layout of RF section    

Silicon Labs suggests copying the RF part of the reference PCB design, or if it is not possible, applying the layout design rules and guidelines described in [AN928.1](https://www.silabs.com/documents/public/application-notes/an928.1-efr32-series1-layout-design-guide.pdf)


<p align="center">
  <img src="RF part.png">
</p>

<center> 

**Figure 2. RF part** 

</center>



#### Layout of the matching network   

The layout design of the match is critical to achieve the targeted power and efficiency. 
This is especially true for the RX path, which can be easily detuned by the PCB parasitics.   
 

Although the size of a matching network is relatively small, due to the high frequency the matching is sensitive to the physical parameters of the matching layout (beside the component values, of course).  
Compared to a reference layout design:
> - Different component distances and trace lengths introduce different parasitic inductances
> - Different substrate thickness, dielectric constant and gaps between traces introduce different parasitic capacitances
> - Different component distance/orientation/relative orientation introduce different coupling between components
> - Different component types/sizes introduce different component parasitics

These can cause detuning of the reference matching or filtering and/or detuning of the load of the RF crystal. The possible effects can be:
> - Decreased TX output power of the fundamental
> - Decreased RX sensitivity
> - Increased spurious emission levels  
> - Increased current consumption
> - Frequency offset between different boards



## Layout Design Practices 
### I. Layout Design Practices - 2.4 GHz matching network  


1. Minimize distance between neighboring components. Matching network components should be close to each other and to the RF IC. For nearby components use the width of the pads for trace width

<p align="center">
  <img src="1.png">
</p>

<center> 

**Figure 3.** 

</center>




2. The first component of the matching network should be placed as close to the 2G4RF_IOP pin of the RF IC as possible to reduce the series parasitic inductance and to avoid any detuning effects.

<p align="center">
  <img src="2.png">
</p>

<center> 

**Figure 4.** 

</center>


3. Couplings through the ground can occur between the nearby filtering capacitors (especially at high harmonics). This can decrease the effectiveness of the low-pass filtering and can cause higher harmonics that can be observed both conducted and radiated.
 To avoid the possible high harmanic levels, it is recommended to connect the nearby filtering capacitors to ground planes on different sides of the transmission line.

<p align="center">
  <img src="3.png">
</p>

<center> 

**Figure 5.** 

</center>

**Note:** It is true only in the case of Series 1. In the case Series 2 capacitors should be connected on same sides of the transmissin line. 


4. Use at least 0.5 mm separation between traces/pads to the adjacent GND pours in the area of the matching network. This technique will minimize the parasitic capacitance and reduce the detuning effects.

<p align="center">
  <img src="4.png">
</p>

<center> 

**Figure 6.** 

</center>

5. The area under the RF chip and the matching network (in 4-layer PCBs, this is the first inner layer beneath the top layer) should be filled with continuous ground metal as it will show good ground reference for the matching network and will ensure a good, low impedance return path to the RF chip’s ground as well. Board routing and wiring should not be placed in this region to prevent coupling effects with the matching network. It is also recommended that the GND return path between the GND vias of the TX/RX matching network and the GND vias of the RFIC paddle should not be blocked in any way; the return currents should see a clear, unhindered pathway through the GND plane to the back of the RFIC.

<p align="center">
  <img src="gnd.png">
</p>

<center> 

**Figure 7.**
</center> 


6. Use 50 Ω grounded coplanar lines where possible for connecting the antenna or the U.FL connector to the matching to reduce sensitivity to PCB thickness variation. This will also reduce radiation and coupling effects. Use many GND vias near the coplanar lines in order to further reduce radiation.

<p align="center">
  <img src="IFA.png">
</p>

<center> 

**Figure 8.** 
</center> 

***
### II. Layout Design Practices - Sub-GHz matching network

The layout consistency for the 4-layer dual-band EFR32 PCB is shown in the following figure. The individual layers will be marked with the currently presented colours during this presentation. Let’s see the sub-GHz EFR32 layout design practices one by one.

<p align="center">
  <img src="layers.png">
</p>

<center> 

**Figure 9.** 

</center>




1. Use the shortest traces possible to connect the first RX matching network component with the RX input pins of the chip. The RX matching network is very sensitive to any extra parasitic inductance and parasitic capacitance, this way it is important to minimize the detuning effects by using as short traces as possible.


<p align="center">
  <img src="1s_2.png">
</p>

<center> 

**Figure 10.** 

</center>


2. In order to decrease the parasitic capacitance towards the ground, it is recommended to apply a keepout on all inner layers beneath the area of the RX matching network.

<p align="center">
  <img src="2s.png">
</p>

<center> 

**Figure 11.** 

</center>

3. The traces, which connect the first TX components with the TX outputs of the chip, must be routed on different layer.  
The figure below shows one example. The RX inputs of the chip are connected to the first RX matching components on the Top layer. But the traces, between first TX components and TX outputs, are routed on the inner layer 2.

<p align="center">
  <img src="3s.png">
</p>

<center> 

**Figure 12.** 

</center>

4. On the first inner layer, the area beneath the remainder sub-GHz matching network (balun and low-pass filter) should be filled with ground metal.
Traces can be routed beneath the area of the balun and low-pass filter sections on all other inner layers.


<p align="center">
  <img src="4s.png">
</p>

<center> 

**Figure 13.** 

</center>

5. No traces should be routed on the layer beneath the traces that connects the first TX matching components with TX pins.



<p align="center">
  <img src="5s.png">
</p>

<center> 

**Figure 14.** 

</center>


6. It is recommended to add an isolating ground metal with many vias between the 2.4 GHz and sub-GHz matching networks.


<p align="center">
  <img src="6s.png">
</p>

<center> 

**Figure 15.** 

</center>

7. Use at least 0.5 mm separation between traces/pads to the adjacent GND pours in the area of the matching network. This technique will minimize the parasitic capacitance and reduce the detuning effects.



<p align="center">
  <img src="7s.png">
</p>

<center> 

**Figure 16.** 

</center>


8. Use 50 Ω grounded coplanar lines where possible for connecting the antenna or the U.FL connector to the matching to reduce sensitivity to PCB thickness variation. This will also reduce radiation and coupling effects. Use many GND vias near the coplanar lines in order to further reduce radiation.

<p align="center">
  <img src="antenna trace.png">
</p>

<center> 

**Figure 17.** 
</center> 

****

### III. Layout Design Practices - HFXO

1. The crystal should be placed as close to the RFIC as possible to ensure that wire parasitic capacitances are kept as low as possible; this will reduce any frequency offsets. Avoid leading supply traces close or beneath the crystal or parallel with a crystal signal or clock trace. 

<p align="center">
  <img src="crystal.png">
</p>

<center> 

**Figure 18.** 

</center>

2. Connect the crystal case to the ground using many vias to avoid radiation of the ungrounded parts. Do not leave any metal unconnected and floating that may be an unwanted radiator. 

<p align="center">
  <img src="crystal2.png">
</p>

<center> 

**Figure 19.** 

</center>

3. Use an isolating ground metal between the crystal and VDD traces to avoid any detuning effects on the crystal caused by the nearby power supply and to avoid the leakage of the crystal or clock signal and its harmonics to the supply lines.

<p align="center">
  <img src="crystal3.png">
</p>

<center> 

**Figure 20.** 

</center>

****
### IV. Layout Design Practices - VDD filtering

1. The lowest value capacitors have to be placed the closest to the VDD pins. Beside that, they need good grounding (with many close vias) at the ground side. The largest value capacitor filters can be placed far away from the VDD pins and is not required in case of battery operation.



<p align="center">
  <img src="VDD.png">
</p>

<center> 

**Figure 21.** 

</center>

***

### V. Layout Design Practices - General

1. Use 50 Ohm grounded coplanar transmission lines to connect distant RF components. Online impedance calculators can be used to get the necessary trace width or see below the typical values for 50 Ohm grounded coplanar lines. 

<p align="center">
  <img src="coplanar2.png">
</p>

<center> 

**Figure 22.** 

</center>

<p align="center">
  <img src="coplanar.png">
</p>

<center> 

**Figure 23.** 

</center>

2. The grounding effect in the thermal straps used with capacitors should be improved. In addition, the trace near the GND pin of these capacitors should be thickened, this will minimize series parasitic inductance between the ground pour and the GND pins. Additional vias placed close to the GND pin of capacitors (thus connecting it to the bottom or inner layer GND plane) will further help reduce these effects.

<p align="center">
  <img src="vias.png">
</p>

<center> 

**Figure 24.** 

</center>

3. The exposed pad footprint for the paddle of the EFR32 IC should use as many vias as possible to ensure good grounding and heat sink capability. In the reference designs there are 25 vias for the 7x7 mm sized package ICs, each with 10 mil diameter. The paddle ground should also be connected to the top layer GND metal, if possible, to further improve RF grounding; this may be accomplished with diagonal trace connections through the corners of the EFR32 IC footprint.

<p align="center">
  <img src="vias2.png">
</p>

<center> 

**Figure 25.** 

</center>

4. Use as many grounding vias at the GND metal edges (especially at the edge of the PCB and along the VDD traces) as possible in order to reduce their harmonic radiation caused by the fringing field.

<p align="center">
  <img src="vias3.png">
</p>

<center> 

**Figure 26.** 

</center>


5. In a design with more than two layers, all of the wires/traces should be placed in one of the inner layers, especially the VDD trace. Also avoid putting it close to the edge of the PCB. The whole top and bottom layers should contain as much continuous GND metallization as possible in order to reduce the traces' radiations.

<p align="center">
  <img src="layers.png">
</p>

<center> 

**Figure 27.** 

</center>



## Quiz 

There are some mistakes in the following design. Please try to find all the problems, which can cause detuning of the fundamental frequency, lower power level, higher current consumption and higher harmonic power level as well. (tip: 7 mistakes)

<p align="center">
  <img src="example.png">
</p>

<center> 

**Figure 28.** 

</center>