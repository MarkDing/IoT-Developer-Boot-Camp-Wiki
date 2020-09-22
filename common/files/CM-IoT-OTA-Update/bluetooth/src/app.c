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
#include "sl_simple_button_instances.h"
#include "sl_simple_button.h"

// The advertising set handle allocated from Bluetooth stack.
static uint8_t advertising_set_handle = 0xff;

// These variables are used for the connection handle and the notification parameters to send to the OTA Server
uint8_t conn_handle;
uint8_t notification_data[1] = {0};
uint16_t notification_len = 0;
uint8_t notifyEnabled = false;

static BootloaderInformation_t bldInfo;
static BootloaderStorageSlot_t slotInfo;

/* OTA variables */
static uint32_t ota_image_position = 0;
static uint8_t ota_in_progress = 0;
static uint8_t ota_image_finished = 0;
static uint16_t ota_time_elapsed = 0;

extern const sl_led_t sl_led_led0;
extern sl_simple_button_context_t simple_btn0_context;

void sl_button_on_change(const sl_button_t *handle)
{
  sl_simple_button_context_t *ctxt = ((sl_simple_button_context_t *)handle[0].context);
  if (ctxt->state) {
      ctxt->history += 1;
      sl_bt_external_signal(1);
  }
}

static int32_t get_slot_info()
{
  int32_t err;

  bootloader_getInfo(&bldInfo);
  sl_app_log("Gecko bootloader version: %u.%u\r\n", (bldInfo.version & 0xFF000000) >> 24, (bldInfo.version & 0x00FF0000) >> 16);

  err = bootloader_getStorageSlotInfo(0, &slotInfo);

  if(err == BOOTLOADER_OK)
  {
      sl_app_log("Slot 0 starts @ 0x%8.8x, size %u bytes\r\n", slotInfo.address, slotInfo.length);
  }
  else
  {
      sl_app_log("Unable to get storage slot info, error %x\r\n", err);
  }

  return(err);
}

static void erase_slot_if_needed()
{
  uint32_t offset = 0;
  uint8_t buffer[256];
  int i;
  int dirty = 0;
  int32_t err = BOOTLOADER_OK;
  int num_blocks = 0;

  /* check the download area content by reading it in 256-byte blocks */

  num_blocks = slotInfo.length / 256;

  while((dirty == 0) && (offset < 256*num_blocks) && (err == BOOTLOADER_OK))
  {
    err = bootloader_readStorage(0, offset, buffer, 256);
    if(err == BOOTLOADER_OK)
    {
      i=0;
      while(i<256)
      {
        if(buffer[i++] != 0xFF)
        {
          dirty = 1;
          break;
        }
      }
      offset += 256;
    }
    sl_app_log(".");
  }

  if(err != BOOTLOADER_OK)
  {
      sl_app_log("error reading flash! %x\r\n", err);
  }
  else if(dirty)
  {
      sl_app_log("download area is not empty, erasing...\r\n");
    bootloader_eraseStorageSlot(0);
    sl_app_log("done\r\n");
  }
  else
  {
      sl_app_log("download area is empty\r\n");
  }

  return;
}

static void print_progress()
{
  // estimate transfer speed in kbps
  int kbps = ota_image_position*8/(1024*ota_time_elapsed);

  sl_app_log("pos: %u, time: %u, kbps: %u\r\n", ota_image_position, ota_time_elapsed, kbps);
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
  /* bootloader init must be called before calling other bootloader_xxx API calls */
  bootloader_init();

  /* read slot information from bootloader */
  if(get_slot_info() == BOOTLOADER_OK)
  {
    /* the download area is erased here (if needed), prior to any connections are opened */
    erase_slot_if_needed();
  }
  else
  {
      sl_app_log("Check that you have installed correct type of Gecko bootloader!\r\n");
  }
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

  switch (SL_BT_MSG_ID(evt->header)) {
    // -------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack command before receiving this boot event!
    case sl_bt_evt_system_boot_id:

      // Extract unique ID from BT Address.
      sc = sl_bt_system_get_identity_address(&address, &address_type);
      sl_app_assert(sc == SL_STATUS_OK,
                    "[E: 0x%04x] Failed to get Bluetooth address\n",
                    (int)sc);

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
      sl_app_assert(sc == SL_STATUS_OK,
                    "[E: 0x%04x] Failed to write attribute\n",
                    (int)sc);

      // Create an advertising set.
      sc = sl_bt_advertiser_create_set(&advertising_set_handle);
      sl_app_assert(sc == SL_STATUS_OK,
                    "[E: 0x%04x] Failed to create advertising set\n",
                    (int)sc);

      // Set advertising interval to 100ms.
      sc = sl_bt_advertiser_set_timing(
        advertising_set_handle,
        160, // min. adv. interval (milliseconds * 1.6)
        160, // max. adv. interval (milliseconds * 1.6)
        0,   // adv. duration
        0);  // max. num. adv. events
      sl_app_assert(sc == SL_STATUS_OK,
                    "[E: 0x%04x] Failed to set advertising timing\n",
                    (int)sc);
      // Start general advertising and enable connections.
      sc = sl_bt_advertiser_start(
        advertising_set_handle,
        advertiser_general_discoverable,
        advertiser_connectable_scannable);
      sl_app_assert(sc == SL_STATUS_OK,
                    "[E: 0x%04x] Failed to start advertising\n",
                    (int)sc);


      break;

      // -------------------------------
      // This event indicates that a new connection was opened.
      case sl_bt_evt_connection_opened_id:

        sl_app_log("connection opened\r\n");
        conn_handle = evt->data.evt_connection_opened.connection;
        sl_app_log("Connection handle: %d\r\n", conn_handle);
        //set timeout to 100 msec, because erasing the slot may take several seconds!!!
        sc = sl_bt_connection_set_parameters(conn_handle, 6, 6, 0, 10, 0, 0xffff);
        break;

    // -------------------------------
    // This event indicates that a connection was closed.
    case sl_bt_evt_connection_closed_id:
      sl_app_log("connection closed, reason: 0x%2.2x\r\n", evt->data.evt_connection_closed.reason);

      if (ota_image_finished) {
          sl_app_log("Installing new image\r\n");
        bootloader_setImageToBootload(0);
        bootloader_rebootAndInstall();
      } else {
          // Restart advertising after client has disconnected.
          sc = sl_bt_advertiser_start(
            advertising_set_handle,
            advertiser_general_discoverable,
            advertiser_connectable_scannable);
          sl_app_assert(sc == SL_STATUS_OK,
                        "[E: 0x%04x] Failed to start advertising\n",
                        (int)sc);
      }
      break;

    ///////////////////////////////////////////////////////////////////////////
    // Add additional event handlers here as your application requires!      //
    ///////////////////////////////////////////////////////////////////////////

    case sl_bt_evt_system_soft_timer_id:
      if(ota_in_progress)
      {
        ota_time_elapsed++;
        print_progress();
      }
      break;

      /* write operations to ota_data, ot_control characteristics are handled here :*/
    case sl_bt_evt_gatt_server_user_write_request_id:
    {
      uint32_t connection = evt->data.evt_gatt_server_user_write_request.connection;
      uint32_t characteristic = evt->data.evt_gatt_server_user_write_request.characteristic;
//      sl_app_log("sl_bt_evt_gatt_server_user_write_request_id\r\n");

      if(characteristic == gattdb_ota_control)
      {
        switch(evt->data.evt_gatt_server_user_write_request.value.data[0])
        {
          case 0://Erase and use slot 0
              ota_image_position=0;
              ota_in_progress=1;
              sl_app_log("bleOtaDfu Transaction Begin\r\n");
            break;

          case 3://END OTA process
              //wait for connection close and then reboot
              ota_in_progress=0;
              ota_image_finished=1;
              sl_app_log("upload finished. received file size %u bytes\r\n", ota_image_position);
              sl_led_turn_off(&sl_led_led0);
              sl_bt_connection_close(connection);
            break;

          default:
              sl_bt_connection_close(connection);
            break;
        }
        sl_bt_gatt_server_send_user_write_response(connection,characteristic,0);
      }

      else if(characteristic == gattdb_ota_data)
      {
        if(ota_in_progress)
        {
          bootloader_writeStorage(0,//use slot 0
              ota_image_position,
            evt->data.evt_gatt_server_user_write_request.value.data,
            evt->data.evt_gatt_server_user_write_request.value.len);

            sl_app_log("Receive %d bytes, total length %d\r\n", evt->data.evt_gatt_server_user_write_request.value.len, ota_image_position);
            ota_image_position+=evt->data.evt_gatt_server_user_write_request.value.len;
        }

      }
//      sl_bt_gatt_server_send_user_write_response(connection,characteristic,0);
    }
      break;

    case  sl_bt_evt_gatt_server_characteristic_status_id:
      if ((evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_ota_control)
          && (evt->data.evt_gatt_server_characteristic_status.status_flags == 0x01)) {
           if (evt->data.evt_gatt_server_characteristic_status.client_config_flags == 0x00) {
                 notifyEnabled = false;
           }
           else {
              notifyEnabled = true;
//              sl_bt_gatt_send_characteristic_confirmation(evt->data.evt_gatt_server_characteristic_status.connection);
           }

        }
        break;

      // external signal indication (come from the interrupt handler)
      case sl_bt_evt_system_external_signal_id:

        conn_handle = evt->data.evt_system_external_signal.extsignals;

        if (notifyEnabled){
            if (evt->data.evt_system_external_signal.extsignals == 1){
               notification_data[0] = 0xA5;
               sc = sl_bt_gatt_server_send_characteristic_notification(conn_handle, gattdb_ota_control, sizeof(notification_data), notification_data, &notification_len);
               sl_app_log("OTA request: 0x%04x\r\n", notification_data[0]);
               sl_led_turn_on(&sl_led_led0);
            }
        }

      break;
    // -------------------------------
    // Default event handler.
    default:
      break;
  }
}
