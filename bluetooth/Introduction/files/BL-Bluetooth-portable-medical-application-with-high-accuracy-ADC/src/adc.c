/***************************************************************************//**
 * @file
 * @brief ADC functions
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
 ******************************************************************************
* # Experimental Quality
* This code has not been formally tested and is provided as-is. It is not
* suitable for production environments. In addition, this code will not be
* maintained and there may be no bug maintenance planned for these resources.
* Silicon Labs may update projects from time to time.
******************************************************************************/
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_iadc.h"
#include "em_gpio.h"
#include "app_log.h"
#include "app_log.h"


/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

// Set CLK_ADC to 10MHz
#define CLK_SRC_ADC_FREQ      20000000  // CLK_SRC_ADC
#define CLK_ADC_FREQ          10000000  // CLK_ADC - 10 MHz max in normal mode

// The LSB[3:0] of each ADC sample will be a random number
// Hence the mask is defined to only select the relevant bits from an
// ADC sample
#define ADC_CGM_BIT_MASK            0x7

#define IADC_INPUT_0_BUS          ABUSALLOC
#define IADC_INPUT_0_BUSALLOC     GPIO_ABUSALLOC_AODD0_ADC0

/**************************************************************************//**
 * @brief  ADC Initializer
 *****************************************************************************/
void initADC(void)
{
  // Declare init structs
  IADC_Init_t init = IADC_INIT_DEFAULT;
  IADC_AllConfigs_t initAllConfigs = IADC_ALLCONFIGS_DEFAULT;
  IADC_InitSingle_t initSingle = IADC_INITSINGLE_DEFAULT;
  IADC_SingleInput_t initSingleInput = IADC_SINGLEINPUT_DEFAULT;
  /*
   * Enable IADC0 and GPIO clock branches.
   *
   * Note: On EFR32xG21 devices, CMU_ClockEnable() calls have no effect
   * as clocks are enabled/disabled on-demand in response to peripheral
   * requests.  Deleting such lines is safe on xG21 devices and will
   * reduce provide a small reduction in code size.
   */
  CMU_ClockEnable(cmuClock_IADC0, true);
  CMU_ClockEnable(cmuClock_GPIO, true);
  // Reset IADC to reset configuration in case it has been modified
  IADC_reset(IADC0);
  // Select clock for IADC
  CMU_ClockSelectSet(cmuClock_IADCCLK, cmuSelect_FSRCO);

  // Modify init structures and initialize
  init.warmup = iadcWarmupKeepWarm;

  /*
   * Configuration 0 is used by both scan and single conversions by
   * default.  Use internal bandgap as the reference and specify the
   * reference voltage in mV.
   *
   * Resolution is not configurable directly but is based on the
   * selected oversampling ratio (osrHighSpeed), which defaults to
   * 2x and generates 12-bit results.
   *
   * Set oversampling rate to 32x; digital averaging to 16x
   * resolution formula res = 11 + log2(oversampling * digital averaging)
   * in this case res = 11 + log2(32 * 16) = 20
   *
   */
  initAllConfigs.configs[0].reference = iadcCfgReferenceInt1V2;
  initAllConfigs.configs[0].vRef = 1210;
  initAllConfigs.configs[0].osrHighSpeed = iadcCfgOsrHighSpeed32x;
  initAllConfigs.configs[0].analogGain = iadcCfgAnalogGain0P5x;
  initAllConfigs.configs[0].digAvg = iadcDigitalAverage16;

  // Divides CLK_SRC_ADC to set the CLK_ADC frequency
  // Default oversampling (OSR) is 2x, and Conversion Time = ((4 * OSR) + 2) / fCLK_ADC
  initAllConfigs.configs[0].adcClkPrescale = IADC_calcAdcClkPrescale(
                                                           IADC0,
                                                           CLK_ADC_FREQ,
                                                           0,
                                                           iadcCfgModeNormal,
                                                           init.srcClkPrescale);
  // Set alignment to right justified with 20 bits for data field
  initSingle.alignment = iadcAlignRight20;

  // Configure Input sources for single ended conversion
  initSingleInput.posInput = iadcPosInputAvdd;
  initSingleInput.negInput = iadcNegInputGnd;


  // Initialize IADC
  IADC_init(IADC0, &init, &initAllConfigs);

  // Initialize Single
  IADC_initSingle(IADC0, &initSingle, &initSingleInput);

  app_log_info("init ADC\n");
}

/**************************************************************************//**
 * @brief
 *    poll signle adc result
 * @param[in]
 *    none
 * @return
 *    iadc value
 *****************************************************************************/
uint8_t adcPollResult(void)
{
  volatile uint32_t sample;
  uint8_t ret;
  volatile double singleResult;

  /* start converting */
  IADC_command(IADC0, iadcCmdStartSingle);

  /* Wait for conversion to be complete
  ** while combined status bits 8 & 6 don't equal 1 and 0 respectively
  ** */
  while((IADC0->STATUS &
        (_IADC_STATUS_CONVERTING_MASK | _IADC_STATUS_SINGLEFIFODV_MASK))
        != IADC_STATUS_SINGLEFIFODV) ;

  /* Read data from the FIFO */
  sample = IADC_pullSingleFifoResult(IADC0).data;
  /*20-bit result - "singleResult" is obtained by the formula: sample*VREF/(2^20)*/
  /*This corresponds to sampling frequency of roughly 4.8 KHz*/
  singleResult = (sample * 1.21 * 2) / 0xFFFFF;

  uint32_t valueInt, valueFram;
  valueInt = (uint32_t) (singleResult * 1000);
  valueFram = (uint32_t) ((singleResult * 1000 - valueInt) * 1000);

  ret = valueFram & ADC_CGM_BIT_MASK;
  if(ret < 3){
      ret ++;
  }

  return ret;
}
