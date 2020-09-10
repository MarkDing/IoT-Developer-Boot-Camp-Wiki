English | [中文](Schematic-Design-Guide-CN)

<details>
<summary><font size=5>Table of Contents</font> </summary>

- [1. Introduction](#1-introduction)
- [2. Find the relative reference design from Silicon Labs' website](#2-Find-the-relative-reference-design-from-Silicon-Labs'-website)
- [3. Hardware design considerations](#3-Hardware-design-considerations)
- [4. RF circuits matching guide](#4-RF-circuits-matching-guide)
- [5. Customize the schematic design to fit the application](#5-Customize-the-schematic-design-to-fit-the-application)

</details>

# 1. Introduction
If the customer wants to design its own schematic of project, first of all the customer should find a reference from Silicon Labs' web site. Then it can study the schematic with hardware design consideration and RF circuits matching guide. The customer makes its own schematic based on the reference design and the final application cases.

# 2. Find the relative reference design from Silicon Labs' website
There are relative reference designs for all of the solutions in the website. Basically the reference designs differ from frequency bands, output power levels, DC-DC regulator configurations, and chip packages. 
- Frequency bands: 

  2.4GHz for: Proprietary, BLE, and Zigbee;

  Sub-G for:  Proprietary, Z-Wave.

- Output power levels: 

  0dBm, +8dBm, +10dBm, +13~14dBm, +20dBm.
- DC-DC regulator configuration: 

  Use it for power efficiency; 
 
  Don’t use it for saving cost.

- Chip packages:

  32/40/48/68 pins QFN package and 125pins BGA package for EFR32 chips.

For example, you want to search the reference schematic of proprietary chip, you can go to the following [web page](https://www.Silabs.com/):
<div align="center">
  <img src="files/HW-Schematic-Design-Guide/Proprietary-Page.png">  
</div> 

In this page, you can move mouse to "Products" tab, then the pop-down menu will appear with Wireless products which include the Proprietary item, then click left button of your mouse. And it will enter the Proprietary product list [web page](https://www.Silabs.com/wireless/proprietary) as following:
<div align="center">
  <img src="files/HW-Schematic-Design-Guide/Proprietary-Product.png">  
</div>

You can find the generations of proprietary products as listed in the bottom table. You can select "EFR32FG1 series 1 sub-GHz and 2.4GHz SoCs" for more [details](https://www.Silabs.com/wireless/proprietary/efr32fg1-series-1-sub-ghz-2-4-ghz-socs):
<div align="center">
  <img src="files/HW-Schematic-Design-Guide/Proprietary-S1.png">  
</div>

In this page, you can find the brief introduction of the proprietary series 1 chips and the parameter list of each chips. Ignore the chips list, you can click the "View Document" in this page and acess the documents list [pages](https://www.Silabs.com/support/resources.p-wireless_proprietary_efr32fg1-series-1-sub-ghz-and-2-4-ghz-socs) directly:
<div align="center">
  <img src="files/HW-Schematic-Design-Guide/Proprietary-Reference.png">  
</div>

For example, If you want to look for 434MHz +10dBm output power reference design, you can click item "EFR32FG1 BRD4251B 2400/433 MHz 10dBm Radio Board Full Design Package" and download the reference design package to your local address.
The BRD4251B schematic for example can be found [here](https://www.Silabs.com/documents/public/schematic-files/EFR32FG1-BRD4251B-B00-schematic.pdf).

But keep in mind that this reference design can be applied to a cluster of chips that have difference packages and frequency band configurations. So you can just replace the chips to your interest chip with different package and frequency band.
# 3. Hardware design considerations
After you get the reference design, you may have the concern why the reference is designed like this. Therefore Silicon Labs provides a lot of application notes or reference manual for customer reference. For basic hardware considerations, Customer can refer to the following application note for detail descriptions: [AN0002.1](https://www.Silabs.com/documents/public/application-notes/an0002.1-efr32-efm32-series-1-hardware-design-considerations.pdf) for EFR32xG1x series 1 chips, and refer to [AN0002.2](https://www.silabs.com/documents/public/application-notes/an0002.2-efr32-efm32-series-2-hardware-design-considerations.pdf) for EFR32xG2x series 2 chips. And there are several hardware considerations in different divisions as following:
- Power supply configurations: 
  - General requirements: VREGVDD = AVDD, must be the highest voltage on  EFR32; DVDD,IOVDD,RFVDD,PAVDD =< AVDD; DECOUPLE < DVDD
  - If internal DC-DC is not used, typically tie all the power pins to the main power supply.
  - If internal DC-DC is used, typically tie VREGVDD, AVDD, IOVDD to the main power supply, tie DVDD, RFVDD to DCDC output (VREGSW), tie PAVDD to DCDC output if TXP(Transmit Power) =< +13dBm, tie PAVDD to VMCU main supply if TXP > +13dBm.

  For more information, please refer to [AN0948](https://www.Silabs.com/documents/public/application-notes/an0948-power-configurations-and-dcdc.pdf) for EFR32xG1x series 1 chips, and refer to [AN0948.2](https://www.silabs.com/documents/public/application-notes/an0948.2-efr32-series-2-power-configurations-and-dcdc.pdf) for EFR32xG2x series 2 chips.
- Debug connector and reset pin:
  - Serial wire debug

    The Serial Wire Debug (SWD) interface is supported by all EFR32 Wireless Gecko Series devices and consists of the SWCLK (clock input) and SWDIO (data in/out) lines, in addition to the optional SWO (serial wire output). The SWO line is used for instrumentation trace and program counter sampling, and is not needed for flash programming and normal debugging. However, it can be valuable in advanced debugging scenarios, and designers are strongly encouraged to include this along with the other SWD signals.And more there are VCOM port and PTI trace port in the mini simplicity studio debug port. This allow the debug port to do powerful program, control, and trace functions.
  The debug port map and description is as following:
  <div align="center">
   <img src="files/HW-Schematic-Design-Guide/SWD-Debug-Port.png">  
  </div>
  
  Most commonly it is recommended to reserve this debug port in custom boards to facilitate the debug and test purpose.

  - JTAG debug

    EFR32 Wireless Gecko Series devices optionally support JTAG debug using the TCLK (clock), TDI (data input), TDO (data output), and TMS (input mode select) lines. TCLK is the JTAG interface clock. TDI carries input data, and is sampled on the rising edge of TCLK. TDO carries output data and is shifted out on the falling edge of TCLK. Finally, TMS is the input mode select signal, and is used to navigate through the Test Access Port (TAP) state machine.
    The 10-pin Cortex debug port is defined as following:
    <div align="center">
     <img src="files/HW-Schematic-Design-Guide/Cortex-Debug-Port.png">  
    </div>
    
  For more information for debug connecors, please refer to [AN0958](https://www.Silabs.com/documents/public/application-notes/an958-mcu-stk-wstk-guide.pdf)
  - Reset pin

    There are a weak pull-up resistor and a low pass filter at reset pin inside the chip. It will allow reset pin floated and prevent noise glitches from outside interfer. The RESETn must not be connected through an external pull-up to an active supply, otherwise driven high as this could damage the device.

- External clock source:

  EFR32 Wireless Gecko Series devices support different external clock sources to generate the high and low frequency clocks in addition to the internal LF and HF RC oscillators. The possible external clock sources for both the LF and HF domains are external oscillators (square or sine wave) or crystals/ceramic resonators.

  - Low Frequency Clock Sources

    An external low frequency clock can be supplied from a crystal/ceramic resonator or an external clock source. It can source a low-frequency clock from an external source such as a TCXO or VCXO.
    <div align="center">
     <img src="files/HW-Schematic-Design-Guide/LF-Connection.png">  
    </div>
  
  - High Frequency Clock Sources

    The high frequency clock can be sourced from a crystal or ceramic resonator or an external square or sine wave source. It can source a high-frequency clock from an external source such as a TCXO or VCXO.
    <div align="center">
     <img src="files/HW-Schematic-Design-Guide/HF-Connection.png">  
    </div>
   
For additional information on the external oscillators, refer to [AN0016.1](https://www.Silabs.com/documents/public/application-notes/an0016.1-efm32-series-1-oscillator-design-considerations.pdf) for EFR32xG1x series 1 chips, and refer to [AN0016.2](https://www.silabs.com/documents/public/application-notes/an0016.2-efr32-series-2-oscillator-design-considerations.pdf) for EFR32xG2x series 2 chips.

# 4. RF circuits matching guide 
The EFR32 Wireless Gecko Series devices include chip variants that provide 2.4 GHz-only operation, sub-GHz-only operation, or dual-band (2.4GHz and sub-GHz)operation.For RF matching circuits design of these bands, there are 2 application notes described the methods. One is for sub-GHz band [AN923](https://www.Silabs.com/documents/public/application-notes/AN923-subGHz-Matching.pdf) for EFR32xG1x series 1 chips, EFR32xG2x series 2 chips have no sub-GHz bands till now, and the other is for 2.4GHz band [AN930](https://www.Silabs.com/documents/public/application-notes/an930-efr32-matching-guide.pdf) for EFR32xG1x series 1 chips, and [AN930.2](https://www.silabs.com/documents/public/application-notes/an930.2-efr32-series-2.pdf) for EFR32xG2x series 2 chips.

The matching effort strives to simultaneously achieve several goals:
- Provide for tying together the TX and RX signal paths, external to the RFIC.
- Provide the desired nominal TX output power level (measured at the connector to the antenna / load).
- Obtain this nominal TX output power at the nominal supply voltage.
- Provide optimal RX Sensitivity.
- Minimize current consumption (i.e., maximize efficiency).
- Comply with regulatory specifications for spurious emissions.

- Matching circuits design for sub-GHz band 

  The sub-GHz LNA and PA circuits in EFR32 RFICs are fully differential and are not tied together inside the chip. As a result, a total of four pins are required on the RFIC to provide access to the LNA and PA circuits: SUBGRF_OP/ON for the TX output, and SUBGRF_IP/IN for the RX input.
  AN923 describes the matching method in detail.

   The matching circuits consist of impedance transformation block, Balun, and Low Pass Filter as following picture shows:
   <div align="center">
    <img src="files/HW-Schematic-Design-Guide/Sub-G-Match-Topology.png">  
   </div>
  
  The matching circuits are classified into 2 types due to the balun difference. One is for low frequency band(<500MHz), and the other is for high frequency band(>500MHz)
  <div align="center">
   <img src="files/HW-Schematic-Design-Guide/Sub-G-Match-LT-500MHz.png">  
  </div>


  <div align="center">
  <img src="files/HW-Schematic-Design-Guide/Sub-G-Match-MT-500MHz.png">  
  </div>

  And more the matching circuits are different from components values which classified with output power levels and sub frequency bands:

  Matching components value table:
  <div align="center">
   <img src="files/HW-Schematic-Design-Guide/Sub-G-Match-Circuits.png">  
  </div>

  Performance table:
  <div align="center">
   <img src="files/HW-Schematic-Design-Guide/Sub-G-Tested-Performance.png">  
  </div>

- Matching circuits design for 2.4GHz band(For EFR32xG1x series 1 chips)

  AN930 describes the 2.4GHz matching method in detail.The 2.4 GHz front end has a unified, single-ended TX and RX pin (2G4RF_IOP), so the TX and RX paths are tied together internally. The 2G4RF_ION TX pin has to be grounded at the pin. Externally, a single-ended matching network and harmonic filtering are required.

  2 main 2.4 GHz matching topologies are presented here:

  A ladder 2-element LC match for up to 10 dBm power levels:
  <div align="center">
   <img src="files/HW-Schematic-Design-Guide/2.4G-2element.png">  
  </div>
 
  A ladder 4-element LCLC match for up to 20 dBm power levels:
  <div align="center">
   <img src="files/HW-Schematic-Design-Guide/2.4G-4element.png">  
  </div>
  
The performances:
   Tx output power and harmonics:
  <div align="center">
   <img src="files/HW-Schematic-Design-Guide/2.4G-Tx-TestResults.png">  
  </div>
  
   Rx sensitivity:
   <div align="center">
    <img src="files/HW-Schematic-Design-Guide/2.4G-Rx-TestResults.png">  
   </div>
  
- Matching circuits design for 2.4GHz band(For EFR32xG2x series 2 chips)

  AN930.2 describes the 2.4GHz matching method in detail. For EFR32xG21 series chips, there are 2 pins(RF2G4-IO1 and RF2G4-IO2) for RF input/output function, the internal switches ground one of the 2 sides to create single-ended inputs/outputs on either RFIO pins. And there are 3 internal PAs inside the chips which require different optimum load impedance. Thus there are several types of matching networks as following:

  - Matching network for 0dBm PA
  
   <div align="center">
   <img src="files/HW-Schematic-Design-Guide/2G4-21-0dBm.png">  
   </div>
 
  - Matching network for +10dBm PA
   
   <div align="center">
   <img src="files/HW-Schematic-Design-Guide/2G4-21-10dBm.png">  
   </div>
  

  - Matching network for 0/+10dBm PA

  <div align="center">
   <img src="files/HW-Schematic-Design-Guide/2G4-21-0-10dBm.png">  
  </div>

  - Matching network for +20dBm PA

  <div align="center">
   <img src="files/HW-Schematic-Design-Guide/2G4-21-20dBm.png">  
  </div>
  
  - The RF performances for these matching networks for EFR32xG21 series chips:

  <div align="center">
   <img src="files/HW-Schematic-Design-Guide/2G4-21-Perf.png">  
  </div>

  For EFR32xG22 series chips, there is only one pin(RF2G4-IO) for RF input/output function. And there are 2 internal PAs inside the chips which require similar optimum load impedance. Thus there is one matching network for both 0dBm and +6dBm PAs, But the DC-blocking capacitor can be removed when +6dBm PA is utilized. As there is one matching network, the matching components' values can be tuned to accomodate different PCB stackups, the following matching circuits show the difference:

  - Combined matching network for both 0/+6dBm PA on a 4-layer PCB

   <div align="center">
   <img src="files/HW-Schematic-Design-Guide/2G4-22-0-6dBm-1.png">  
   </div>

  - Combined matching network for both 0/+6dBm PA on a 4-layer thin PCB

   <div align="center">
   <img src="files/HW-Schematic-Design-Guide/2G4-22-0-6dBm-2.png">  
   </div>

  - Combined matching network for 0dBm PA on a 2-layer PCB

   <div align="center">
   <img src="files/HW-Schematic-Design-Guide/2G4-22-0dBm.png">  
   </div>
  
  - The RF performances for these matching networks for EFR32xG22 series chips:
  
  <div align="center">
   <img src="files/HW-Schematic-Design-Guide/2G4-22-Perf.png">  
  </div>


# 5. Customize the schematic design to fit the application
   There are several points should be considered before a project's schematic design:
- Which protocol do you want to use?
  Proprietary can provide most flexible communication protocol. Zigbee and BLE are more professional with network communication and control. Z-Wave is more safety for door lock application.
- Does your product care about power efficiency? Is your product battery powered?
  Please consider to use internal DC-DC regulator if power efficiency is critical.
- What frequency band does your product work in?
  Basically 2.4GHz is used in Zigbee, BLE, and some cases for Proprietary;
  Sub-G bands are used for Proprietary and Z-wave.
- What output power requirement with your product?
  Consider the output power requirement base on range, RF regulation, and power consumption restriction.

Based on above considerations, The customer can tailor the reference schematic to its own application.



