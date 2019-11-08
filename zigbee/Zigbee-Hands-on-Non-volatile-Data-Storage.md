
# Table of Content

- [1. Introduction](#1-introduction)
    - [What is Non-Volatile Memory?](#what-is-non-volatile-memory)
    - [Why need Non-Volatile Storage in EmberZNet PRO?](#why-need-non-volatile-storage-in-emberznet-pro)
    - [How does Silicon Labs implement the Non-Volatile Data Storage?](#how-does-silicon-labs-implement-the-non-volatile-data-storage)
- [2. Access NVM3 objects with Token API](#2-access-nvm3-objects-with-token-api)
    - [Types of Tokens: Manufacturing Tokens and Dynamic Tokens](#types-of-tokens-manufacturing-tokens-and-dynamic-tokens)
        - [Manufacturing Tokens](#manufacturing-tokens)
        - [Dynamic Tokens](#dynamic-tokens)
            - [Basic (Non-indexed) Tokens](#basic-non-indexed-tokens)
            - [Indexed Tokens](#indexed-tokens)
    - [Types of Tokens: Default Tokens and Custom Tokens](#types-of-tokens-default-tokens-and-custom-tokens)
        - [Default Tokens](#default-tokens)
        - [Custom Tokens](#custom-tokens)
    - [Creating and Accessing Tokens](#creating-and-accessing-tokens)
    - [Token Header Files](#token-header-files)
    - [Creating Dynamic Tokens](#creating-dynamic-tokens)
        - [Define the Token Name](#define-the-token-name)
        - [Define the Token Type](#define-the-token-type)
        - [Define the Token Storage](#define-the-token-storage)
    - [Accessing Dynamic Tokens](#accessing-dynamic-tokens)
        - [Accessing Basic (Non-indexed) Tokens](#accessing-basic-non-indexed-tokens)
        - [Accessing Indexed Tokens](#accessing-indexed-tokens)
    - [Manufacturing Tokens](#manufacturing-tokens-1)
        - [Accessing Manufacturing Tokens](#accessing-manufacturing-tokens)
        - [Where to Find Default Token Definitions](#where-to-find-default-token-definitions)
- [3. Lab](#3-lab)
    - [Preparing the WSTK](#preparing-the-wstk)
    - [Get the basic Zigbee projects](#get-the-basic-zigbee-projects)
        - [Import the projects into Simplicity Studio](#import-the-projects-into-simplicity-studio)
        - [Create Custom Tokens](#create-custom-tokens)
        - [Access the basic Token LED1_ON_OFF](#access-the-basic-token-led1_on_off)
            - [Step 1: Retrieve the basic Token data](#step-1-retrieve-the-basic-token-data)
            - [Step 2: Write the basic Token data](#step-2-write-the-basic-token-data)
            - [Step 3: Testing your project](#step-3-testing-your-project)
        - [Access the manufacturing Token](#access-the-manufacturing-token)
            - [Step 4: Read the manufacturing Token MFG_STRING](#step-4-read-the-manufacturing-token-mfg_string)
- [4. Conclusion](#4-conclusion)

# Non-Volatile Data Storage in EmberZNet PRO
This training demonstrates the basic usage of Non-Volatile storage on EmberZNet Stack. And also some of the basic knowledge are included in this documentation to help everyone to associate the knowledge and the practice.

# 1. Introduction
## What is Non-Volatile Memory?
Non-Volatile Memory (NVM) or Non-Volatile Storage is memory that can retrieve stored information even when the device is power-cycled. It typically refers to storage in semiconductor memory chips, including flash memory storage such as NAND flash and solid-state drives (SSD), and ROM chips such as EPROM (erasable programmable ROM) and EEPROM (electrically erasable programmable ROM).
On Silicon Labs microcontrollers and radio SoCs, it does not offer internal EEPROM, the NVM is implemented as flash memory.  

## Why need Non-Volatile Storage in EmberZNet PRO?
Usually, the EmberZNet stack and application need to store some data objects which should remain after power cycle. Some data is considered manufacturing data that is written only once at manufacturing time, on the other side, some data are written and read frequently over the life of the product which is referred to as dynamic data.  

## How does Silicon Labs implement the Non-Volatile Data Storage?
Totally, Silicon Labs offers 3 different implementations for Non-Volatile data storage in **flash** memory. And also offer the Token mechanism for storing and retrieving data from the Non-Volatile Data Storage.  

**Persistent Store (PS Store)**  
PS Store is only used with Bluetooth devices on all platforms except for EFR32 Series 2. The persistent store size is 2048 bytes and uses two flash pages for storage. Both the Bluetooth stack and application can store data in this area.  
Since this documentation focus on EmberZNet PRO, we will not introduce it much in this document.  

**SimEEv1 and SimEEv2**  
SimEEv1(Simulated EEPROM version 1) or SimEEv2(Simulated EEPROM version 2) are used with EmberZNet PRO, Silicon Labs Thread, Silicon Labs Connect on EM35x and EFR32 Series 1 platforms. SimEEv1 uses two virtual pages, with each virtual page consisting of two flash pages, while SimEEv2 uses three virtual pages where each virtual page consists of 6 flash pages.  

**NVM3**  
The third generation Non-Volatile Memory (NVM3) data storage driver is an alternative to SimEEv1/v2 and PS Store, it is designed to work in EmberZNet, Silicon Labs Thread, Connect, and Bluetooth applications running on EFR32 as well as MCU applications running on EFM32.
Since the NVM3 is more configurable which allows for better balance of token capacity versus reserved flash, and it's compatible with DMP application, it's recommended for developing on EFR32.  

**Token**  
Tokens let the application store defined data types in Non-Volatile Storage, and SimEEv1/v2 and NVM3 are designed to operate below the token system. 
The diagram below illustrates the relationship between the Tokens and the Non-volatile Data Storage mechanisms. Silicon Labs offers three different dynamic token implementations: Simulated EEPROM Version 1 (SimEEv1), Simulated EEPROM Version 2 (SimEEv2), and Third Generation Non-Volatile Memory (NVM3).  

<div align="center">
  <img src="https://github.com/MarkDing/IoT-Developer-Boot-Camp-Wiki/blob/master/zigbee/images/Non-volatile_Data_Storage_and_Tokens.png">  
</div>  
<div align="center">
  <b>Non-Volatile Storage and Tokens</b>
</div>  

***

# 2. Access NVM3 objects with Token API
As NVM3 is the recommended implementation for Non-Volatile data storage, the section below introduce how to access the NVM3 objects with Token API.

## Types of Tokens: Manufacturing Tokens and Dynamic Tokens
Depending on how the tokens are going to be used, it can be distinguished as Manufacturing Tokens or Dynamic Tokens. 

<div align="center">
  <img src="https://github.com/MarkDing/IoT-Developer-Boot-Camp-Wiki/blob/master/zigbee/images/types_of_tokens_1.png">
</div>
</br>

### Manufacturing Tokens
Manufacturing tokens are written either only once or very infrequently during the lifetime of the chip, and they are stored at absolute addresses of the flash.

### Dynamic Tokens
Dynamic tokens can be accessed (both read and written) frequently. They are stored in a dedicated area of the flash where we use a memory-rotation algorithm to prevent flash overuse.
There are two types of dynamic tokens, with the types distinguished by their format.

#### Basic (Non-indexed) Tokens
These can be thought of as a simple char variable type. They can be used to store an array, but if one element changes the entire array must be rewritten.
A counter token is a special type of non-indexed dynamic token meant to store a number that increments by 1 at a time.

#### Indexed Tokens
Indexed dynamic tokens can be considered as a linked array of char variables where each element is expected to change independently of the others and therefore is stored internally as an independent token and accessed explicitly through the token API

## Types of Tokens: Default Tokens and Custom Tokens
Depending on whether a token is provided by Silicon Labs as part of a networking protocol stack, or created by a user, we can also categorize tokens as default tokens or custom tokens.

<div align="center">
  <img src="https://github.com/MarkDing/IoT-Developer-Boot-Camp-Wiki/blob/master/zigbee/images/types_of_tokens_2.png">
</div>
</br>

### Default Tokens
The networking stack contains some default tokens. These tokens are grouped by their software purpose.  
   * Manufacturing Tokens are set at manufacturing time and cannot be changed by the application.
   * Stack Tokens are runtime configuration options set by the stack; these dynamic tokens should not be changed by the application.
   * Application Framework Tokens are application tokens used by the Application Framework and generated by AppBuilder; these dynamic tokens should not be changed by the application after project generation. Examples of these are ZCL attribute tokens and plugin tokens.

### Custom Tokens
In addition to default tokens, users can add these types of tokens specific to their application.  
   * Custom Manufacturing Tokens are defined by the user and set at manufacturing time.
   * Custom Application Tokens are additional application tokens users may add to meet their unique application needs for non-volatile data storage.

## Creating and Accessing Tokens
Now that we have talked about how to use tokens. This includes knowing how to create new tokens, and how to read and potentially modify tokens, where to find default tokens.

## Token Header Files
A token header file is simply a .h file that contains token definitions. Manufacturing tokens and dynamic tokens have separate token header files. 
In this hands-on, we will create a header file  ```custom-token.h``` contains the custom dynamic token definitions.

## Creating Dynamic Tokens
Adding a dynamic token to the header file involves three steps:
* Define the token name.
* Add any typedef needed for the token, if it is using an application-defined type.
* Define the token storage.

### Define the Token Name
When defining the name, do not prepend the word TOKEN. For NVM3 dynamic tokens, use the word NVM3KEY as the prefix.
```
/**
* Custom Zigbee Application Tokens
*/
// Define token names here
#define NVM3KEY_LED1_ON_OFF			(NVM3KEY_DOMAIN_USER | 0x0001)
```
Please note that the token key values must be unique within this device.
For NVM3, custom application tokens should use the **NVM3KEY_DOMAIN_USER** range so as not to collide with the stack tokens in other ranges such as **NVM3KEY_DOMAIN_ZIGBEE**. See the table below for the NVM3 default instance key space.  

<div align="center">
  <img src="https://github.com/MarkDing/IoT-Developer-Boot-Camp-Wiki/blob/master/zigbee/images/NVM3_default_instance_key_space.png">
</div>  
</br>  

### Define the Token Type
The token type can be either a built-in C data type, or defined as a custom data structure using typedef. 
```
#if defined(DEFINETYPES)
// Include or define any typedef for tokens here
typedef struct {
  uint8_t ledIndex; 		// LED index
  bool ledOnOff;			// LED ON OFF status
} ledOnOffStatus_t;
#endif //DEFINETYPES
```

### Define the Token Storage
After any custom types are defined, the token storage is defined. This informs the token management software about the tokens being defined.
Each token, whether custom or default, gets its own entry in this part:

```
#ifdef DEFINETOKENS
// Define the actual token storage information here
DEFINE_BASIC_TOKEN(LED1_ON_OFF,
                  ledOnOffStatus_t,
                  {1, false})
#endif
```

DEFINE_BASIC_TOKEN takes three arguments: the name (LED1_ON_OFF), the data type (ledOnOffStatus_t) what we defined above, and the default value of the token if it has never been written by the application ({1, false}).

In this case, the first value (ledIndex) is initialized as ```1``` to indicates the LED1, and the next value (ledOnOff) is set to ```fales``` to represent the default status of the LED1.

## Accessing Dynamic Tokens
The networking stack provides a simple set of APIs for accessing token data. The APIs differ slightly depending on the type of the tokens,

### Accessing Basic (Non-indexed) Tokens
The non-indexed/basic token API functions include:  
```
void halCommonGetToken(data, token)  
void halCommonSetToken(token, data)  
```
In this case, 'token' is the token key, and 'data' is the token data. Note that ```halCommonGetToken()``` and ```halCommonSetToken()``` are general token APIs that can be used for both basic dynamic tokens, and manufacturing tokens.

Now let us use an example to explain the usage of these APIs. In this hands-on, the application needs to store the LED1's on/off status frequently, and restore the LED1 last on/off status after power up. As we have defined the token as above, then you can access it with a code snippet like this:

```
ledOnOffStatus_t led1OnOffStatus;

// Retrieve the previous status of LED1
halCommonGetToken(&led1OnOffStatus, TOKEN_LED1_ON_OFF);

led1OnOffStatus.ledOnOff = <current status>;

// Store the current status of LED1
halCommonSetToken(TOKEN_LED1_ON_OFF, &led1OnOffStatus);
```

Since this hands-on is designed for new to the Silicon Labs EmberZNet stack, we will focus on the basic token usage, if you are interested about how to write the counter token, please read the section [3.3.1.1 Accessing Counter Tokens](https://www.silabs.com/documents/public/application-notes/an1154-tokens-for-non-volatile-storage.pdf) of AN1154.

### Accessing Indexed Tokens
For accessing the Indexed Tokens, please use the APIs below. Similar as explained above, we will not spend much space of this documentation to introduce Indexed Tokens, please refer to the section [3.3.2 Accessing Indexed Tokens] of AN1154 for more information.
```
void halCommonGetIndexedToken(data, token, index)
void halCommonSetIndexedToken(token, index, data)
```

## Manufacturing Tokens
Manufacturing tokens are defined in the same way as basic (non-indexed) dynamic tokens. The major difference between them is that on where the tokens are stored and how they are accessed. 
Manufacturing tokens reside in the dedicated flash page for manufacturing tokens (with fixed absolute addresses).

### Accessing Manufacturing Tokens
as the name suggests, are usually written once at manufacturing time into fixed locations in a dedicated flash page. Since their addresses are fixed, they can be easily read from external programming tools if Read Protection for this flash area is disabled. 
And since the same flash cell cannot be written repeatedly without erase operations in between. Writing a manufacturing token from on-chip code works only if the token is currently in an erased state. Overwriting the manufacturing token that has been already written before always requires erasing the flash page for the manufacturing token with external programming tools.

Manufacturing tokens should be accessed with their own dedicated API below.
```
halCommonGetMfgToken(data, token);
halCommonSetMfgToken(token, data);
```
They have the same parameters as the basic tokens APIs. The two primary purposes for using the dedicated manufacturing token access APIs are:
* For slightly faster access;
* For access early in the boot process before emberInit() is called.
And the Manufacturing tokens can also be accessed through the basic token APIs ```halCommonGetToken()``` and ```halCommonSetToken()```.

Also let us use an example to explain the usage of these dedicated APIs for accessing manufacturing tokens. Generally, manufacturer will program the manufacturing sting token during the product manufacturing with Simplicity Commander, and we can use the on-chip code snippet below to retrieve the data from the manufacturing token.

```
tokTypeMfgString mfgString;
// Retrive the manufacturing string from the manufacturing token
halCommonGetMfgToken(mfgString, TOKEN_MFG_STRING);
```

### Where to Find Default Token Definitions
To view the stack tokens, refer to the file:  
```<install-dir>/stack/config/token-stack.h```

To view the Application Framework tokens, please navigate to the project directory after the project has been generated in AppBuilder. The files ```<project_name>_tokens.h``` which contains the tokens for ZCL attributes, and the protocol-specific token file ```znet-token.h``` which includes plugin token headers and the custom application token header.

To view the manufacturing tokens for the EFR32 series of chips, refer to the following files:
```<install-dir>/hal/micro/cortexm3/efm32/token-manufacturing.h```

***

# 3. Lab
This section provides step-by-step instructions to demonstrate how to store and retrieve the LED1's status to/from the Non-Volatile data storage (it's NVM3 in this hands-on) objects with basic token. And also demonstrate how to access manufacturing token with the dedicated APIs.

**Prerequisites**
Please make sure that you have finished the [preparatory course]() and make sure all of the SDKs software and Starter Kits are ready.

## Preparing the WSTK
This hands-on requires either EFR32MG21/EFR32MG13/EFR32MG12 radio board, and EFR32MG12 radio board BRD4162A is recommended since we created the example project with this kit. Below is the layout of the starter kit.
<div align="center">
  <img src="https://github.com/MarkDing/IoT-Developer-Boot-Camp-Wiki/blob/master/zigbee/images/brd4162_kit.png">
</div>  
</br>  

Connect the starter kit to PC using the "J-Link USB" connector and the cable provided with the starter kit. And turn the power switch to "AEM" position. Start the Simplicity Studio V4, and it should able to find the attached device, and list it in the Debug Adapter area of the launcher console.
<div align="center">
  <img src="https://github.com/MarkDing/IoT-Developer-Boot-Camp-Wiki/blob/master/zigbee/images/detect_wstk.png">
</div>
</br>

## Get the basic Zigbee projects
Now you should have finished the previous 3 hands-on, we will continue this hands-on based on the result of those. 
If you haven't get the source code, please read the [How to Get Code ?](https://github.com/MarkDing/IoT-Developer-Boot-Camp/blob/master/CONTRIBUTING.md#how-to-get-code-) for getting the primary source code of these hands-on.

### Import the projects into Simplicity Studio
We have provided two projects ```Zigbee_Light_ZC``` and ```Zigbee_Switch_ZR``` as the starting projects for the series of hands-on. Since the Non-volatile data storage mechanism does not depend on the mesh node type. We will only demonstrate how to access the NVM3 object via token API on the Zigbee router device side, it refers to the ```Zigbee_Switch_ZR``` project.
Import the ```Zigbee_Switch_ZR``` example project to your Simplicity Studio workspace.

<div align="center">
  <img src="https://github.com/MarkDing/IoT-Developer-Boot-Camp-Wiki/blob/master/zigbee/images/import_ssv4_project.gif">
</div>
</br>

### Create Custom Tokens
We are going to create a header file ```custom-token.h```, and define the token name, token type and token storage in this header file. Below is content of the header file we used in this hands-on.

```
// File: custom-token.h
//
// Description: Custom token definitions used by the application.
//
// Copyright 2019 by Silicon Labs Corporation.  All rights reserved.

/**
* Custom Zigbee Application Tokens
*/
// Define token names here
#define NVM3KEY_LED1_ON_OFF			(NVM3KEY_DOMAIN_USER | 0x0001)

#if defined(DEFINETYPES)
// Include or define any typedef for tokens here
typedef struct {
  uint8_t ledIndex; 		// LED index
  bool ledOnOff;			// LED ON OFF status
} ledOnOffStatus_t;
#endif //DEFINETYPES

#ifdef DEFINETOKENS
// Define the actual token storage information here
DEFINE_BASIC_TOKEN(LED1_ON_OFF,
                  ledOnOffStatus_t,
                  {1, false})
#endif
```
Firstly, we will define the Token name as ```NVM3KEY_LED1_ON_OFF```.

<font color=red><b>Question</b></font>: Why need to define the token name with the prepended word ```NVM3KEY``` ? Can I define it as ```TOKEN_LED1_ON_OFF```?
<font color=red><b>Hint</b></font>: Please back to the section [Define the Token Name](#define-the-token-name).

Then define the Token type for recording the LED On/Off status, define a structure type ```ledOnOffStatus_t``` which includes two different data type to represent the LED index and LED status.

After that, define the token storage with the macro ```DEFINE_BASIC_TOKEN```. 

After creating the custom token header file, you need one more step: add the header file to the application, through the Includes tab in the .isc file in Simplicity Studio, under the “Token Configuration” section. 

<div align="center">
  <img src="https://github.com/MarkDing/IoT-Developer-Boot-Camp-Wiki/blob/master/zigbee/images/add_custom_token_header_file.gif">
</div>
</br>

### Access the basic Token LED1_ON_OFF
Let's moving on for how to access the defined token. Below are step-by-step instructions for adding code to store the LED status to Non-volatile data storage, and retrieve the data for restoring the LED status. 
Each step for this lab will have an associated comment that starts off ```Non-volatile Data Storage: Step x```

#### Step 1: Retrieve the basic Token data
Open the ```Zigbee_Switch_ZR_callback.c``` and define a "ledOnOffStatus_t" type variable. 
```
ledOnOffStatus_t led1OnOffStatus;
```
Navigate to the function ```void emberAfMainInitCallback(void)``` of the ```Zigbee_Switch_ZR_callback.c``` which will be called from the application's main function during the initialization, and retrieve the basic token "LED1_ON_OFF" with the API ```halCommonGetToken()```.

```
// Retrieve the LED1 status before reset/power-off from the token
halCommonGetToken(&led1OnOffStatus, TOKEN_LED1_ON_OFF);
```
And then apply the retrieved status to the LED1 by using the API ```halSetLed()``` or ```halClearLed()```  

```
// Restore the LED1 status during initialization
if(led1OnOffStatus.ledOnOff){
  halSetLed(led1OnOffStatus.ledIndex);
}
else{
  halClearLed(led1OnOffStatus.ledIndex);
}
```

#### Step 2: Write the basic Token data
In the last hands-on, we defined a event handler ```ledBlinkingHandler()``` to toggle the LED1 periodically, we need to store the LED1 status after each toggling process.
Navigate to the function ```void ledBlinkingHandler(void)``` in the ```Zigbee_Switch_ZR_callback.c```. Also you can address the function with the comment ```Non-volatile Data Storage: Step 2```.
Write the basic token "LED1_ON_OFF" with the API ```halCommonSetToken()```. Please not that the LED1 toggle process of last hands-on is surrounded by the token retrieving and storing process.
```
halCommonGetToken(&led1OnOffStatus, TOKEN_LED1_ON_OFF);
halToggleLed(led1OnOffStatus.ledIndex);
led1OnOffStatus.ledOnOff = !led1OnOffStatus.ledOnOff;
// Store the current status of LED1
halCommonSetToken(TOKEN_LED1_ON_OFF, &led1OnOffStatus);
```

#### Step 3: Testing your project
Once you've add the necessary code to you project, compile and flash the ```Zigbee_Switch_ZR``` project to your BRD4162A radio board. The LED1 on the starter kit will blinky periodically after few seconds delay after power up, reset the device, the application will restore the LED1 to the status before reset/power-off.

### Access the manufacturing Token
#### Step 4: Read the manufacturing Token MFG_STRING
Manufacturing token can be written from on-chip code only if the token is currently in an erased state. Generally, the manufacturer will write the manufacturing token with external programming tools, such as Simplicity Commander.
This part will involve reading the manufacturing Token MFG_STRING which hold the manufacturing string programmed by the manufacture during production.
Navigate to the function ```void emberAfMainInitCallback(void)``` of the ```Zigbee_Switch_ZR_callback.c```, and read the manufacturing Token MFG_STRING with the API ```halCommonGetMfgToken```.
```
tokTypeMfgString mfgString;
halCommonGetMfgToken(mfgString, TOKEN_MFG_STRING);
```

<font color=red><b>Question</b></font>: Can the Manufacturing tokens be accessed through the basic token APIs?  
<font color=red><b>Hint</b></font>: Please back to the section [Accessing Manufacturing Tokens](#accessing-manufacturing-tokens).  

# 4. Conclusion  
We hope you enjoyed the Non-volatile data storage Lab, and understood the implementation provided by Silicon Labs, they are [NVM3, SimEEv1/SimEEv2 and PS Store](#how-does-silicon-labs-implement-the-non-volatile-data-storage).
Also, you should have learned how to create and access the basic token, as well as how to access the manufacturing token.

For more information on Non-volatile data storage and Tokens, please refer to the following documentations.

[UG103.7: Non-Volatile Data Storage Fundamentals](https://www.silabs.com/documents/public/user-guides/ug103-07-non-volatile-data-storage-fundamentals.pdf)  
[AN1154: Using Tokens for Non-Volatile Data Storage](https://www.silabs.com/documents/public/application-notes/an1154-tokens-for-non-volatile-storage.pdf)  
[AN1135: Using Third Generation NonVolatile Memory (NVM3) Data Storage](https://www.silabs.com/documents/public/application-notes/an1135-using-third-generation-nonvolatile-memory.pdf)  
