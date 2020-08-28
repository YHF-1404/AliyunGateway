/**
  ******************************************************************************
  * @file    spi_master_use.h
  * @author  YHF
  * @brief   This file contains SPI config
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 YHF.
  * All rights reserved.</center></h2>
  *
  ******************************************************************************
  */
#ifndef __SPI_MASTER_USE_H
#define __SPI_MASTER_USE_H
#ifdef __cplusplus
 extern "C" {
#endif
#include "driver/spi_master.h"
#include "NRF24L01_MAL_nrf.h"

//extern spi_device_handle_t  H_SPI_2;


void SPI_Init(void);
#ifdef __cplusplus
} 
#endif
#endif /*__SPI_MASTER_USE_H*/



/************************ (C) COPYRIGHT YHF *****END OF FILE****/
