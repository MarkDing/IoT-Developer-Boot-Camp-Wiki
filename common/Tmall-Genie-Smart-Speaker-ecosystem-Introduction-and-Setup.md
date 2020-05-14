# Introduction

The Tmall Genie is a smart speaker developed by Chinese e-commerce company Alibaba Group, it was fist released in July 2017, by using the intelligent personal assistant service [AliGenie](https://en.wikipedia.org/wiki/AliGenie), it supports voice interaction for web searches, music streaming, control home automation devices, etc. Since Tmall Genie's first launch in 2017, it leads the smart speaker market in China and has about 40% smart speaker market share in the recent two years. And the built in AliGenie is a open-platform intelligent assistant, any developer can access the service to control their own products via voice command. The official statistics from Alibaba shows up to 700 IoT product manufacturer are involved in the ecosystem, and more and more manufacturer are integrating their products into the ecosystem.

Silicon Labs is the leader in silicon, software, and solutions for a smarter, more connected world. And there are lots of our customer are focusing on the smart home marketing, and if we can provide some example projects to extend the capability of our wireless solutions to support Tmall Genie smart speaker, the customers will benefit from the examples for getting to market much faster.  

<div align="center">
<img src="files/CM-Smart-Speaker/Tmall BlockDigram.png">
</div>

# Project Description

The [Aligenie](https://www.aligenie.com/)</span> platform provides different solutions that developer can adopt to access the service for controlling smart devices via voice command.

1.  Tmall Genie naturally plays role as BLE Mesh provisioner, it can form the network and directly add/remove device. The project [Tmall Genie control the Bluetooth Mesh Devices with the Built-in Hub](Tmall-Genie-control-the-Bluetooth-Mesh-Devices-with-the-Built-in-Hub) shows how to archived it.
2.  Alibaba provide an interface to send directive to 3rd party cloud, we can interpret the directive into the command. The project [Tmall Genie control the WIFI devices via cloud](Tmall-Genie-control-the-WIFI-devices-via-cloud) show how to build 3rd party cloud to interpret the directive from Alibaba cloud. So that we can achieve Tmall Genie to control a WiFi device via voice command.
3.  Alibaba provide a cloud service so the customer doesn't need to build their own. And it provide a Alink SDK for embedded device. The project [Tmall Genie control the Zigbee devices via cloud with Micrium OS](Tmall-Genie-control-the-Zigbee-devices-via-cloud-with-Micrium-OS) shows how to porting the Alink SDK to EFR32MG12 under Micrium OS. The Tmall Genie controls the smart device via Alibaba cloud to cloud service.
4.  Alibaba has AliOS thing with Alink SDK included. Porting AliOS thing on 3rd party WiFi platform to achieve the goal that smart speaker controls the device via voice command. The project [Tmall Genie control the Zigbee devices via cloud with AliOS](Tmall-Genie-control-the-Zigbee-devices-via-cloud-with-AliOS) shows step by step on how to do that.