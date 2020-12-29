<details>
<summary><font size=5>Table of Contents</font> </summary>  
</br>

- [1. Overview](#1-overview)
- [2. Broadcast Address](#2-broadcast-address)
- [3. Broadcast Table](#3-broadcast-table)
  - [3.1. nwkNetworkBroadcastDeliveryTime](#31-nwknetworkbroadcastdeliverytime)
  - [3.2. Passive Acknowledgement](#32-passive-acknowledgement)
- [4. Reference](#4-reference)

</details>

********

# 1. Overview
Broadcast is a common communication approach in Zigbee network. Zigbee broadcast happens in network layer. Zigbee also defines multicast in network layer, but it's just for network data. No Zigbee network command is using multicast. For the multicast of network data, it's actually an application layer multicast. In network layer, it's implemented with broadcast.

In this page, we will introduce a little bit about network broadcast.

# 2. Broadcast Address
In Zigbee spec, the broadcast address is defined as below:
|Broadcast Address|Destination Group|
|:-|:-|
|0xFFFF|All devices in PAN|
|0xFFFE|Reserved|
|0xFFFD|macRxOnWhenIdle=True|
|0xFFFC|All routers and coordinator|
|0xFFFB|Low power routers only|
|0xFFF8 ~ 0xFFFA|Reserved|
When a broadcast is sent, the network destination address will be filled with one of them. When a broadcast is delievered to the MAC layer, the MAC will fill the destination MAC address according to the device type:
- For routers and coordinator, the destination MAC address will be filled with 0xFFFF.
- For end devices, the destination MAC address will be filled with the node ID of the parent.

# 3. Broadcast Table
Broadcast doesn't use the MAC layer acknowledgement. Instead, a passive acknowledgement mechanism is used. Passive acknowledgement means that every router and coordiantor needs to keep track of which of its neighbor has already relayed the broadcast successfully. To achieve that, a broadcast table is implemented on every router and coordinator.

In Zigbee spec, broadcast table is called BTT, short for Broadcast Transaction Table. The entry is called BTR, short for Broadcast Transaction Record. The BTR should contain the following info:
|Field Name|Size|Description|
|:-|:-|:-|
|Source Address|2 Bytes|The 16-bit network address of the broadcast initiator|
|Sequence Number|1 Byte|The NWK layer sequence number of the initiator's broadcast|
|Expiration Time|1 Byte|A countdown timer indicating the number of seconds until this entry expires. The initial value is nwkNetworkBroadcastDeliveryTime.|

## 3.1. nwkNetworkBroadcastDeliveryTime
This time is the assumed maximum time that a broadcast may stay over the air. In the spec, there is a small paramgraph describing the reason why we need this:
```
Processing of a broadcast with a NWK source of the local device shall only be done when the device has been powered up and operating on the network for nwkNetworkBroadcastDeliveryTime. This prevents broadcasts from being
processed that might have recently originated from the device after a reset.
```
For example, if a device detected an address conflict and then it would change its node ID and then sent out a broadcast to notify the other devices in the network. If the device reset after the broadcast is sent out, it might be able to received the broadcast back which is relayed by other routers. At this time, the broadcast table is cleared as the device is resetted, the device will handle this broadcast and change the node ID again. Now with this setting added in the spec, devices will need to wait for at least nwkNetworkBroadcastDeliveryTime before it's allowed to handle the broadcast message with the network source of the local device.

In EmberZnet, the nwkNetworkBroadcastDeliveryTime is set to 3 seconds.

## 3.2. Passive Acknowledgement
Both the originating device and the relaying device need to track the broadcast which are relayed by their neighbors. Only if the device has received the relayed broadcast message from its all neighbors, the broadcast would be considered as transmitted successful. If not, the device might start a retry.

In EmberZnet, the implementation is kind of special. For the originating device, it doesn't check the passive acknowledgement. The outgoing message is retried for exact 2 times. This means if you send a broadcast, you would see 3 broadcasts over the air.

For the relaying nodes, they just check if the received broadcast exists in the broadcast table. If not, record it and they relay it. Otherwise, check if the broadcast table entry is already passively acknowledged (has received the relayed back message from all neighbors). If not, relay it. Otherwise, drop it.

By default, the relaying nodes need to get the passive acknowledgement from all their neighbors before the broadcast is considered acknowledged. This could possibly cause a same broadcast message be relayed more than once in a big network. EmberZnet has provided an API with which you can set the minimal passive acknowledgements required from the neighbors.
```C
/** @brief Sets the number of broadcast passive acknowledgements required before
 * terminating a broadcast transmission. A value of 0xFF causes the node to wait
 * for all neighbors to re-broadcast the packet before terminating the
 * transmission. The default value is 0xFF.
 *
 * @param minAcksNeeded  The minimum number of acknowledgments (re-broadcasts)
 * to wait for until deeming the broadcast transmission complete.
 *
 * @return None.
 */
void emberBroadcastSetMinAcksNeeded(uint8_t minAcksNeeded);

```

# 4. Reference
- [Zigbee Spec](https://zigbeealliance.org/wp-content/uploads/2019/11/docs-05-3474-21-0csg-zigbee-specification.pdf)





