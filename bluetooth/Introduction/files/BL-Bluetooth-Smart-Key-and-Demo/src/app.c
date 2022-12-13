/***************************************************************************//**
 * @file
 * @brief Core application logic.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#include <stdio.h>
#include <math.h>

#include "em_common.h"
#include "app_assert.h"
#include "sl_bluetooth.h"
#include "gatt_db.h"
#include "app.h"

#include "app_log.h"
#include "sl_simple_button_instances.h"
#include "sl_simple_button_btn1_config.h"
#include "sl_simple_led_instances.h"
#include "sl_simple_timer.h"

#define D_DIRECTED_ADV 1
#define D_THRESHOLD_UNLOCK 1.0
#define D_THRESHOLD_LOCK   2.5
#define D_UNLOCK 1
#define D_LOCK   0

#define KEY_ARRAY_SIZE 25
#define MODIFIER_INDEX 0
#define DATA_INDEX 2

#define SHIFT_KEY_CODE 0x02

#define APP_KEY_SIZE  16

#define APP_AUTHENTICATION_STATUS_KEY  0x4000

#define ARRAY_LEN(x)                (sizeof(x) / sizeof((x)[0]))

static uint8_t connection_handle = 0xff;
static uint8_t bonding_handle = 0xff;
static uint8_t connection_security = 0;
static uint8_t app_authenticated = 0;
static uint8_t adv_type = 0;

static uint8_t app_key[APP_KEY_SIZE] = {0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55};

// The advertising set handle allocated from Bluetooth stack.
static uint8_t advertising_set_handle = 0xff;

static float rssi_value;
static float distance;

uint8_t lock_state_current = D_LOCK;
uint8_t lock_state_target = D_LOCK;

bd_addr address_direct;
bd_addr address_peer;
uint8_t address_type;
uint8_t security_mode;
uint8_t key_size;
char *type[] = {"Public", "Static","Resolvable","Non-resolvable"};

// Periodic timer handle.
static sl_simple_timer_t app_periodic_timer;
static sl_simple_timer_t app_adv_timer;

static void app_periodic_timer_cb(sl_simple_timer_t *timer, void *data)
{
  (void)data;
  (void)timer;
  //sl_status_t sc;
  //app_log("app_periodic_timer_cb\r\n");
  sl_bt_connection_get_rssi(connection_handle);
}

static void app_adv_timer_cb(sl_simple_timer_t *timer, void *data)
{
  (void)data;
  (void)timer;
  sl_status_t sc;
  adv_type++;
  if(adv_type%2){
    app_log("legacy_advertiser\r\n");
    // Start advertising and enable connections.
    sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                       sl_bt_advertiser_connectable_scannable);
    app_assert_status(sc);
  }else{
    app_log("Directed ADV...\r\n");
    bd_addr target_addr = address_direct;// address_direct, address_peer;
    app_log("Client BD address: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
            target_addr.addr[5],
            target_addr.addr[4],
            target_addr.addr[3],
            target_addr.addr[2],
            target_addr.addr[1],
            target_addr.addr[0]);
    sc = sl_bt_legacy_advertiser_start_directed(
        advertising_set_handle,
        sl_bt_legacy_advertiser_low_duty_directed_connectable,//sl_bt_legacy_advertiser_low_duty_directed_connectable
        target_addr,
        1
        );
    app_assert_status(sc);
  }
}

static void app_door_init(void)
{
  app_log("BT KEY, Door lock!!!\r\n");
  sl_led_led0.turn_off(sl_led_led0.context);
}

static void app_door(void)
{
  if(app_authenticated == 0)
    return;

  if(lock_state_current != lock_state_target){
      lock_state_current = lock_state_target;
      app_log("RSSI: %f, Distance: %f meter(s)\r\n", rssi_value, distance);

      if(lock_state_current == D_UNLOCK){
        app_log("Door unlock!!!\r\n");
        sl_led_led0.turn_on(sl_led_led0.context);
      }else{
        app_log("Door lock!!!\r\n");
        sl_led_led0.turn_off(sl_led_led0.context);
      }
  }
}

void sl_button_on_change(const sl_button_t *handle)
{
  if(&sl_button_btn0 == handle){
      if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED){
          sl_bt_external_signal(1);
          //app_log("Button 0 pushed\r\n");
      } else {
          //app_log("Button 0 released\r\n");
      }
  }
}

/**************************************************************************//**
 * RSSI average.
 *
 * @param[in] rssi Measured RSSI from the receiver
 * @return :: average RSSI
 *****************************************************************************/
#define D_AVE_LENGTH 20
static float rssi_average(float r)
{
  static int index = -1;
  static float buffer[D_AVE_LENGTH];
  static float sum = 0;
  float r_a = 0;
  int i = 0;
  if(index == -1){
    //initialize
    for(i = 0; i < D_AVE_LENGTH; i++){
      buffer[i] = r;
    }
    sum = r*D_AVE_LENGTH;
    index = 0;
  } else {
    sum -= buffer[index];
    buffer[index] = r;
    sum += r;
    index++;
    if(index >= D_AVE_LENGTH){
      index = 0;
    }
  }
  r_a = sum/D_AVE_LENGTH;

  //app_log("index:%d\tr:%f\tsum:%f\tr_f:%f\t\r\n", index, r, sum, r_a);

  return r_a;
}

/**************************************************************************//**
 * Convert an RSSI-value to distance in meters.
 *
 * @param[in] tx_power Reference RSSI value of the TX-device at 1.0 m distance in dBm, for example -45.0f
 * @param[in] rssi Measured RSSI from the receiver
 * @return :: distance
 *****************************************************************************/
static float app_rssi_to_distance(float tx_power, float rssi)
{
  float scalerP = 0.7;
  float expScalerP = 5.1;
  float ratio = rssi * 1.0f / tx_power;

  if (ratio <= 1.0f) {
    return(powf(ratio, 10.0f));
  } else {
    return(scalerP * powf(ratio, expScalerP) + 0.111f);
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
  //app_log("BT KEY app init\r\n");
  app_door_init();
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
/**
* @brief set node tx power.
* @param [in] power : TX power in 0.1 dBm steps.
* @return 0: success, negetive value: failure
*/
sl_status_t set_tx_power(int16_t set_power_min, int16_t set_power_max)
{
  sl_status_t sc;
  int16_t ret_min_power, ret_max_power;
  int16_t aaa,bbb,ccc,ddd,eee;
  sc = sl_bt_system_get_tx_power_setting(&aaa,&bbb,&ccc,&ddd,&eee);
  //app_log("get tx power before set:\r\nsupport_min: %d\r\nsupport_max: %d\r\nset_min: %d\r\nset_max: %d\r\nrf_path_gain: %d\r\n", aaa, bbb, ccc, ddd, eee);

  if(set_power_max > 200)
    set_power_max = 200;
  if(set_power_min < -260)
    set_power_min = -260;

  sl_bt_system_halt(1);
  sc = sl_bt_system_set_tx_power(set_power_min, set_power_max, &ret_min_power, &ret_max_power);
  sl_bt_system_halt(0);
  //app_log("set tx power:\r\nset_min: %d\r\nset_max: %d\r\nret_min: %d\r\nret_max: %d\r\n", set_power_min, set_power_max, ret_min_power, ret_max_power);
  if(sc) {
      app_log("set tx power failed 0x%04X.\n", sc);
  }

  sc = sl_bt_system_get_tx_power_setting(&aaa,&bbb,&ccc,&ddd,&eee);
  app_log("get tx power after set:\r\nsupport_min: %d\r\nsupport_max: %d\r\nset_min: %d\r\nset_max: %d\r\nrf_path_gain: %d\r\n", aaa, bbb, ccc, ddd, eee);
  return sc == 0 ? 0 : 1;
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
  bd_addr address;
  uint8_t address_type;
  uint8_t system_id[8];
  char device_name[16];

  switch (SL_BT_MSG_ID(evt->header)) {
    // -------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack command before receiving this boot event!
    case sl_bt_evt_system_boot_id:

      //set_tx_power(-26,-26);
      //sc = sl_bt_ota_set_configuration(1);
      //app_assert_status(sc);

      // Extract unique ID from BT Address.
      sc = sl_bt_system_get_identity_address(&address, &address_type);
      app_assert_status(sc);

      app_log("BD address: %02X:%02X:%02X:%02X:%02X:%02X\tType: %s\r\n",
              address.addr[5],
              address.addr[4],
              address.addr[3],
              address.addr[2],
              address.addr[1],
              address.addr[0],
              type[address_type]);

      // Pad and reverse unique ID to get System ID.
      system_id[0] = address.addr[5];
      system_id[1] = address.addr[4];
      system_id[2] = address.addr[3];
      system_id[3] = 0xFF;
      system_id[4] = 0xFE;
      system_id[5] = address.addr[2];
      system_id[6] = address.addr[1];
      system_id[7] = address.addr[0];

      sc = sl_bt_gatt_server_write_attribute_value(gattdb_system_id,
                                                   0,
                                                   sizeof(system_id),
                                                   system_id);
      app_assert_status(sc);

      //Attach address information on device name.
      snprintf(device_name, 9, "HID_%02X%02X", address.addr[1], address.addr[0]);
      sc = sl_bt_gatt_server_write_attribute_value(gattdb_device_name,
                                                   0,
                                                   8,
                                                   (const uint8_t *)device_name);
      app_assert_status(sc);

      // Bit 0,Bonding requires authentication (Man-in-the-Middle protection)
      // Bit 1,Encryption requires bonding. Note that this setting will also enable bonding.
      // Bit 2,Secure connections only
      // No Input and No Output
      if(GPIO_PinInGet(SL_SIMPLE_BUTTON_BTN1_PORT, SL_SIMPLE_BUTTON_BTN1_PIN) == 0){
        app_log("Connect with bonded only!\r\n");
        sc = sl_bt_sm_configure(0x1F,sm_io_capability_displayonly);
      }else{
        app_log("Connect with not only bonded!\r\n");
        sc = sl_bt_sm_configure(0x0F,sm_io_capability_displayonly);
      }
      app_assert_status(sc);

      sc = sl_bt_sm_set_passkey(123456);
      app_assert_status(sc);

      // Bondings allowed
      sc = sl_bt_sm_set_bondable_mode(1);
      app_assert_status(sc);

      // Support only 1 bonding
      // New bonding will overwrite the oldest existing bonding
      sc = sl_bt_sm_store_bonding_configuration(2,1);
      app_assert_status(sc);

      // Create an advertising set.
      sc = sl_bt_advertiser_create_set(&advertising_set_handle);
      app_assert_status(sc);

      // Generate data for advertising
      sc = sl_bt_legacy_advertiser_generate_data(advertising_set_handle,
                                                 sl_bt_advertiser_general_discoverable);
      app_assert_status(sc);

      // Set advertising interval to 100ms.
      sc = sl_bt_advertiser_set_timing(
        advertising_set_handle,
        160, // min. adv. interval (milliseconds * 1.6)
        160, // max. adv. interval (milliseconds * 1.6)
        0,   // adv. duration
        0);  // max. num. adv. events
      app_assert_status(sc);
      // Start advertising and enable connections.
      sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                         sl_bt_advertiser_connectable_scannable);
      app_assert_status(sc);
      break;

    // -------------------------------
    // This event indicates that a new connection was opened.
    case sl_bt_evt_connection_opened_id:
      connection_handle = evt->data.evt_connection_opened.connection;
      bonding_handle = evt->data.evt_connection_opened.bonding;
      app_log("Connection opened, connection handle:%d, bonding handle:%d\r\n", connection_handle, bonding_handle);

      sl_bt_sm_increase_security(evt->data.evt_connection_opened.connection);

      address_direct = evt->data.evt_connection_opened.address;
      app_log("Remote device address: %02X:%02X:%02X:%02X:%02X:%02X\tType: %s\r\n",
                                address_direct.addr[5],
                                address_direct.addr[4],
                                address_direct.addr[3],
                                address_direct.addr[2],
                                address_direct.addr[1],
                                address_direct.addr[0],
                                type[evt->data.evt_connection_opened.address_type]);

      // Stop timer.
      sc = sl_simple_timer_stop(&app_adv_timer);
      app_assert_status(sc);

      // Start timer used for periodic indications.
      sc = sl_simple_timer_start(&app_periodic_timer,
                                 50,
                                 app_periodic_timer_cb,
                                 NULL,
                                 true);
      app_assert_status(sc);

      break;

    // -------------------------------
    // This event indicates that a connection was closed.
    case sl_bt_evt_connection_closed_id:
      app_log("connection closed: 0x%04X\r\n", evt->data.evt_connection_closed.reason);

      // Stop timer.
      sc = sl_simple_timer_stop(&app_periodic_timer);
      app_assert_status(sc);

      connection_handle = 0xff;
      bonding_handle = 0xff;
      connection_security = 0;

      // Generate data for advertising
      sc = sl_bt_legacy_advertiser_generate_data(advertising_set_handle,
                                               sl_bt_advertiser_general_discoverable);
      app_assert_status(sc);

      #if D_DIRECTED_ADV
      app_log("Directed ADV...\r\n");
      bd_addr target_addr = address_direct;// address_direct, address_peer;
      app_log("Client BD address: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
              target_addr.addr[5],
              target_addr.addr[4],
              target_addr.addr[3],
              target_addr.addr[2],
              target_addr.addr[1],
              target_addr.addr[0]);
      sc = sl_bt_legacy_advertiser_start_directed(
          advertising_set_handle,
          sl_bt_legacy_advertiser_low_duty_directed_connectable,//sl_bt_legacy_advertiser_low_duty_directed_connectable
          target_addr,
          1
          );
      #else
      // Restart advertising after client has disconnected.
      sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                       sl_bt_advertiser_connectable_scannable);
      #endif
      app_assert_status(sc);

      // Start timer used for periodic indications.
      sc = sl_simple_timer_start(&app_adv_timer,
                                 1280,
                                 app_adv_timer_cb,
                                 NULL,
                                 true);
      app_assert_status(sc);

      break;

    case sl_bt_evt_sm_passkey_display_id:
      // Display passkey
      app_log("Passkey: %4lu\r\n", evt->data.evt_sm_passkey_display.passkey);
      //passkey = evt->data.evt_sm_passkey_display.passkey;
      //state = DISPLAY_PASSKEY;
      break;

    case sl_bt_evt_sm_bonded_id:
      bonding_handle = evt->data.evt_sm_bonded.bonding;
      sc = sl_bt_sm_get_bonding_details(bonding_handle, &address_peer, &address_type, &security_mode, &key_size);
      app_assert_status(sc);
      app_log("Bonded, handle:%d\taddress: %02X:%02X:%02X:%02X:%02X:%02X\tType: %s\tmode:%d\tkey size:%d\r\n",
                                evt->data.evt_sm_bonded.bonding,
                                address_peer.addr[5],
                                address_peer.addr[4],
                                address_peer.addr[3],
                                address_peer.addr[2],
                                address_peer.addr[1],
                                address_peer.addr[0],
                                type[address_type],
                                security_mode,
                                key_size);
      app_log("Successful bonding\r\n");
      //sl_bt_sm_list_all_bondings();

      break;

    case  sl_bt_evt_sm_bonding_failed_id:

      app_log("Bonding failed, reason: 0x%2X\r\n", evt->data.evt_sm_bonding_failed.reason);
      /* Previous bond is broken, delete it and close connection, host must retry at least once */
      sc = sl_bt_sm_delete_bondings();
      app_assert_status(sc);
      break;
	  
//    case sl_bt_evt_sm_list_bonding_entry_id:
//      address_peer = evt->data.evt_sm_list_bonding_entry.address;
//      app_log("Bonding address: %02X:%02X:%02X:%02X:%02X:%02X\tType: %s\thandle: %d\r\n",
//              address_peer.addr[5],
//              address_peer.addr[4],
//              address_peer.addr[3],
//              address_peer.addr[2],
//              address_peer.addr[1],
//              address_peer.addr[0],
//              type[evt->data.evt_sm_list_bonding_entry.address_type],
//              evt->data.evt_sm_list_bonding_entry.bonding);
//      break;
//
//    case sl_bt_evt_sm_list_all_bondings_complete_id:
//      app_log("List bonding complete\r\n");
//      break;

    case sl_bt_evt_sm_confirm_bonding_id:
      app_log("sl_bt_evt_sm_confirm_bonding_id\r\n");
      sc = sl_bt_sm_bonding_confirm(connection_handle,1);
      if(sc)
        app_log("bonding confirm error: 0x%04X\r\n", sc);
      break;

    case  sl_bt_evt_system_external_signal_id:
      app_log("Remove bonded device!\r\n");
      app_authenticated = 0;
      sc = sl_bt_nvm_save(APP_AUTHENTICATION_STATUS_KEY, 1, &app_authenticated);
      if(sc != SL_STATUS_OK){
        app_log("nvm save error:0x%04X!\r\n", sc);
      }
      sl_bt_sm_delete_bondings();
      break;

    case sl_bt_evt_connection_parameters_id:
      connection_security = evt->data.evt_connection_parameters.security_mode;

      switch (evt->data.evt_connection_parameters.security_mode)
      {
      case connection_mode1_level1:
        app_log("No Security\r\n");
        break;
      case connection_mode1_level2:
        app_log("Unauthenticated pairing with encryption (Just Works)\r\n");
        break;
      case connection_mode1_level3:
        app_log("Authenticated pairing with encryption (Legacy Pairing)\r\n");
        break;
      case connection_mode1_level4:
        app_log("Authenticated Secure Connections pairing with encryption (BT 4.2 LE Secure Pairing)\r\n");
        break;
      default:
        break;
      }

      if(connection_security > connection_mode1_level1){
          size_t len;
          sc = sl_bt_nvm_load(APP_AUTHENTICATION_STATUS_KEY,1,&len, &app_authenticated);
          if(sc == SL_STATUS_OK){
            if(app_authenticated == 1){
                app_log("Authenticated!\r\n");
            }
          }else{
              app_log("nvm load error:0x%04X!\r\n", sc);
          }
      }
      break;

    case sl_bt_evt_connection_phy_status_id:
      app_log("phy: 0x%02X\r\n", evt->data.evt_connection_phy_status.phy);
      break;

    case sl_bt_evt_connection_remote_used_features_id:
      app_log("remote used features, connection: 0x%02X\r\n", evt->data.evt_connection_remote_used_features.connection);
      break;

    case sl_bt_evt_gatt_mtu_exchanged_id:
      app_log("mtu: 0x%04X\r\n", evt->data.evt_gatt_mtu_exchanged.mtu);
      break;

    case sl_bt_evt_gatt_server_attribute_value_id:
      //app_log("gatt_server_attribute_value, connection: 0x%04X\r\n", evt->data.evt_gatt_server_attribute_value.connection);
      break;

    case sl_bt_evt_gatt_server_user_write_request_id:
      app_log("sl_bt_evt_gatt_server_user_write_request_id, len:%d\r\n", evt->data.evt_gatt_server_user_write_request.value.len);
      for(int i = 0; i < evt->data.evt_gatt_server_user_write_request.value.len; i++){
        app_log("%02X ", evt->data.evt_gatt_server_user_write_request.value.data[i]);
      }
      app_log("\r\n");


      if(evt->data.evt_gatt_server_user_write_request.characteristic == gattdb_key_value){
        sl_bt_gatt_server_send_user_write_response(evt->data.evt_gatt_server_user_write_request.connection, evt->data.evt_gatt_server_user_write_request.characteristic, SL_STATUS_OK);

        if((connection_security >= connection_mode1_level2)&&(connection_security <= connection_mode1_level4)){
          if(APP_KEY_SIZE == evt->data.evt_gatt_server_user_write_request.value.len){
            if(memcmp(app_key, evt->data.evt_gatt_server_user_write_request.value.data, APP_KEY_SIZE) == 0){
              app_log("Authentication completed!\r\n Lock control enable!\r\n");


              app_authenticated = 1;
              sc = sl_bt_nvm_save(APP_AUTHENTICATION_STATUS_KEY, 1, &app_authenticated);
              if(sc != SL_STATUS_OK){
                app_log("nvm save error:0x%04X!\r\n", sc);
              }
              break;
            }
          }
        }
      }


      //sl_bt_sm_delete_bondings();
      //sl_bt_connection_close(connection_handle);

      app_log("Authentication failed!\r\n Lock control Disable!\r\n");
      //app_log("Delete bonding information!\r\n");
      //app_log("Connection close!\r\n\r\n\r\n\r\n");

      break;

    case sl_bt_evt_gatt_server_characteristic_status_id:

      break;

    case sl_bt_evt_connection_rssi_id:
      //You can check that printf float is enabled for your build configuration
      //by right clicking on your project in the [Project Explorer] window,
      //selecting [C/C++ Build]>[Settings], then under [GNU ARM C Linker]>[General],
      //check the [Printf float] checkbox beneath the [C Library] selection box
      //app_log("rssi: %d\t", evt->data.evt_connection_rssi.rssi);
      rssi_value = rssi_average(evt->data.evt_connection_rssi.rssi);
      distance = app_rssi_to_distance(-49.0f,rssi_value);
      //app_log("RSSI: %f, Distance: %f meter(s)\r\n", rssi_value, distance);

      if(distance <= D_THRESHOLD_UNLOCK){
          lock_state_target = D_UNLOCK;
      }else if(distance >= D_THRESHOLD_LOCK){
          lock_state_target = D_LOCK;
      }

      app_door();

      break;

    case sl_bt_evt_system_error_id:
      app_log("error: %04X\r\n", evt->data.evt_system_error.reason);
      break;

    ///////////////////////////////////////////////////////////////////////////
    // Add additional event handlers here as your application requires!      //
    ///////////////////////////////////////////////////////////////////////////

    // -------------------------------
    // Default event handler.
    default:
      app_log("BLE unhandled evt: %8.8x class %2.2x method %2.2x\r\n", evt->header, (evt->header >> 16) & 0xFF, (evt->header >> 24) & 0xFF);
      break;
  }
}
