/***************************************************************************//**
 * @file
 * @brief Core application logic.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "em_common.h"
#include "sl_app_assert.h"
#include "sl_app_log.h"
#include "sl_bluetooth.h"
#include "gatt_db.h"
#include "app.h"

#include "btl_interface.h"
#include "btl_interface_storage.h"

#include "sl_simple_led_instances.h"
#include "sl_simple_led.h"

// connection parameters
#define CONN_INTERVAL_MIN             80   //100ms
#define CONN_INTERVAL_MAX             80   //100ms
#define CONN_SLAVE_LATENCY            0    //no latency
#define CONN_TIMEOUT                  100  //1000ms
#define CONN_MIN_CE_LENGTH            0
#define CONN_MAX_CE_LENGTH            0xffff

#define DISCONNECTED  0
#define SCANNING      1
#define FIND_SERVICE  2
#define FIND_CHAR     3
#define ENABLE_NOTIF  4
#define OTA_START     5
#define OTA_STOP      6

//ota service uuid //1d14d6ee-fd63-4fa1-bfa4-8f47b42119f0
static const uint8_t uuid_ota_service[16] = { 0xf0, 0x19, 0x21, 0xb4, 0x47, 0x8f, 0xa4, 0xbf, 0xa1, 0x4f, 0x63, 0xfd, 0xee, 0xd6, 0x14, 0x1d };
//ota control char uuid //F7BF3564-FB6D-4E53-88A4-5E37E0326063
static const uint8_t uuid_ota_control_char[16] = { 0x63, 0x60, 0x32, 0xe0, 0x37, 0x5e, 0xa4, 0x88, 0x53, 0x4e, 0x6d, 0xfb, 0x64, 0x35, 0xbf, 0xf7 };
//ota control char uuid //984227F3-34FC-4045-A5D0-2C581F81A153
static const uint8_t uuid_ota_data_char[16] = { 0x53, 0xa1, 0x81, 0x1f, 0x58, 0x2c, 0xd0, 0xa5, 0x45, 0x40, 0xfc, 0x34, 0xf3, 0x27, 0x42, 0x98 };

/* soft timer handles */
#define RESTART_TIMER    1
#define WAIT_TIMER    2

static void bleOtaDataTransmit(void);
static int32_t readOtaData(uint32_t *imgSize);

static void check_image_validity();


/***************************************************************************************************
 Local Variables
 **************************************************************************************************/
static uint8_t _conn_handle;
static int _main_state;
static uint32_t _service_handle;
static uint16_t _char_control_handle;
static uint16_t _char_data_handle;
static uint8_t* charValue;

static int32_t verifyImage_result;


/* Default maximum packet size is 20 bytes. This is adjusted after connection is opened based
 * on the connection parameters */
static uint8_t _max_packet_size;
static uint8_t _min_packet_size;  // Target minimum bytes for one packet

extern const sl_led_t sl_led_led0;

// Tag ID 4 bytes + Tag Length 4 bytes + Tag Payload
typedef struct {
  uint32_t tag;
  uint32_t len;
} GblTag_t;


static void reset_variables()
{
  _conn_handle = 0xFF;
  _main_state = DISCONNECTED;
  _service_handle = 0;
  _char_control_handle = 0;
  _char_data_handle = 0;

  _max_packet_size = 20;
  _min_packet_size = 20;

}


// Parse advertisements looking for advertised Silicon Labs OTA service
static uint8_t find_service_in_advertisement(uint8_t *data, uint8_t len)
{
  uint8_t ad_field_length;
  uint8_t ad_field_type;
  uint8_t i = 0;
  // Parse advertisement packet
  while (i < len) {
    ad_field_length = data[i];
    ad_field_type = data[i + 1];
    // Partial ($06) or complete ($07) list of 128-bit UUIDs
    if (ad_field_type == 0x06 || ad_field_type == 0x07) {
      // compare UUID to Silicon Labs OTA service UUID
      if (memcmp(&data[i + 2], uuid_ota_service, 16) == 0) {
        return 1;
      }
    }
    // advance to the next AD struct
    i = i + ad_field_length + 1;
  }
  return 0;
}


/**
 * Read TAG of OTA GBL image and calculate the size of GBL
 */
static int32_t readOtaData(uint32_t *imgSize)
{

  uint32_t otaDfuSlotId = 0;
  uint32_t size = 0;
  int32_t result;
  GblTag_t gbl;

//   to see if the first tag is header tag, if not then exit
  bootloader_readStorage(otaDfuSlotId, size, (uint8_t *)&gbl, 8);
  if (gbl.tag != 0x03A617EB){
      sl_app_log("The first tag is not header tag, exit!\r\n");
    return -1;
  }


  while(1){
    result = bootloader_readStorage(otaDfuSlotId, size, (uint8_t *)&gbl, 8);

    if(BOOTLOADER_OK == result){
      size = size + 8 + gbl.len;
      sl_app_log("GBL tag = 0x%4d, len = 0x%4d, size = 0x%4d\r\n", gbl.tag, gbl.len, size);
    }
    else{
        sl_app_log("Unexpected error(data read file): %x\r\n", result);
      break;
    }

    // reach end tag of GBL
    if (gbl.tag == 0xFC0404FC){
        sl_app_log("reach end of tag of GBL\r\n");
        *imgSize = size;
      return 0;
    }
  }
  return -1;
}

bool write_ctl_char_rsp = false;

static void bleOtaDataTransmit(void)
{
  int32_t otaDfuSlotId = 0;
  int32_t otaDfuOffset = 0;
  uint32_t dfu_toload = 0;
  uint32_t ota_image_size = 0;
  uint32_t dfu_size = 0;
  int16_t result;
  uint8_t dfu_data[256];

  sl_status_t sc;
  uint16_t sent_len;

  if(verifyImage_result == 0){

      sl_app_log("verifyImage_result = %d\r\n", verifyImage_result);
      result = readOtaData(&ota_image_size);
        if (result != 0){
            sl_app_log("Read ota image error\r\n");
        } else {
            sl_app_log("ota image size: 0x%4d\r\n", ota_image_size);
            dfu_toload = ota_image_size;
            sl_app_log("dfu to load size: 0x%4d\r\n", dfu_toload);
        }
  } else{
      sl_app_log("No valid GBL image found\r\n");
  }


  if(!write_ctl_char_rsp)
    {
      sl_bt_system_set_soft_timer(32768, RESTART_TIMER, 0);
    }


    while (dfu_toload > 0){

        dfu_size = dfu_toload > _max_packet_size ? _max_packet_size: dfu_toload; // _max_packet_size = mtu - 3

        //transmit ota image
        result = bootloader_readStorage(otaDfuSlotId, otaDfuOffset, dfu_data, dfu_size);
        if (result){
            sl_app_log("Unexpected error(read slot): %x\r\n", result);
            sl_bt_connection_close(_conn_handle);
        }

        do {
          sc = sl_bt_gatt_write_characteristic_value_without_response(_conn_handle, _char_data_handle, dfu_size, dfu_data, &sent_len);
        } while (sc != SL_STATUS_OK);

//        sc = sl_bt_gatt_write_characteristic_value(_conn_handle, _char_data_handle, dfu_size, dfu_data);

        if(sc){
            sl_app_log("Unexpected error, characteristic write failed: 0x%x\r\n", sc);
            sl_bt_connection_close(_conn_handle);
        }

        sl_app_log("transmit ota image otaDfuOffset = %d, dfu_size = %d\r\n", otaDfuOffset, dfu_size);

        otaDfuOffset += dfu_size;
        dfu_toload -= dfu_size;
        sl_app_log("dfu to load : 0x%4d\r\n", dfu_toload);
    }

    if (ota_image_size > 0){
        //signal to finish bleOtaDfuTransaction
      //  sc = sl_bt_gatt_write_characteristic_value_without_response(_conn_handle, _char_control_handle, 1, (uint8_t*)"\x03",&sent_len);
          sc = sl_bt_gatt_write_characteristic_value(_conn_handle, _char_control_handle, 1, (uint8_t*)"\x03");
          if (sc){
              sl_app_log("Unexpected error(finish read file): %x\r\n", sc);
              sl_bt_connection_close(_conn_handle);
          }else{
              sl_app_log("Finish transmitting ota image\r\n");
              sl_led_turn_off(&sl_led_led0); //LED0 turn off, finish sending ota image
          }
    }

}


/**************************************************************************//**
 * Application Init.
 *****************************************************************************/
SL_WEAK void app_init(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application init code here!                         //
  // This is called once during start-up.                                    //
  /////////////////////////////////////////////////////////////////////////////

  check_image_validity();
}

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
SL_WEAK void app_process_action(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application code here!                              //
  // This is called infinitely.                                              //
  // Do not call blocking functions from here!                               //
  /////////////////////////////////////////////////////////////////////////////
}

static void check_image_validity()
{
  int32_t result;
  uint32_t otaDfuSlotId = 0;
  ApplicationData_t appInfo;
  uint32_t bootloaderVersion;
  BootloaderStorageSlot_t storageSlot;
  BootloaderStorageInformation_t storageInfo;

  bootloader_init();

  bootloader_getStorageInfo(&storageInfo);
  sl_app_log("storage information version 0x%x, capability %d, num slots %d\r\n", storageInfo.version, storageInfo.capabilities, storageInfo.numStorageSlots);

  result = bootloader_getStorageSlotInfo(otaDfuSlotId, &storageSlot);
  if (BOOTLOADER_OK != result) {
    return;
  }
  sl_app_log("slot address 0x%x, length %d\r\n", storageSlot.address, storageSlot.length);

  verifyImage_result = bootloader_verifyImage(otaDfuSlotId, NULL);
  if(verifyImage_result != BOOTLOADER_OK){
        sl_app_log("No valid GBL image found, error %d\r\n", verifyImage_result);
        return;
    }
    sl_app_log("bootloader_verifyImage done\r\n");

  //   to see if new image has been flashed to OTA Server or not
  result = bootloader_getImageInfo(otaDfuSlotId, &appInfo, &bootloaderVersion);
  if(result != BOOTLOADER_OK){
      sl_app_log("No valid GBL image found, error %d\r\n", result);
      return;
  }
  sl_app_log("Application version %d, bootloader Version %x\r\n", appInfo.version, bootloaderVersion);
}


/**************************************************************************//**
 * Bluetooth stack event handler.
 * This overrides the dummy weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/
void sl_bt_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;
  uint16_t set_mtu;

  switch (SL_BT_MSG_ID(evt->header)) {
    // -------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack command before receiving this boot event!
    case sl_bt_evt_system_boot_id:
      // Print boot message.
      sl_app_log("Bluetooth stack booted: v%d.%d.%d-b%d\n",
                       evt->data.evt_system_boot.major,
                       evt->data.evt_system_boot.minor,
                       evt->data.evt_system_boot.patch,
                       evt->data.evt_system_boot.build);
      reset_variables();

      sc = sl_bt_gatt_set_max_mtu(247, &set_mtu);
      sl_app_assert(sc == SL_STATUS_OK,
                    "[E: 0x%04x] Failed to set discovery type\n",
                    (int)sc);

      // Set passive scanning on 1Mb PHY
      sc = sl_bt_scanner_set_mode(gap_1m_phy, 0);
      sl_app_assert(sc == SL_STATUS_OK,
                    "[E: 0x%04x] Failed to set discovery type\n",
                    (int)sc);
      // Set scan interval and scan window
      sc = sl_bt_scanner_set_timing(gap_1m_phy, 16, 16); // scan window 10ms, scan interval 10 ms
      sl_app_assert(sc == SL_STATUS_OK,
                    "[E: 0x%04x] Failed to set discovery timing\n",
                    (int)sc);
      // Set the default connection parameters for subsequent connections
      sc = sl_bt_connection_set_default_parameters(CONN_INTERVAL_MIN,
                                                   CONN_INTERVAL_MAX,
                                                   CONN_SLAVE_LATENCY,
                                                   CONN_TIMEOUT,
                                                   CONN_MIN_CE_LENGTH,
                                                   CONN_MAX_CE_LENGTH);
      sl_app_assert(sc == SL_STATUS_OK,
                    "[E: 0x%04x] Failed to set connection timing\n",
                    (int)sc);
      // Start scanning - looking for thermometer devices
      sc = sl_bt_scanner_start(gap_1m_phy, scanner_discover_generic);

      sl_app_assert(sc == SL_STATUS_OK,
                    "[E: 0x%04x] Failed to start discovery #1\n",
                    (int)sc);

      sl_app_log("Start scanning\n");
      _main_state = SCANNING;
      break;

    // -------------------------------
    // This event is generated when an advertisement packet or a scan response
    // is received from a slave
    case sl_bt_evt_scanner_scan_report_id:
      // Parse advertisement packets
      if (evt->data.evt_scanner_scan_report.packet_type == 0) {
        // If a thermometer advertisement is found...
        if (find_service_in_advertisement(&(evt->data.evt_scanner_scan_report.data.data[0]),
                                          evt->data.evt_scanner_scan_report.data.len) != 0) {
          // then stop scanning for a while
          sc = sl_bt_scanner_stop();
          sl_app_assert(sc == SL_STATUS_OK,
                        "[E: 0x%04x] Failed to stop discovery\n",
                        (int)sc);

          // and connect to that device
            sc = sl_bt_connection_open(evt->data.evt_scanner_scan_report.address,
                                       evt->data.evt_scanner_scan_report.address_type,
                                       gap_1m_phy,
                                       NULL);
            sl_app_assert(sc == SL_STATUS_OK,
                          "[E: 0x%04x] Failed to connect\n",
                          (int)sc);
        }
      }
      break;

    // -------------------------------
    // This event indicates that a new connection was opened.
    case sl_bt_evt_connection_opened_id:
      sl_app_log("Connection open \n");

      _conn_handle = evt->data.evt_connection_opened.connection;
      // Discover Silicon Labs OTA service on the slave device
      sc = sl_bt_gatt_discover_primary_services_by_uuid(evt->data.evt_connection_opened.connection,
                                                        sizeof(uuid_ota_service),
                                                        uuid_ota_service);
      sl_app_assert(sc == SL_STATUS_OK,
                    "[E: 0x%04x] Failed to discover primary services\n",
                    (int)sc);
      _main_state = FIND_SERVICE;
      break;

    // -------------------------------
    // This event indicates that a connection was closed.
    case sl_bt_evt_connection_closed_id:
      sl_app_log("Connection close\r\n");
      _main_state = DISCONNECTED;
      reset_variables();
      // Create one-shot soft timer that will restart discovery after 1 second delay
      sl_bt_system_set_soft_timer(32768, WAIT_TIMER, true);
      break;

    ///////////////////////////////////////////////////////////////////////////
    // Add additional event handlers here as your application requires!      //
    ///////////////////////////////////////////////////////////////////////////

    case sl_bt_evt_gatt_mtu_exchanged_id:
      /* Calculate maximum data per one notification / write-without-response, this depends on the MTU.
       * up to ATT_MTU-3 bytes can be sent at once  */
      _max_packet_size = evt->data.evt_gatt_mtu_exchanged.mtu - 3;
      _min_packet_size = _max_packet_size;  // Try to send maximum length packets whenever possible
      sl_app_log("MTU exchanged: %d\r\n", evt->data.evt_gatt_mtu_exchanged.mtu);
      break;

      // This event is generated when a new service is discovered
     case sl_bt_evt_gatt_service_id:

           sl_app_log("Service discovered\r\n");
           _service_handle = evt->data.evt_gatt_service.service;
           sl_app_log("Service handle: %d\r\n", _service_handle);

       break;

       // This event is generated when a new characteristic is discovered
     case sl_bt_evt_gatt_characteristic_id :

       if (evt->data.evt_gatt_characteristic.uuid.len == 16) {
         if (memcmp(uuid_ota_control_char, evt->data.evt_gatt_characteristic.uuid.data, 16) == 0) {
             sl_app_log("Char ota control discovered\r\n");
           _char_control_handle = evt->data.evt_gatt_characteristic.characteristic;
           sl_app_log("Characteristic ota control handle: %d\r\n", _char_control_handle);
         }
         else if (memcmp(uuid_ota_data_char, evt->data.evt_gatt_characteristic.uuid.data, 16) == 0) {
             sl_app_log("Char ota data discovered\r\n");
           _char_data_handle = evt->data.evt_gatt_characteristic.characteristic;
           sl_app_log("Characteristic ota data handle: %d\r\n", _char_data_handle);
         }
       }

       break;

      // This event is generated for various procedure completions, e.g. when a
      // write procedure is completed, or service discovery is completed
    case sl_bt_evt_gatt_procedure_completed_id:

      switch (_main_state) {

        case FIND_SERVICE:
          if (_service_handle > 0) {
            // Service found, next search for characteristics
              sc = sl_bt_gatt_discover_characteristics(_conn_handle, _service_handle);
              _main_state = FIND_CHAR;
          } else {
            // No service found -> disconnect
              sl_bt_connection_close(_conn_handle);
          }
          break;

        case FIND_CHAR:
          if (_char_control_handle > 0) {
            // Char found, turn on notification
//              sc = sl_bt_gatt_set_characteristic_notification(_conn_handle, _char_control_handle, gatt_notification);
                sc = sl_bt_gatt_set_characteristic_notification(_conn_handle, _char_control_handle, gatt_indication);
              _main_state = ENABLE_NOTIF;
          } else {
            // No characteristic found? -> disconnect
              sl_bt_connection_close(_conn_handle);
          }
          break;

        case ENABLE_NOTIF:
          _main_state = OTA_START;
          sl_app_log("Turn on notification/indication done\r\n");
          break;

        case OTA_START:
          _main_state = OTA_STOP;
          sl_app_log("Receive write response\r\n");
          write_ctl_char_rsp =  true;
          bleOtaDataTransmit();
          break;

        case OTA_STOP:
          sl_app_log("OTA_STOP\r\n");
          break;

        default:
          break;
      }
      break;

      // This event is generated when a characteristic value was received e.g. an indication/notification
    case sl_bt_evt_gatt_characteristic_value_id:
      charValue = &(evt->data.evt_gatt_characteristic_value.value.data[0]);
      sl_app_log("charValue receive: %d\r\n", *charValue);
      if (evt->data.evt_gatt_characteristic_value.characteristic == _char_control_handle) {
        if (*charValue == 0xA5){
           sl_app_log("receive OTA request\r\n");
          sl_led_turn_on(&sl_led_led0);//LED0 turn on, indicate receiving ota request

          //signal to begin bleOtaDfuTransaction
        //  sc = sl_bt_gatt_write_characteristic_value_without_response(_conn_handle, _char_control_handle, 1, (uint8_t*)"\x0",&sent_len);
            sc = sl_bt_gatt_write_characteristic_value(_conn_handle, _char_control_handle, 1, (uint8_t*)"\x0");
            if (sc){
                sl_app_log("Unexpected error(begin read file): %x\r\n", sc);
                sl_bt_connection_close(_conn_handle);
            }else{
                sl_app_log("Begin to transmit ota image\r\n");
            }

        }
      }
      break;

      /* Software Timer event */
    case sl_bt_evt_system_soft_timer_id:
      switch (evt->data.evt_system_soft_timer.handle) {
        case RESTART_TIMER:
          // Restart discovery using the default 1M PHY
          sl_bt_scanner_start(gap_1m_phy, scanner_discover_generic);
          _main_state = SCANNING;
          break;
        case WAIT_TIMER:
        // -------------------------------
        // Default event handler.
        default:
          break;
      }
      break;

      default:
      break;
  }
}


