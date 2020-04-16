/***************************************************************************//**
 * @brief Sensor sample application.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER

#include "stack/include/ember.h"
#include "hal/hal.h"
#ifndef UNIX_HOST
#include "heartbeat/heartbeat.h"
#include "wstk-sensors/wstk-sensors.h"
#endif
#include "poll/poll.h"
#include "command-interpreter/command-interpreter2.h"
#include "debug-print/debug-print.h"

#define SENSOR_SINK_TX_POWER    0
#define SENSOR_SINK_PAN_ID      0x01FF
#define SENSOR_SINK_PROTOCOL_ID 0xC00F

#define SENSOR_SINK_PROTOCOL_ID_OFFSET  0
#define SENSOR_SINK_COMMAND_ID_OFFSET   2
#define SENSOR_SINK_EUI64_OFFSET        3
#define SENSOR_SINK_NODE_ID_OFFSET      11
#define SENSOR_SINK_DATA_OFFSET         13
#define SENSOR_SINK_MAXIMUM_DATA_LENGTH 8
#define SENSOR_SINK_MINIMUM_LENGTH      SENSOR_SINK_DATA_OFFSET
#define SENSOR_SINK_MAXIMUM_LENGTH      (SENSOR_SINK_MINIMUM_LENGTH \
                                         + SENSOR_SINK_MAXIMUM_DATA_LENGTH)

#define SINK_ADVERTISEMENT_PERIOD_MS (60 * MILLISECOND_TICKS_PER_SECOND)
#define SENSOR_TIMEOUT_MS            (60 * MILLISECOND_TICKS_PER_SECOND)
#define SENSOR_PAIR_TIMEOUT_MS       (5 * MILLISECOND_TICKS_PER_SECOND)

#define NETWORK_UP_LED               BOARDLED0

#define SENSOR_SINK_SECURITY_KEY    { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, \
                                      0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, \
                                      0xAA, 0xAA, 0xAA, 0xAA }

enum {
  SENSOR_SINK_COMMAND_ID_ADVERTISE_REQUEST = 0,
  SENSOR_SINK_COMMAND_ID_ADVERTISE         = 1,
  SENSOR_SINK_COMMAND_ID_PAIR_REQUEST      = 2,
  SENSOR_SINK_COMMAND_ID_PAIR_CONFIRM      = 3,
  SENSOR_SINK_COMMAND_ID_DATA              = 4,
};
typedef uint8_t SensorSinkCommandId;

static int8_t txPower = SENSOR_SINK_TX_POWER;
static EmberNodeId sinkNodeId = EMBER_NULL_NODE_ID;
static bool confirmed;
static EmberKeyData securityKey = SENSOR_SINK_SECURITY_KEY;

static EmberStatus send(EmberNodeId nodeId,
                        SensorSinkCommandId commandId,
                        uint8_t *buffer,
                        uint8_t bufferLength);
static EmberStatus requestAdvertise(void);

static uint8_t message[SENSOR_SINK_MAXIMUM_LENGTH];
static EmberMessageLength messageLength;
static EmberMessageOptions txOptions = EMBER_OPTIONS_ACK_REQUESTED;

static uint16_t sensorReportPeriodMs =  (1 * MILLISECOND_TICKS_PER_SECOND);

EmberEventControl reportControl;

//------------------------------------------------------------------------------
// Callbacks

void reportHandler(void)
{
  if (!emberStackIsUp() || sinkNodeId == EMBER_NULL_NODE_ID || !confirmed) {
    emberEventControlSetInactive(reportControl);
    emberAfPluginPollEnableShortPolling(FALSE);
  } else {
    EmberStatus status, sensorStatus;
    uint8_t buffer[SENSOR_SINK_MAXIMUM_DATA_LENGTH];
    int32_t tempData = 0;
    uint32_t rhData = 0;
    uint8_t i;

    // Sample temperature and humidity from sensors.
    // Temperature is sampled in "millicelsius".
    #ifndef UNIX_HOST
    sensorStatus = emberAfPluginWstkSensorsGetSample(&rhData, &tempData);
    #else
    sensorStatus = EMBER_SUCCESS;
    #endif

    if (sensorStatus == EMBER_SUCCESS) {
      emberStoreLowHighInt32u(buffer, tempData);
      emberStoreLowHighInt32u(buffer + 4, rhData);

      status = send(sinkNodeId,
                    SENSOR_SINK_COMMAND_ID_DATA,
                    buffer,
                    SENSOR_SINK_MAXIMUM_DATA_LENGTH);
      emberAfCorePrint("TX: Data to 0x%2x:", sinkNodeId);
      for (i = 0; i < SENSOR_SINK_MAXIMUM_DATA_LENGTH; i++) {
        emberAfCorePrint(" %x", buffer[i]);
      }
      emberAfCorePrintln(": 0x%x", status);
      emberEventControlSetDelayMS(reportControl, sensorReportPeriodMs);
    }
  }
}

void emberAfIncomingMessageCallback(EmberIncomingMessage *message)
{
  if (message->length < SENSOR_SINK_MINIMUM_LENGTH
      || (emberFetchLowHighInt16u(message->payload + SENSOR_SINK_PROTOCOL_ID_OFFSET)
          != SENSOR_SINK_PROTOCOL_ID)) {
    return;
  }

  switch (message->payload[SENSOR_SINK_COMMAND_ID_OFFSET]) {
    case SENSOR_SINK_COMMAND_ID_ADVERTISE_REQUEST:
      emberAfCorePrintln("RX: Advertise Request from 0x%2x", message->source);
      break;
    case SENSOR_SINK_COMMAND_ID_ADVERTISE:
    {
      emberAfCorePrintln("RX: Advertise from 0x%2x", message->source);
      if (sinkNodeId == EMBER_NULL_NODE_ID || !confirmed) {
        EmberStatus status = send(EMBER_COORDINATOR_ADDRESS,
                                  SENSOR_SINK_COMMAND_ID_PAIR_REQUEST,
                                  NULL,
                                  0);
        emberAfCorePrintln("TX: Pair Request to 0x%2x: 0x%x",
                           EMBER_COORDINATOR_ADDRESS,
                           status);
        if (status == EMBER_SUCCESS) {
          sinkNodeId = EMBER_COORDINATOR_ADDRESS;
          confirmed = FALSE;

          // If the node is sleepy switch to short poll mode to get the confirm
          // command.
          if (emberGetNodeType() == EMBER_STAR_SLEEPY_END_DEVICE) {
            emberAfPluginPollEnableShortPolling(TRUE);
          }
        }
      } else {
        emberAfPluginPollEnableShortPolling(FALSE);
      }
      break;
    }
    case SENSOR_SINK_COMMAND_ID_PAIR_REQUEST:
      emberAfCorePrintln("RX: Pair Request from 0x%2x", message->source);
      break;
    case SENSOR_SINK_COMMAND_ID_PAIR_CONFIRM:
    {
      emberAfCorePrintln("RX: Pair Confirm from 0x%2x", message->source);
      if (message->source == sinkNodeId) {
        confirmed = TRUE;
        reportHandler();
        emberAfPluginPollEnableShortPolling(FALSE);
      }
      break;
    }
    case SENSOR_SINK_COMMAND_ID_DATA:
    {
      uint8_t i;
      emberAfCorePrint("RX: Data from 0x%2x:", message->source);
      for (i = SENSOR_SINK_DATA_OFFSET; i < message->length; i++) {
        emberAfCorePrint(" %x", message->payload[i]);
      }
      emberAfCorePrintln("");
      break;
    }
    default:
      emberAfCorePrintln("RX: Unknown from 0x%2x", message->source);
      break;
  }
}

#define MAX_TX_FAILURES     10
static uint8_t txFailureCount = 0;

void emberAfMessageSentCallback(EmberStatus status,
                                EmberOutgoingMessage *message)
{
  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("TX: 0x%x", status);
    txFailureCount++;
    if (SENSOR_SINK_MINIMUM_LENGTH <= message->length
        && (emberFetchLowHighInt16u(message->payload + SENSOR_SINK_PROTOCOL_ID_OFFSET)
            == SENSOR_SINK_PROTOCOL_ID
            && (message->payload[SENSOR_SINK_COMMAND_ID_OFFSET]
                == SENSOR_SINK_COMMAND_ID_DATA))
        && txFailureCount >= MAX_TX_FAILURES) {
      emberAfCorePrintln("EVENT: dropped sink 0x%2x", sinkNodeId);
      sinkNodeId = EMBER_NULL_NODE_ID;
    }
  } else {
    // Success: reset the failures count.
    txFailureCount = 0;
  }
}

void emberAfStackStatusCallback(EmberStatus status)
{
  switch (status) {
    case EMBER_NETWORK_UP:
      emberAfCorePrintln("Network up");

      // Unicast an advertise request to the sink.
      requestAdvertise();
      break;
    case EMBER_NETWORK_DOWN:
      sinkNodeId = EMBER_NULL_NODE_ID;
      emberAfCorePrintln("Network down");
      break;
    case EMBER_JOIN_FAILED:
      emberAfCorePrintln("Join failed");
      break;
    default:
      emberAfCorePrintln("Stack status: 0x%x", status);
      break;
  }
}

// This callback is called when the application starts and can be used to
// perform any additional initialization required at system startup.
void emberAfMainInitCallback(void)
{
  emberAfCorePrintln("INIT: %p", EMBER_AF_DEVICE_NAME);
  emberAfCorePrintln("\n%p>", EMBER_AF_DEVICE_NAME);

  emberNetworkInit();
}

// This callback is called in each iteration of the main application loop and
// can be used to perform periodic functions.
void emberAfMainTickCallback(void)
{
  #ifndef UNIX_HOST
//  if (emberStackIsUp()) {
//    halSetLed(NETWORK_UP_LED);
//  } else {
//    halClearLed(NETWORK_UP_LED);
//  }
  #endif
}

void emberAfFrequencyHoppingStartClientCompleteCallback(EmberStatus status)
{
  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("FH Client sync failed, status=0x%x", status);
  } else {
    emberAfCorePrintln("FH Client Sync Success");
  }
}

void emberAfEnergyScanCompleteCallback(int8_t mean,
                                       int8_t min,
                                       int8_t max,
                                       uint16_t variance)
{
  emberAfCorePrintln("Energy scan complete, mean=%d min=%d max=%d var=%d",
                     mean, min, max, variance);
}

#if defined(EMBER_AF_PLUGIN_MICRIUM_RTOS) && defined(EMBER_AF_PLUGIN_MICRIUM_RTOS_APP_TASK1)

// Simple application task that prints something every second.

void emberAfPluginMicriumRtosAppTask1InitCallback(void)
{
  emberAfCorePrintln("app task init");
}

#include <kernel/include/os.h>
#define TICK_INTERVAL_MS 1000

void emberAfPluginMicriumRtosAppTask1MainLoopCallback(void *p_arg)
{
  RTOS_ERR err;
  OS_TICK yieldTimeTicks = (OSCfg_TickRate_Hz * TICK_INTERVAL_MS) / 1000;

  while (true) {
    emberAfCorePrintln("app task tick");

    OSTimeDly(yieldTimeTicks, OS_OPT_TIME_DLY, &err);
  }
}

#endif // EMBER_AF_PLUGIN_MICRIUM_RTOS && EMBER_AF_PLUGIN_MICRIUM_RTOS_APP_TASK1

//------------------------------------------------------------------------------
// Static functions

static EmberStatus send(EmberNodeId nodeId,
                        SensorSinkCommandId commandId,
                        uint8_t *buffer,
                        uint8_t bufferLength)
{
  messageLength = 0;
  emberStoreLowHighInt16u(message + messageLength, SENSOR_SINK_PROTOCOL_ID);
  messageLength += 2;
  message[messageLength++] = commandId;
  MEMCOPY(message + messageLength, emberGetEui64(), EUI64_SIZE);
  messageLength += EUI64_SIZE;
  emberStoreLowHighInt16u(message + messageLength, emberGetNodeId());
  messageLength += 2;
  if (bufferLength != 0) {
    MEMCOPY(message + messageLength, buffer, bufferLength);
    messageLength += bufferLength;
  }
  return emberMessageSend(nodeId,
                          0, // endpoint
                          0, // messageTag
                          messageLength,
                          message,
                          txOptions);
}

static EmberStatus requestAdvertise(void)
{
  EmberStatus status = send(EMBER_COORDINATOR_ADDRESS,
                            SENSOR_SINK_COMMAND_ID_ADVERTISE_REQUEST,
                            NULL,
                            0);
  // Enable short poll to get the advertise unicast. This implicitly kicks
  // off the periodic polling.
  if (emberGetNodeType() == EMBER_STAR_SLEEPY_END_DEVICE) {
    emberAfPluginPollEnableShortPolling(TRUE);
  }
  // Use the report event to timeout on the pairing process.
  emberEventControlSetDelayMS(reportControl, SENSOR_PAIR_TIMEOUT_MS);
  emberAfCorePrintln("TX: Advertise Request (unicast), status 0x%x",
                     status);

  return status;
}

//------------------------------------------------------------------------------
// CLI commands

void joinCommand(void)
{
  EmberStatus status;
  EmberNetworkParameters parameters;

  uint8_t length = 0;
  uint8_t *contents = NULL;

  // Initialize the security key to the default key prior to joining the
  // network.
  emberSetSecurityKey(&securityKey);

  MEMSET(&parameters, 0, sizeof(EmberNetworkParameters));
  parameters.radioTxPower = txPower;
  parameters.radioChannel = emberUnsignedCommandArgument(0);
  parameters.panId = SENSOR_SINK_PAN_ID;

  if (emberCommandArgumentCount() > 1) {
    contents = emberStringCommandArgument(1, &length);
    status = emberSetSelectiveJoinPayload(length, contents);
  } else {
    emberClearSelectiveJoinPayload();
  }

  status = emberJoinNetwork(EMBER_STAR_END_DEVICE, &parameters);
  emberAfCorePrintln("join end device 0x%x", status);
}

void joinSleepyCommand(void)
{
  EmberStatus status;
  EmberNetworkParameters parameters;

  // Initialize the security key to the default key prior to joining the
  // network.
  emberSetSecurityKey(&securityKey);

  MEMSET(&parameters, 0, sizeof(EmberNetworkParameters));
  parameters.radioTxPower = txPower;
  parameters.radioChannel = emberUnsignedCommandArgument(0);
  parameters.panId = SENSOR_SINK_PAN_ID;
  status = emberJoinNetwork(EMBER_STAR_SLEEPY_END_DEVICE, &parameters);
  emberAfCorePrintln("join sleepy 0x%x", status);
}

void joinRangeExtenderCommand(void)
{
  EmberStatus status;
  EmberNetworkParameters parameters;

  // Initialize the security key to the default key prior to joining the
  // network.
  emberSetSecurityKey(&securityKey);

  MEMSET(&parameters, 0, sizeof(EmberNetworkParameters));
  parameters.radioTxPower = txPower;
  parameters.radioChannel = emberUnsignedCommandArgument(0);
  parameters.panId = SENSOR_SINK_PAN_ID;
  status = emberJoinNetwork(EMBER_STAR_RANGE_EXTENDER, &parameters);
  emberAfCorePrintln("join range extender 0x%x", status);
}

void pjoinCommand(void)
{
  EmberStatus status;
  uint8_t duration = (uint8_t)emberUnsignedCommandArgument(0);
  uint8_t length = 0;
  uint8_t *contents = NULL;

  if (emberCommandArgumentCount() > 1) {
    contents = emberStringCommandArgument(1, &length);
    status = emberSetSelectiveJoinPayload(length, contents);
  } else {
    emberClearSelectiveJoinPayload();
  }

  emberPermitJoining(duration);
}

void setTxOptionsCommand(void)
{
  txOptions = emberUnsignedCommandArgument(0);
  emberAfCorePrintln("TX options set: MAC acks %s, security %s, priority %s",
                     ((txOptions & EMBER_OPTIONS_ACK_REQUESTED) ? "enabled" : "disabled"),
                     ((txOptions & EMBER_OPTIONS_SECURITY_ENABLED) ? "enabled" : "disabled"),
                     ((txOptions & EMBER_OPTIONS_HIGH_PRIORITY) ? "enabled" : "disabled"));
}

void setTxPowerCommand(void)
{
  txPower = emberSignedCommandArgument(0);

  if (emberSetRadioPower(txPower) == EMBER_SUCCESS) {
    emberAfCorePrintln("TX power set: %d", (int8_t)emberGetRadioPower());
  } else {
    emberAfCorePrintln("TX power set failed");
  }
}

void setSecurityKeyCommand(void)
{
  EmberKeyData key;
  emberCopyKeyArgument(0, &key);

  if (emberSetSecurityKey(&key) == EMBER_SUCCESS) {
    uint8_t i;

    emberAfCorePrint("Security key set {");
    for (i = 0; i < EMBER_ENCRYPTION_KEY_SIZE; i++) {
      emberAfCorePrint("%x", key.contents[i]);
    }
    emberAfCorePrintln("}");
  } else {
    emberAfCorePrintln("Security key set failed");
  }
}

void setReportPeriodCommand(void)
{
  sensorReportPeriodMs = emberUnsignedCommandArgument(0);

  emberAfCorePrintln("Report period set to %d ms", sensorReportPeriodMs);
}

void advertiseRequestCommand(void)
{
  requestAdvertise();
}

void dataCommand(void)
{
  emberEventControlSetActive(reportControl);
}

void infoCommand(void)
{
  emberAfCorePrintln("%p:", EMBER_AF_DEVICE_NAME);
  emberAfCorePrintln("  network state: 0x%x", emberNetworkState());
  emberAfCorePrintln("      node type: 0x%x", emberGetNodeType());
  emberAfCorePrintln("          eui64: >%x%x%x%x%x%x%x%x",
                     emberGetEui64()[7],
                     emberGetEui64()[6],
                     emberGetEui64()[5],
                     emberGetEui64()[4],
                     emberGetEui64()[3],
                     emberGetEui64()[2],
                     emberGetEui64()[1],
                     emberGetEui64()[0]);
  emberAfCorePrintln("        node id: 0x%2x", emberGetNodeId());
  emberAfCorePrintln("         pan id: 0x%2x", emberGetPanId());
  emberAfCorePrintln("        channel: %d", (uint16_t)emberGetRadioChannel());
  emberAfCorePrintln("          power: %d", (int16_t)emberGetRadioPower());
  emberAfCorePrintln("     TX options: MAC acks %s, security %s, priority %s",
                     ((txOptions & EMBER_OPTIONS_ACK_REQUESTED) ? "enabled" : "disabled"),
                     ((txOptions & EMBER_OPTIONS_SECURITY_ENABLED) ? "enabled" : "disabled"),
                     ((txOptions & EMBER_OPTIONS_HIGH_PRIORITY) ? "enabled" : "disabled"));
}

void counterCommand(void)
{
  uint8_t counterType = emberUnsignedCommandArgument(0);
  uint32_t counter;
  EmberStatus status = emberGetCounter(counterType, &counter);

  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("Get counter failed, status=0x%x", status);
  } else {
    emberAfCorePrintln("Counter type=0x%x: %d", counterType, counter);
  }
}

void startEnergyScanCommand(void)
{
  EmberStatus status;
  uint8_t channelToScan = emberUnsignedCommandArgument(0);
  uint8_t samples = emberUnsignedCommandArgument(1);
  status = emberStartEnergyScan(channelToScan, samples);

  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("Start energy scanning failed, status=0x%x", status);
  } else {
    emberAfCorePrintln("Start energy scanning: channel %d, samples %d",
                       channelToScan, samples);
  }
}

EmberEventControl buttonEvent;
EmberEventControl blinkEvent;
static bool flash_is_erased = false;

void bootloaderFlashEraseCommand(void);
void bootloaderInitCommand(void);

/*
 * Join the network
 */
EmberStatus joinNetwork(void)
{
  EmberStatus status;
  EmberNetworkParameters parameters;

  emberSetSecurityKey(&securityKey);

  MEMSET(&parameters, 0, sizeof(EmberNetworkParameters));
  parameters.radioTxPower = txPower;
  parameters.radioChannel = 0;
  parameters.panId = SENSOR_SINK_PAN_ID;

  emberClearSelectiveJoinPayload();

  status = emberJoinNetwork(EMBER_STAR_END_DEVICE, &parameters);
  emberAfCorePrintln("join end device 0x%x", status);
  return status;
}

/*
 * Device joined network: LED1 ON, LED0 blinking
 * Device no network: LED1 OFF, LED0 OFF
 */
void blinkHandler(void){
  if (EMBER_JOINED_NETWORK == emberNetworkState()) {
    halSetLed(BOARDLED1);
    halToggleLed(BOARDLED0);
  } else {
    halClearLed(BOARDLED1);
    halClearLed(BOARDLED0);
  }
  emberEventControlSetDelayMS(blinkEvent, 300);
}

/*
 * It erase the slot flash for the first boot up.
 * Start joining the network if the device no network
 */
void buttonHandler(void)
{
  emberEventControlSetInactive(buttonEvent);

  if (flash_is_erased == false) {
    bootloaderInitCommand();
    bootloaderFlashEraseCommand();
	  flash_is_erased = true;
    emberEventControlSetDelayMS(blinkEvent, 300);
  }
  if (EMBER_NO_NETWORK == emberNetworkState()){
    joinNetwork();
  }
}

/*
 * LED1 ON while button pressed. LED1 OFF while button released.
 * And then active buttonEvent
 */
void halButtonIsr(uint8_t button, uint8_t state)
{
  halSetLed(BOARDLED1);
  if (state == BUTTON_RELEASED) {
    halClearLed(BOARDLED1);
    emberEventControlSetActive(buttonEvent);
  }
}
