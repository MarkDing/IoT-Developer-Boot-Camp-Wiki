# Table of Content
- [1. Introduction](#1-introduction)
    - [1.1. Application features](#11-application-features)
    - [1.2. Purpose](#12-purpose)
- [2. Using Event](#2-using-event)
- [3. Testing your project](#3-testing-your-project)
- [4. Conclusion](#4-conclusion)

# 1. Introduction

## 1.1. Application features
The boot camp series hands-on workshop will cover four functionalities below, and the application development is split into four steps respectively to show how an application should be built up from the beginning.  
The exercise in this documentation is the 3rd exercise in the “Zigbee Boot Camp” series.  
-   In the 1st phase, a basic network forming by the Light, and a joining process by the Switch will be realized.  
-   The 2nd part will prepare the devices to transmit, receive, and process the On-Off commands by using APIs.  
-   **At the 3rd step the Switch will have a periodic event to execute any custom code, which will be a LED blinking in our case.**  
-   The 4th thing to do is to make the Switch to be able to store any custom data in its flash by using Non-volatile memory.  

## 1.2. Purpose
In the previous hand-on “Forming and Joining” and “Sending OnOff Command”, we learned how to form a basic centralized Zigbee network and join the network, and how to send onoff command from the Switch node to the Light node in the Zigbee mesh network.  
In this hands-on, we provide step-by-step instructions to demonstrate how to use the Zigbee Stack event mechanism to schedule events on the Switch node.  
The figure below illustrates the working flow of this hands-on.  

<div align="center">
  <img src="https://github.com/MarkDing/IoT-Developer-Boot-Camp-Wiki/blob/master/zigbee/images/using_event_working_flow.png">  
</div>  
</br>  

**Note**:
Before all the individual steps would be performed, please make sure that both the hardware and software are ready for the development. Read the chapter “2 Fundamental steps” of the previous 2 hands-on for more detail about it.  

# 2. Using Event
EmberZNet Stack has Event control mechanism that basically allows application to run a piece of code at desired time interval.  

An event should be initialized somewhere in the code, hence a function should be used which is called at the beginning of the application.  

The *Main Init* callback is called from the application’s main function. It gives the application a chance to do any initialization required at system startup. It can be imagined like a function at the top of the *“main()”* before the classical “*while(true)*”.  

**Step 1**:
Double click the Zigbee_Switch_ZR.isc file to open it with the AppBuilder, and then enable this callback in the AppBuilder’s Callbacks tab. See the figure below.  

<div align="center">
  <img src="https://github.com/MarkDing/IoT-Developer-Boot-Camp-Wiki/blob/master/zigbee/images/main_init_enableing.png">  
</div>  
<div align="center">
  <b>Main Init callback enabling</b>
</div>  
</br>  

**Step 2**:
The AppBuilder provides manner to add any custom event to the application.  
Basically, two things need for this.  
-   Event Controller – structure of the task  
-   Event Handler – function on the task  

Open the *AppBuilder* -> *Includes* tab. Add the custom event command ```ledBlinkingEventControl``` and callback ```ledBlinkingEventHandler``` to the *Event Configuration* window respectively. See figure below.  
<div align="center">
  <img src="https://github.com/MarkDing/IoT-Developer-Boot-Camp-Wiki/blob/master/zigbee/images/custom_event_adding_in_AppBuilder.png">  
</div>  
<div align="center">
  <b>Custom event adding in AppBuilder</b>
</div>  
</br>  

Save and Generate the project.  

**Step 3**:
As earlier, the callback function ```emberAfMainInitCallback()``` should be added to the *Zigbee_Switch_ZR_callbacks.c* file and initialize the event.  
The related code snippet should be like the followings:  

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

  //Reschedule the event after a delay of 1 seconds
  emberEventControlSetDelayMS(ledBlinkingEventControl, 2000);
}
```

It’s worth to mention that the event should be set to inactive right after its function starts to be executed and re-schedule after it’s done.  

# 3. Testing your project
Build the applications and download the image to the Switch devices. Press the Reset button on the starter kit, you will notice that the LED1 on the board will be turned on after few seconds delay, and then blink with 2s interval.  

# 4. Conclusion
In this hands-on, you learned how to create a custom event, define the event function and event control structure, and implement the event function for scheduling the LED blinking event.  
