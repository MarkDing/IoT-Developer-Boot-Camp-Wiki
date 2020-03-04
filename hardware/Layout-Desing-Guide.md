
# EFR32 Layout Design Guide

## Why is the proper layout design importan? 
If the layout is designed incorrectly that can cause 
+ Degradation in the communication distance 
+ The unit can violate the relevant EMC regulation (high harmonics, spur) 
+ The battery life can be reduced

## RF circuit 


<p align="center">
  <img src="C:\git\IoT-Developer-Boot-Camp-Wiki\hardware\matching network.png">
</p>

<center> 

**Figure 1. RF circuitry** 

</center>


The most important part of the proper layout design is the RF circuitry itself.
RF part includes, in the case of dual band RFIC, the Sub-GHz matching newtork, the 2.4 GHz matching network, all power supply filterings and the high frequency crystal.        


### Function of RF matching network

The RF transmitter ICs can synthetize high frequency and high power signals at their TX output: fundamental, its harmonics and additional spurious content.
On the Silicon Labs’ reference design boards the function of the TX matching is to drive most of the fundamental power into the antenna, thus to provide the required impedance transformation between the RFIC and the load.
Simultaneously for suppressing harmonics and spurs, a proper filtering network should be used between the matching network and the antennat design is the RF circuitry itself.


> **TX matching:** 
> + Deliver the required TX RF power
> + Impedance transformation
> + Harmonic/spur suppression 



> **RX matching:**  
> + Impedance match  


### Function of VDD filterings

Power supplies, high speed blocks and converters mounted on the board can cause extra spurs in the RF spectrum or sensitivity degradation.
So, the filtering and the isolation away of these blocks from the RF part become important.


### Function of HFXO 


 
## Layout of the RF section    

Silicon Labs suggests copying the RF part of the reference PCB design, or if it is not possible, applying the layout design rules and guidelines described in [AN928.1](https://www.silabs.com/documents/public/application-notes/an928.1-efr32-series1-layout-design-guide.pdf)


<p align="center">
  <img src="C:\git\IoT-Developer-Boot-Camp-Wiki\hardware\RF part.png">
</p>

<center> 

**Figure 2. RF part** 

</center>


### Layout of the matching network   

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



#### Layout Design Practices - 2.4 GHz matching network  


1. Minimize distance between neighboring components. Matching network components should be close to each other and to the RF IC. For nearby components use the width of the pads for trace width

<p align="center">
  <img src="C:\git\IoT-Developer-Boot-Camp-Wiki\hardware\1.png">
</p>

<center> 

**Figure 3.** 

</center>




2. The first component of the matching network should be placed as close to the 2G4RF_IOP pin of the RF IC as possible to reduce the series parasitic inductance and to avoid any detuning effects.

<p align="center">
  <img src="C:\git\IoT-Developer-Boot-Camp-Wiki\hardware\2.png">
</p>

<center> 

**Figure 4.** 

</center>


3. Couplings through the ground can occur between the nearby filtering capacitors (especially at high harmonics), which can decrease the effectiveness of the low-pass filtering
 and can cause higher harmonics that can be observed both conducted and radiated.
 To avoid the possible high harmanic levels, it is recommended to connect the nearby filtering capacitors to ground planes on different sides of the transmission line.

<p align="center">
  <img src="C:\git\IoT-Developer-Boot-Camp-Wiki\hardware\3.png">
</p>

<center> 

**Figure 5.** 

</center>

**Note:** It is true only in the case of Series 1. In the case Series 2 capacitors should be connected on same sides of the transmissin line. 


4. Use at least 0.5 mm separation between traces/pads to the adjacent GND pours in the area of the matching network. This technique will minimize the parasitic capacitance and reduce the detuning effects.

<p align="center">
  <img src="C:\git\IoT-Developer-Boot-Camp-Wiki\hardware\4.png">
</p>

<center> 

**Figure 6.** 

</center>

#### Layout Design Practices - sub-GHz matching network




1. Use the shortest traces possible to connect the first RX matching network component with the RX input pins of the chip.

<p align="center">
  <img src="C:\git\IoT-Developer-Boot-Camp-Wiki\hardware\1s.png">
</p>

<center> 

**Figure 7.** 

</center>


2. To decrease the parasitic capacitance towards the ground, it is recommended to apply a keepout on all inner layers beneath the area of the RX matching network.

<p align="center">
  <img src="C:\git\IoT-Developer-Boot-Camp-Wiki\hardware\2s.png">
</p>

<center> 

**Figure 8.** 

</center>

3. The traces that connect the first TX/RX component(s) with the TX output / RX input of the chip must be routed on different layers. 

<p align="center">
  <img src="C:\git\IoT-Developer-Boot-Camp-Wiki\hardware\3s.png">
</p>

<center> 

**Figure 9.** 

</center>

4. On the first inner layer, the area beneath the remainder sub-GHz matching network (balun and low-pass filter) should be filled with ground metal.
Traces can be routed beneath the area of the balun and low-pass filter sections on all other inner layers.


<p align="center">
  <img src="C:\git\IoT-Developer-Boot-Camp-Wiki\hardware\4s.png">
</p>

<center> 

**Figure 10.** 

</center>

5. No traces should be routed on the layer beneath the traces that connects the first TX matching components with TX pins.



<p align="center">
  <img src="C:\git\IoT-Developer-Boot-Camp-Wiki\hardware\5s.png">
</p>

<center> 

**Figure 11.** 

</center>


6. It is recommended to add an isolating ground metal with many vias between the 2.4 GHz and sub-GHz matching networks.


<p align="center">
  <img src="C:\git\IoT-Developer-Boot-Camp-Wiki\hardware\6s.png">
</p>

<center> 

**Figure 12.** 

</center>

7. Use at least 0.5 mm separation between traces/pads to the adjacent GND pours in the area of the matching network. This technique will minimize the parasitic capacitance and reduce the detuning effects.



<p align="center">
  <img src="C:\git\IoT-Developer-Boot-Camp-Wiki\hardware\7s.png">
</p>

<center> 

**Figure 12.** 

</center>
