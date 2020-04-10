/* Copyright (c) 2007 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT. 
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRENTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 * $LastChangedRevision: 2132 $
 */ 

/** @file
 * Header file defining the hardware depenent interface of the C8051F320
 *
 *
 */
#include "main.h"

#ifndef HAL_NRF_HW_H__
#define HAL_NRF_HW_H__

//GPIO_InitTypeDef  GPIO_InitStruct;

/** Macro that set radio's CSN line LOW.
 *
 */
#define CSN_LOW() \
  do { HAL_GPIO_WritePin(CSN_GPIO_Port, CSN_Pin, 0);} while(0)

/** Macro that set radio's CSN line HIGH.
 *
 */
#define CSN_HIGH() \
  do { HAL_GPIO_WritePin(CSN_GPIO_Port, CSN_Pin, 1);} while(0)

/** Macro that set radio's CE line LOW.
 *
 */
#define CE_LOW()\
  do { HAL_GPIO_WritePin(CE_GPIO_Port, CE_Pin, 0);} while(0)

/** Macro that set radio's CE line HIGH.
 *
 */
#define CE_HIGH() \
  do { HAL_GPIO_WritePin(CE_GPIO_Port, CE_Pin, 1);} while(0)

/**
 * Pulses the CE to nRF24L01 for at least 10 us // rewrite the routine using the timer. for 10 micro second
 */
 //call timer init and define it somewhre, call the function here.
#if 0 
#define CE_PULSE() do { \
  CE_HIGH();\
  timerDelay(10);\
  CE_LOW();  \
  } while(0)
#endif

// XXX FIXME
#if 1 
#define CE_PULSE() do { \
  CE_HIGH();\
  HAL_Delay(1);\
  CE_LOW();  \
  } while(0)
#endif

#if 0 
#define CE_PULSE() do { \
  uint8_t count = 100; \
  CE_HIGH(); \
  while(count--) \
   ; \
  CE_LOW();  \
  } while(0)
#endif

/** Basis function, nrf_rw
 * This function is used by the basis functions
 * to exchange data with the data.
 *
 * @param value Databyte to write
 *
 * @return Databyte from radio.
*/
uint8_t hal_nrf_rw(uint8_t value);

#endif /* HAL_NRF_HW_H__ */
