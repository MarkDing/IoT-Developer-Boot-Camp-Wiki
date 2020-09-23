/***************************************************************************//**
 * @file sl_connect_ota_unicast_bootloader_test.c
 * @brief APIs for the sl_ota-unicast-bootloader-test plugin.
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
#include "sl_ota_bootloader_test_common.h"
#include "sl_btl-interface.h"
#include "sl_cli.h"
#include "sl_flex_assert.h"

#if defined(SL_CATALOG_CONNECT_OTA_UNICAST_BOOTLOADER_SERVER_PRESENT)
#include SL_CATALOG_CONNECT_OTA_UNICAST_BOOTLOADER_SERVER
#endif // SL_CATALOG_CONNECT_OTA_UNICAST_BOOTLOADER_SERVER_PRESENT

#if defined(SL_CATALOG_CONNECT_OTA_UNICAST_BOOTLOADER_CLIENT_PRESENT)
#include SL_CATALOG_CONNECT_OTA_UNICAST_BOOTLOADER_CLIENT
#endif // SL_CATALOG_CONNECT_OTA_UNICAST_BOOTLOADER_CLIENT_PRESENT

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
/// Timing event for scheduling the bootload process of the received image.
EmberEventControl emAfPluginOtaUnicastBootloaderTestEventControl;

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
/// Node ID of the target
static EmberNodeId target;
/// Enable resuming an image after the timeout period.
static bool ota_resume_enable = true;
/// the image index to start/resume the download
static uint32_t unicast_download_start_index = 0;

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
/**************************************************************************//**
 * Event code (used to schedule a bootload action at the client).
 *****************************************************************************/
void emAfPluginOtaUnicastBootloaderTestEventHandler(void)
{
  emberEventControlSetInactive(emAfPluginOtaUnicastBootloaderTestEventControl);
  bootloader_flash_image();
}

#if defined(SL_CATALOG_CONNECT_OTA_UNICAST_BOOTLOADER_SERVER_PRESENT)
//------------------------------------------------------------------------------
// OTA Bootloader Server implemented callbacks.
/**************************************************************************//**
 * A callback invoked during an image distribution process to retrieve
 * a contiguous segment of the image being distributed.
 *****************************************************************************/
bool emberAfPluginOtaUnicastBootloaderServerGetImageSegmentCallback(uint32_t startIndex,
                                                                    uint32_t endIndex,
                                                                    uint8_t imageTag,
                                                                    uint8_t *imageSegment)
{
  APP_INFO("(server): get segment, start: %d, end: %d, tag: 0x%x\n",
           startIndex, endIndex, imageTag);

  //Initialize bootloader (and flash part) if not yet initialized or in shutdown.
  if ( !emberAfPluginBootloaderInterfaceIsBootloaderInitialized() ) {
    if ( !emberAfPluginBootloaderInterfaceInit() ) {
      return false;
    }
  }

  if ( !emberAfPluginBootloaderInterfaceRead(startIndex, endIndex - startIndex + 1, imageSegment) ) {
    return false;
  }

  APP_INFO(".");

  return true;
}

/**************************************************************************//**
 * A callback invoked when the image distribution process is terminated.
 *****************************************************************************/
void emberAfPluginOtaUnicastBootloaderServerImageDistributionCompleteCallback(EmberAfOtaUnicastBootloaderStatus status)
{
  APP_INFO("image distribution completed, 0x%x\n", status);
  OtaUnicastDistributionCompleteCallback();
}

/**************************************************************************//**
 * A callback invoked when a bootload request process has completed.
 *****************************************************************************/
void emberAfPluginOtaUnicastBootloaderServerRequestTargetBootloadCompleteCallback(EmberAfOtaUnicastBootloaderStatus status)
{
  APP_INFO("bootload request completed, 0x%x\n", status);
}

#endif // SL_CATALOG_CONNECT_OTA_UNICAST_BOOTLOADER_SERVER_PRESENT

//------------------------------------------------------------------------------
// OTA Unicast Bootloader Client implemented callbacks.
#if defined(SL_CATALOG_CONNECT_OTA_UNICAST_BOOTLOADER_CLIENT_PRESENT)
/**************************************************************************//**
 * A callback invoked when the client starts receiving a new image.
 *****************************************************************************/
bool emberAfPluginOtaUnicastBootloaderClientNewIncomingImageCallback(EmberNodeId serverId,
                                                                     uint8_t imageTag,
                                                                     uint32_t imageSize,
                                                                     uint32_t *startIndex)
{
  (void)serverId;
  (void)imageSize;
  // The client shall accept images with matching tag
  bool accept = (imageTag == ota_bootloader_test_image_tag);

  if (ota_resume_enable) {
    if (ota_resume_start_counter_reset) {
      *startIndex = 0;
      ota_resume_start_counter_reset = false;
    } else {
      *startIndex = unicast_download_start_index;
    }
  }
  APP_INFO("new incoming unicast image %s (tag=0x%x)\n",
           ((accept) ? "ACCEPTED" : "REFUSED"),
           imageTag);

  return accept;
}

/**************************************************************************//**
 * A callback invoked when an image segment that is part of an image the
 * application chose to download was received.
 *****************************************************************************/
void emberAfPluginOtaUnicastBootloaderClientIncomingImageSegmentCallback(EmberNodeId serverId,
                                                                         uint32_t startIndex,
                                                                         uint32_t endIndex,
                                                                         uint8_t imageTag,
                                                                         uint8_t *imageSegment)
{
  (void)serverId;
  APP_INFO("(client): incoming segment, start: %lu, end: %lu, tag: 0x%x\n",
           startIndex, endIndex, imageTag);

  //Initialize bootloader (and flash part) if not yet initialized or in shutdown.
  if ( !emberAfPluginBootloaderInterfaceIsBootloaderInitialized() ) {
    if ( !emberAfPluginBootloaderInterfaceInit() ) {
      APP_INFO("init failed\n");

      emberAfPluginOtaUnicastBootloaderClientAbortImageDownload(imageTag);

      unicast_download_start_index = 0;
      return;
    }
  }

  if ( !emberAfPluginBootloaderInterfaceWrite(startIndex,
                                              endIndex - startIndex + 1,
                                              imageSegment) ) {
    APP_INFO("write failed\n");

    emberAfPluginOtaUnicastBootloaderClientAbortImageDownload(imageTag);

    unicast_download_start_index = 0;
    return;
  }

  unicast_download_start_index = endIndex + 1;

  APP_INFO(".");
}

/**************************************************************************//**
 * A callback invoked to indicate that an image download has completed.
 *****************************************************************************/
void emberAfPluginOtaUnicastBootloaderClientImageDownloadCompleteCallback(EmberAfOtaUnicastBootloaderStatus status,
                                                                          uint8_t imageTag,
                                                                          uint32_t imageSize)
{
  if (status == EMBER_OTA_UNICAST_BOOTLOADER_STATUS_SUCCESS) {
    APP_INFO("Image download COMPLETED tag=0x%x size=%lu\n",
             imageTag, imageSize);
    unicast_download_start_index = 0;
  } else {
    APP_INFO("Image download FAILED status=0x%x\n", status);
  }
}

/**************************************************************************//**
 * A callback invoked to indicate that a server has requested to perform a
 * bootload operation at a certain point in time in the future.
 *****************************************************************************/
bool emberAfPluginOtaUnicastBootloaderClientIncomingRequestBootloadCallback(EmberNodeId serverId,
                                                                            uint8_t imageTag,
                                                                            uint32_t bootloadDelayMs)
{
  // The client shall bootload an image with matching tag.

  // TODO: we should also maintain a state machine to keep track whether this
  // image was previously correctly received. For now we assume the server would
  // issue a bootload request only if the client reported a successful image
  // download.
  (void)serverId;
  bool accept = (imageTag == ota_bootloader_test_image_tag);

  if (accept) {
    APP_INFO("bootload request for image with tag 0x%x accepted, will bootload in %lu ms\n",
             imageTag, bootloadDelayMs);
    // Schedule a bootload action.
    emberEventControlSetDelayMS(emAfPluginOtaUnicastBootloaderTestEventControl,
                                bootloadDelayMs);
  } else {
    APP_INFO("bootload request refused (tag 0x%x doesn't match)\n",
             imageTag);
  }

  return accept;
}

#endif // SL_CATALOG_CONNECT_OTA_UNICAST_BOOTLOADER_CLIENT_PRESENT

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * Brief description of my_public_function().
 *****************************************************************************/
void cli_bootloader_unicast_set_target(sl_cli_command_arg_t *arguments)
{
  EmberNodeId target_id = sl_cli_get_argument_uint16(arguments, 0);

  target = target_id;
  APP_INFO("unicast target set\n");
}

/**************************************************************************//**
 * Brief description of my_public_function().
 *****************************************************************************/
void cli_bootloader_unicast_unicast_distribute(sl_cli_command_arg_t *arguments)
{
#if defined(SL_CATALOG_CONNECT_OTA_UNICAST_BOOTLOADER_SERVER_PRESENT)
  uint32_t image_size = sl_cli_get_argument_uint32(arguments, 0);
  uint8_t image_tag = sl_cli_get_argument_uint8(arguments, 1);

  EmberAfOtaUnicastBootloaderStatus status =
    emberAfPluginOtaUnicastBootloaderServerInitiateImageDistribution(target,
                                                                     image_size,
                                                                     image_tag);
  if (status == EMBER_OTA_UNICAST_BOOTLOADER_STATUS_SUCCESS) {
    APP_INFO("unicast image distribution initiated\n");
  } else {
    APP_INFO("unicast image distribution failed 0x%x\n", status);
  }
#else
  (void)arguments;
  APP_INFO("OTA unicast bootloader server plugin not included\n");
#endif // SL_CATALOG_CONNECT_OTA_UNICAST_BOOTLOADER_SERVER_PRESENT
}

/**************************************************************************//**
 * Brief description of my_public_function().
 *****************************************************************************/
void cli_bootloader_unicast_request_bootload(sl_cli_command_arg_t *arguments)
{
#if defined(SL_CATALOG_CONNECT_OTA_UNICAST_BOOTLOADER_SERVER_PRESENT)
  uint32_t delay_ms = sl_cli_get_argument_uint32(arguments, 0);
  uint8_t image_tag = sl_cli_get_argument_uint8(arguments, 1);

  EmberAfOtaUnicastBootloaderStatus status =
    emberAfPluginUnicastBootloaderServerInitiateRequestTargetBootload(delay_ms,
                                                                      image_tag,
                                                                      target);
  if (status == EMBER_OTA_UNICAST_BOOTLOADER_STATUS_SUCCESS) {
    APP_INFO("bootload request initiated\n");
  } else {
    APP_INFO("bootload request failed 0x%x\n", status);
  }
#else
  (void)arguments;
  APP_INFO("OTA bootloader server plugin not included\n");
#endif // SL_CATALOG_CONNECT_OTA_UNICAST_BOOTLOADER_SERVER_PRESENT
}
