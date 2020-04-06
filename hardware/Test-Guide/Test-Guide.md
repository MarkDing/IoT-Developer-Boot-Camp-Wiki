

If we would like to have devices which can work appropriately and beside that, they are able to comply with requirement of certification we need to perform a few (pre)tests and tuning as listed below.

1. Determination of antenna matching network 
2. HFXO Capacitor Bank (CTune) calibration 
3. Conducted harmonic measurement
4. Conducted sensitvity measurement 
4. 



## Contucted output power measurement

### Why is Ctune calibration important?
---

The **RailTest tool** can be used to verify the RF performance of a device without the overhead of the any protocol. 

The output power should be measured with a spectrum analyzer.

 antenna should be removed




 The following strategy is proposed to determine the optinal Ctune value:

1. Performe a conducted measurement 
2. Connect custom board to a spectrum analyzer according to ..... section
3. Use a spectrum analyzer to measure the output harmonics of the radio

## Conducted sensitivity measurement


## HFXO Capacitor Bank (CTune) calibration 

### Why is Ctune calibration important?
---

It is **recommended to calibrate the HFXO frequency** for the devices to ensure minimum error of the radio carrier frequency. The CTune is responsible for the quality of the radio link, because it sets up HFXO clock properly which is used for the radio clock as well. This radio clock is used by the synthesizer. Thus incorrect setting may lead to broke a radio link between two devices. 


**All Silicon Labs modules and radio boards are factory calibrated**, and the CTune value is stored on the device information page, while other Silicon Labs radio boards equipped with an external EEPROM which holds the CTune value. 

However, **on custom boards, the calibration should be always performed**. It is a preferable way to measure 10-20 board's HFXO frequency per design, and get the average CTune value which can be used for all devices. 

### Measurements
----
The following strategy is proposed to determine the optimal Ctune value:

1. Performe a conducted measurement 
2. Connect the custom board to a spectrum analyzer according to ..... section
3. Use a spectrum analyzer to measure the fundamental frequency.  
4. Make sure to set appropriate span and resolution bandwitdth (RBW) on the spectrum analyzer. Use a few hundred of MHz frequency range as span and a few kHz as RBW.  
5. Tune the CTune value until the frequency of the fundamental harmonic reach the optimal value. 
6. Use RailTest tool to tune CTune. The RailTest commands are listed below.



### RailTest command 
-----

> rx 0  
> setPower [decidBm]  
> setdebugmode 1  
> freqoverride 868000000  
> getCTune  
> setCTune <0x[hex-value] or [desimal value]>  
> setTxtone 1  
> ...  
> setTxtone 0  
> setCTune <0x[hex-value] or [desimal value]>   
> setTxtone 1


- Railtest starts with rx 0 command. 
- The actual output power can be set by setPower command.
- The actual output frequency can be set by setdebugmode 1 and freqoverride  commands.
- The actual CTunevariable can be read by getCTune command. 
- Then **write over this value**, which can be done by **setCTune** command. 
- After the configuration the setTxTone 1 command sets the radio to transmit mode. In this case the radio transmits CW signal. 
- The transmission can be disabled by setTXtone 0 command. 


 

More details are discussed in this [KBA.](https://www.silabs.com/community/wireless/proprietary/knowledge-base.entry.html/2019/03/18/hfxo_capacitor_bank-7uRt)



# The most important railtest commands

> setpower  

The setpower command can be used to set the current transmit power given in deci dBm.

>setTxTone

To transmit a CW signal on the current channel use the setTxTone command.  

> setDebugMode 1  
> freqOverride 868000000 

To modify the frequency it is needed to set the application into the FREQUENCY_OVERRIDE debug mode via setDebugMode.  In the FREQUENCY_OVERRIDE debug mode, the freqOverride command can be used to switch to another center frequency given in Hz.

> setCtune  

The setCtune command can be used to configure custom PA capacitor tuning values for TX and RX. 


For firther information, see this [KBA](https://www.silabs.com/community/wireless/proprietary/forum.topic.html/railtest_-_more_info-66AE).
