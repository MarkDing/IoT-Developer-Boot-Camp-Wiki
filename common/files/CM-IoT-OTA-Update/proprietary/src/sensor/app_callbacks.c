/***************************************************************************//**
 * @file app_callbacks.c
 * @brief app_callbacks.c
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include PLATFORM_HEADER
#include "stack/include/ember.h"
#include "hal/hal.h"
#include "em_chip.h"
#include "sl_flex_assert.h"
#include "sl_si70xx.h"
#include "sl_i2cspm_instances.h"
#include "poll.h"
#include "sl_app_common.h"
#include "app_process.h"
#include "app_framework_common.h"
#include "sl_simple_led_instances.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#define MAX_TX_FAILURES     (10u)

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
/// report timing event control
EmberEventControl *report_control;
/// report timing period
uint16_t sensor_report_period_ms =  (1 * MILLISECOND_TICKS_PER_SECOND);
/// TX options set up for the network
EmberMessageOptions tx_options = EMBER_OPTIONS_ACK_REQUESTED;

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
/**************************************************************************//**
 * Brief Helper function for sending a command message to sinks.
 *
 * @param node_id is the destination sink node ID
 * @param command_id is the command that is being sent to the sink node
 * @param *buffer is a piece of data related to the command
 * @param buffer_length is the length of the buffer
 * @returns Returns an EmberStatus value indicating the outcome of the transmission.
 *****************************************************************************/
static EmberStatus send(EmberNodeId node_id,
                        sensor_sink_command_id command_id,
                        uint8_t *buffer,
                        uint8_t buffer_length);
/// Destination of the currently processed sink node
static EmberNodeId sink_node_id = EMBER_NULL_NODE_ID;
/// Current contents of the data sent with send()
static uint8_t message[SENSOR_SINK_MAXIMUM_LENGTH];
/// Length of the message data to eb sent with send()
static EmberMessageLength message_length;
/// Sink pair confirmed or not
static bool confirmed;
/// Number of failed message transmissons
static uint8_t tx_failure_count = 0;

//------------------------------------------------------------------------------
//                                  Callbacks
//------------------------------------------------------------------------------

/**************************************************************************//**
 * Here we print out the first two bytes reported by the sinks as a little
 * endian 16-bits decimal.
 *****************************************************************************/
void report_handler(void)
{
  if (!emberStackIsUp() || sink_node_id == EMBER_NULL_NODE_ID || !confirmed) {
    emberEventControlSetInactive(*report_control);
    emberAfPluginPollEnableShortPolling(FALSE);
  } else {
    EmberStatus status;
    EmberStatus sensor_status = EMBER_SUCCESS;
    uint8_t buffer[SENSOR_SINK_MAXIMUM_DATA_LENGTH];
    int32_t temp_data = 0;
    uint32_t rh_data = 0;
    uint8_t i;

    // Sample temperature and humidity from sensors.
    // Temperature is sampled in "millicelsius".
    #ifndef UNIX_HOST
    if (sl_si70xx_measure_rh_and_temp(sl_i2cspm_sensor,
                                      SI7021_ADDR,
                                      &rh_data,
                                      &temp_data)) {
      sensor_status = EMBER_ERR_FATAL;
      APP_INFO("Warning! Invalid Si7021 reading: %lu %ld\n", rh_data, temp_data);
    }
    #endif

    if (sensor_status == EMBER_SUCCESS) {
      emberStoreLowHighInt32u(buffer, temp_data);
      emberStoreLowHighInt32u(buffer + 4, rh_data);

      status = send(sink_node_id,
                    SENSOR_SINK_COMMAND_ID_DATA,
                    buffer,
                    SENSOR_SINK_MAXIMUM_DATA_LENGTH);
      APP_INFO("TX: Data to 0x%04X:", sink_node_id);
      for (i = 0; i < SENSOR_SINK_MAXIMUM_DATA_LENGTH; i++) {
        APP_INFO(" %02X", buffer[i]);
      }
      APP_INFO(": 0x%02X\n", status);
      emberEventControlSetDelayMS(*report_control, sensor_report_period_ms);
    }
  }
}

/**************************************************************************//**
 * Entering sleep is approved or denied in this callback, depending on user
 * demand.
 *****************************************************************************/
bool emberAfCommonOkToEnterLowPowerCallback(bool enter_em2, uint32_t duration_ms)
{
  (void) enter_em2;
  (void) duration_ms;
  return enable_sleep;
}

#include "sl_ota_bootloader_test_common.h"
#include "sl_btl-interface.h"
#include "sl_simple_button.h"

EmberEventControl *button_event_control;
EmberEventControl *led_control;

#define SENSOR_SINK_COMMAND_ID_OTA_REQUEST  (0xA5)  //Don't overlap with enum define in sl_app_common.h

static bool flash_is_erased = false;
static bool ota_update = false;
extern EmberKeyData security_key;

void OtaImageDownloadCompleteCallback(void)
{
  ota_update = false;
}
/*
 * Join the network
 */
void joinNetwork(void)
{
  EmberNetworkParameters parameters;

  MEMSET(&parameters, 0, sizeof(EmberNetworkParameters));
  parameters.radioTxPower = SENSOR_SINK_TX_POWER;
  parameters.radioChannel = 0;
  parameters.panId = SENSOR_SINK_PAN_ID;

  emberClearSelectiveJoinPayload();

  emberJoinNetwork(EMBER_STAR_END_DEVICE, &parameters);
}

/*
 * Erase the slot flash for the first boot up.
 * Start joining the network if the device no network
 * Sending OTA update request to server once joined the network
 */
void button_handler(void)
{
  EmberNetworkStatus status;

  emberEventControlSetInactive(*button_event_control);

  if (flash_is_erased == false) {
    flash_is_erased = true;

    status = emberAfPluginBootloaderInterfaceInit();
    if (status) {
      APP_INFO("bootloader init succeeded!\n\r");
    } else {
      APP_INFO("bootloader init failed! wrong chip?\n\r");
    }

    APP_INFO("flash erase started\n");
    emberAfPluginBootloaderInterfaceChipErase();
    ota_resume_start_counter_reset = true;
  }

  status = emberNetworkState();
  APP_INFO("network status = 0x%x\n\r", status);
  if (EMBER_NO_NETWORK == status){
    APP_INFO("Joining the network\n\r");
    joinNetwork();
    emberEventControlSetDelayMS(*button_event_control, 2000);
  }

  if(EMBER_JOINED_NETWORK == status){
    char* is_ack = ((tx_options & EMBER_OPTIONS_ACK_REQUESTED) ? "enabled" : "disabled");
    char* is_security = ((tx_options & EMBER_OPTIONS_SECURITY_ENABLED) ? "enabled" : "disabled");
    char* is_high_prio = ((tx_options & EMBER_OPTIONS_HIGH_PRIORITY) ? "enabled" : "disabled");

    APP_INFO("Info:\r\n");
    APP_INFO("         MCU Id: 0x%llX\r\n", SYSTEM_GetUnique());
    APP_INFO("  Network state: 0x%02X\r\n", emberNetworkState());
    APP_INFO("      Node type: 0x%02X\r\n", emberGetNodeType());
    APP_INFO("        Node id: 0x%04X\r\n", emberGetNodeId());
    APP_INFO("         Pan id: 0x%04X\r\n", emberGetPanId());
    APP_INFO("        Channel: %d\r\n", (uint16_t)emberGetRadioChannel());
    APP_INFO("          Power: %d\r\n", (int16_t)emberGetRadioPower());
    APP_INFO("     TX options: MAC acks %s, security %s, priority %s\r\n", is_ack, is_security, is_high_prio);

    ota_update = true;

    //Slow down report period
    sensor_report_period_ms = (10 * MILLISECOND_TICKS_PER_SECOND);
    APP_INFO("Report period set to %d ms\n\r", sensor_report_period_ms);

    APP_INFO("Sending OTA update request\n\r");
    send(EMBER_COORDINATOR_ADDRESS, SENSOR_SINK_COMMAND_ID_OTA_REQUEST, NULL, 0);
  }
}

void halButtonIsr(uint8_t state)
{
  if (state == SL_SIMPLE_BUTTON_RELEASED) {
    emberEventControlSetDelayMS(*button_event_control, 300);
  }
}

void led_handler(void)
{
  emberEventControlSetInactive(*led_control);
  if (ota_update == true) {
    emberEventControlSetDelayMS(*led_control, 200);
    sl_led_toggle(&sl_led_led0);
  }else {
    emberEventControlSetDelayMS(*led_control, 2000);
    if (EMBER_NO_NETWORK == emberNetworkState()) {
      sl_led_turn_off(&sl_led_led0);
    } else {
      sl_led_turn_on(&sl_led_led0);
    }
  }
}

/******************************************************************************
* Application framework init callback
******************************************************************************/
void emberAfInitCallback(void)
{
  emberAfAllocateEvent(&report_control, &report_handler);
  emberAfAllocateEvent(&button_event_control, &button_handler);
  emberAfAllocateEvent(&led_control, &led_handler);
  // CLI info message
  APP_INFO("Sensor\r\n");

  ota_update = false;

  emberSetSecurityKey(&security_key);

  emberNetworkInit();

  emberEventControlSetDelayMS(*led_control, 1000);

#if defined(EMBER_AF_PLUGIN_BLE)
  bleConnectionInfoTableInit();
#endif
}

/**************************************************************************//**
 * This function is called when a message is received.
 *****************************************************************************/
void emberAfIncomingMessageCallback(EmberIncomingMessage *message)
{
  if (message->length < SENSOR_SINK_MINIMUM_LENGTH
      || (emberFetchLowHighInt16u(message->payload + SENSOR_SINK_PROTOCOL_ID_OFFSET)
          != SENSOR_SINK_PROTOCOL_ID)) {
    return;
  }

  switch (message->payload[SENSOR_SINK_COMMAND_ID_OFFSET]) {
    case SENSOR_SINK_COMMAND_ID_ADVERTISE_REQUEST:
      APP_INFO("RX: Advertise Request from 0x%02X\n", message->source);
      break;
    case SENSOR_SINK_COMMAND_ID_ADVERTISE:
    {
      APP_INFO("RX: Advertise from 0x%02X\n", message->source);
      if (sink_node_id == EMBER_NULL_NODE_ID || !confirmed) {
        EmberStatus status = send(EMBER_COORDINATOR_ADDRESS,
                                  SENSOR_SINK_COMMAND_ID_PAIR_REQUEST,
                                  NULL,
                                  0);
        APP_INFO("TX: Pair Request to 0x%04X: 0x%02X\n",
                 EMBER_COORDINATOR_ADDRESS,
                 status);
        if (status == EMBER_SUCCESS) {
          sink_node_id = EMBER_COORDINATOR_ADDRESS;
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
      APP_INFO("RX: Pair Request from 0x%02X\n", message->source);
      break;
    case SENSOR_SINK_COMMAND_ID_PAIR_CONFIRM:
    {
      APP_INFO("RX: Pair Confirm from 0x%02X\n", message->source);
      if (message->source == sink_node_id) {
        confirmed = TRUE;
        report_handler();
        emberAfPluginPollEnableShortPolling(FALSE);
      }
      break;
    }
    case SENSOR_SINK_COMMAND_ID_DATA:
    {
      uint8_t i;
      APP_INFO("RX: Data from 0x%04X:", message->source);
      for (i = SENSOR_SINK_DATA_OFFSET; i < message->length; i++) {
        APP_INFO(" %x", message->payload[i]);
      }
      APP_INFO("\n");
      break;
    }
    default:
      APP_INFO("RX: Unknown from 0x%04X\n", message->source);
      break;
  }
}

/**************************************************************************//**
 * This function is called to indicate whether an outgoing message was
 * successfully transmitted or to indicate the reason of failure.
 *****************************************************************************/
void emberAfMessageSentCallback(EmberStatus status,
                                EmberOutgoingMessage *message)
{
  if (status != EMBER_SUCCESS) {
    APP_INFO("TX: 0x%02X\n", status);
    tx_failure_count++;
    if (SENSOR_SINK_MINIMUM_LENGTH <= message->length
        && (emberFetchLowHighInt16u(message->payload + SENSOR_SINK_PROTOCOL_ID_OFFSET)
            == SENSOR_SINK_PROTOCOL_ID
            && (message->payload[SENSOR_SINK_COMMAND_ID_OFFSET]
                == SENSOR_SINK_COMMAND_ID_DATA))
        && tx_failure_count >= MAX_TX_FAILURES) {
      APP_INFO("EVENT: dropped sink 0x%04X\n", sink_node_id);
      sink_node_id = EMBER_NULL_NODE_ID;
    }
  } else {
    // Success: reset the failures count.
    tx_failure_count = 0;
  }
}

/**************************************************************************//**
 * This function is called when the stack status changes.
 *****************************************************************************/
void emberAfStackStatusCallback(EmberStatus status)
{
  switch (status) {
    case EMBER_NETWORK_UP:
      APP_INFO("Network up\n");
      // Unicast an advertise request to the sink.
      request_advertise();
      break;
    case EMBER_NETWORK_DOWN:
      APP_INFO("Network down\n");
      break;
    case EMBER_JOIN_FAILED:
      APP_INFO("Join failed\n");
      break;
    default:
      APP_INFO("Stack status: 0x%02X\n", status);
      break;
  }
}

/**************************************************************************//**
 * This callback is called in each iteration of the main application loop and
 * can be used to perform periodic functions.
 *****************************************************************************/
void emberAfTickCallback(void)
{
//  if (emberStackIsUp()) {
//    sl_led_turn_on(&sl_led_led0);
//  } else {
//    sl_led_turn_off(&sl_led_led0);
//  }
}

/**************************************************************************//**
 * This function is called when a frequency hopping client completed the start
 * procedure.
 *****************************************************************************/
void emberAfFrequencyHoppingStartClientCompleteCallback(EmberStatus status)
{
  if (status != EMBER_SUCCESS) {
    APP_INFO("FH Client sync failed, status=0x%02X\n", status);
  } else {
    APP_INFO("FH Client Sync Success\n");
  }
}

/**************************************************************************//**
 * This function is called when a node has joined the network.
 *****************************************************************************/
void emberAfEnergyScanCompleteCallback(int8_t mean,
                                       int8_t min,
                                       int8_t max,
                                       uint16_t variance)
{
  APP_INFO("Energy scan complete, mean=%d min=%d max=%d var=%d\n",
           mean, min, max, variance);
}

#if defined(EMBER_AF_PLUGIN_MICRIUM_RTOS) && defined(EMBER_AF_PLUGIN_MICRIUM_RTOS_APP_TASK1)

/**************************************************************************//**
 * This function is called from the Micrium RTOS plugin before the
 * Application (1) task is created.
 *****************************************************************************/
void emberAfPluginMicriumRtosAppTask1InitCallback(void)
{
  APP_INFO("app task init\n");
}

#include <kernel/include/os.h>
#define TICK_INTERVAL_MS 1000

/**************************************************************************//**
 * This function implements the Application (1) task main loop.
 *****************************************************************************/
void emberAfPluginMicriumRtosAppTask1MainLoopCallback(void *p_arg)
{
  RTOS_ERR err;
  OS_TICK yield_time_ticks = (OSCfg_TickRate_Hz * TICK_INTERVAL_MS) / 1000;

  while (true) {
    APP_INFO("app task tick\n");

    OSTimeDly(yield_time_ticks, OS_OPT_TIME_DLY, &err);
  }
}

#endif // EMBER_AF_PLUGIN_MICRIUM_RTOS && EMBER_AF_PLUGIN_MICRIUM_RTOS_APP_TASK1

/**************************************************************************//**
 * Request the sink to advertise.
 *****************************************************************************/
EmberStatus request_advertise(void)
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
  emberEventControlSetDelayMS(*report_control, SENSOR_PAIR_TIMEOUT_MS);
  APP_INFO("TX: Advertise Request (unicast), status 0x%02X\n",
           status);

  return status;
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
/**************************************************************************//**
 * Helper function to send messages to the sink (coordinator).
 *****************************************************************************/
static EmberStatus send(EmberNodeId node_id,
                        sensor_sink_command_id command_id,
                        uint8_t *buffer,
                        uint8_t buffer_length)
{
  message_length = 0;
  emberStoreLowHighInt16u(message + message_length, SENSOR_SINK_PROTOCOL_ID);
  message_length += 2;
  message[message_length++] = command_id;
  MEMCOPY(message + message_length, emberGetEui64(), EUI64_SIZE);
  message_length += EUI64_SIZE;
  emberStoreLowHighInt16u(message + message_length, emberGetNodeId());
  message_length += 2;
  if (buffer_length != 0) {
    MEMCOPY(message + message_length, buffer, buffer_length);
    message_length += buffer_length;
  }
  return emberMessageSend(node_id,
                          0, // endpoint
                          0, // messageTag
                          message_length,
                          message,
                          tx_options);
}
