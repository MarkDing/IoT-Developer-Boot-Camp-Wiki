/**************************************************************************//**
 * @file
 * @brief Example using different voltage scaling levels to show influence
 * on current draw in the Profiler
 * @version 0.0.2
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2019 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silicon Labs Software License Agreement. See
 * "http://developer.silabs.com/legal/version/v11/Silicon_Labs_Software_License_Agreement.txt"
 * for details. Before using this software for any purpose, you must agree to the
 * terms of that agreement.
 *
 ******************************************************************************/

#include <stdio.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_burtc.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"

#include "mx25flash_spi.h"

// Included to define EMU_DCDCINIT_WSTK_DEFAULT initializer
#include "bsp.h"

/*
 * A JEDEC standard SPI flash boots up in standby mode in order to
 * provide immediate access, such as when used it as a boot memory.
 *
 * Typical current draw in standby mode for the MX25R8035F device used
 * on EFR32 radio boards is 5 µA, which makes  observing the difference
 * between VS2 and VS0 voltage scaling levels difficult to observe.
 *
 * Fortunately, JEDEC standard SPI flash memories have a lower current
 * deep power-down mode, which can be entered after sending the
 * relevant commands.  This is on the order of 0.007 µA for the
 * MX25R8035F, thus making the difference between between the VS2 and
 * VS0 voltage scaling modes in EM2/3 more obvious.
 */
void powerDownSpiFlash(void)
{
  FlashStatus status;

  MX25_init();
  MX25_RSTEN();
  MX25_RST(&status);
  MX25_DP();
  MX25_deinit();
}


/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
  EMU_DCDCInit_TypeDef dcdcInit = EMU_DCDCINIT_DEFAULT;

  // Chip errata
  CHIP_Init();

  // Enable DC-DC converter
  EMU_DCDCInit(&dcdcInit);

  // Power-down the radio board SPI flash
  powerDownSpiFlash();

  /* Disable Instruction Cache */
  CMU_ClockEnable(cmuClock_ICACHE, true);
  ICACHE0->CTRL |= 0x01;

  /* Disable Radio RAM memories (FRC and SEQ)*/
  CMU_ClockEnable(cmuClock_SYSCFG, true);
  SYSCFG->RADIORAMRETNCTRL = 0x103UL; // 0x103UL : power down both FRCRAM and SEQRAM
  	  	  	  	  	  	  	  	  	  // 0x100UL : power down only FRCRAM
  	  	  	  	  	  	  	  	  	  // 0x003UL : power down only SEQRAM
  	  	  	  	  	  	  	  	  	  // 0x000UL : power down none

  EMU_EM23Init_TypeDef vsInit = EMU_EM23INIT_DEFAULT;
  vsInit.vScaleEM23Voltage = emuVScaleEM23_LowPower;
  EMU_EM23Init(&vsInit);

	/* Disable MCU RAM retention */
  	// EMU_RamPowerDown(SRAM_BASE, SRAM_BASE + SRAM_SIZE);
	// Power down BLK0 0x20000000 - 0x20006000: 0x01; BLK1 0x20006000 - 0x20008000: 0x10UL
	CMU_ClockEnable(cmuClock_SYSCFG, true);
	SYSCFG->DMEM0RETNCTRL = 0x00UL; // value 0: none of the RAM blocks powered down
									// value 1: powerdown RAM block 0 (BLK0)
									// value 2: powerdown RAM block 1 (BLK1)

  /*
   * When developing/debugging code on xG22 that enters EM2 or lower,
   * it's a good idea to have an "escape hatch" type mechanism, e.g. a
   * way to pause the device so that a debugger can connect in order
   * to erase flash, among other things.
   *
   * Before proceeding with this example, make sure PB0 is not pressed.
   * If the PB0 pin is low, turn on LED0 and execute the breakpoint
   * instruction to stop the processor in EM0 and allow a debug
   * connection to be made.
   */
  CMU_ClockEnable(cmuClock_GPIO, true);
  GPIO_PinModeSet(BSP_GPIO_PB0_PORT, BSP_GPIO_PB0_PIN, gpioModeInputPullFilter, 1);

  if (GPIO_PinInGet(BSP_GPIO_PB0_PORT, BSP_GPIO_PB0_PIN) == 0)
  {
    GPIO_PinModeSet(BSP_GPIO_LED0_PORT, BSP_GPIO_LED0_PIN, gpioModePushPull, 1);
    __BKPT(0);
  }
  // Pin not asserted, so disable input
  else
  {
    GPIO_PinModeSet(BSP_GPIO_PB0_PORT, BSP_GPIO_PB0_PIN, gpioModeDisabled, 0);
    CMU_ClockEnable(cmuClock_GPIO, false);
  }

  while(1)
    EMU_EnterEM2(false);
}
