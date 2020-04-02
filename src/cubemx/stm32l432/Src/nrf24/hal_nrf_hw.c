/*
 * Copyright (c) 2006 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is confidential property of Nordic Semiconductor. The use,
 * copying, transfer or disclosure of such information is prohibited except by express written
 * agreement with Nordic Semiconductor.
 *
 */
/** @file
 *  C-file defining the hardware depenent interface of the nRF24l01.
 *
 *  @author Jon Helge Nistad
 *
 */

//#include "main.h"
#include "spi.h"

//extern SPI_HandleTypeDef hspi1;

uint8_t hal_nrf_rw(uint8_t value)
{
  uint8_t RxData;
  HAL_StatusTypeDef errcode;

  errcode = HAL_SPI_TransmitReceive(&hspi1, &value, &RxData, 1, 1000);
  //HAL_SPI_TransmitReceive(&hspi1, &value, &RxData, 1, 1000);
	if (errcode) { 
    printf("tx:%02X, rx:%02X (err:%d) s:%d e:%ld\r\n", value, RxData, errcode, 
			HAL_SPI_GetState(&hspi1), HAL_SPI_GetError(&hspi1));
	}

  return RxData;
}
