/**
  ******************************************************************************
  * @file    NRF24L01_MAL_nrf.h
  * @author  YHF
  * @brief   This file contains NRF24L01 common defines, enumeration, macros and 
  *          structures definitions. 
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 YHF.
  * All rights reserved.</center></h2>
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __NRF24L01_MAL_NRF
#define __NRF24L01_MAL_NRF

#ifdef __cplusplus
 extern "C" {
#endif
#include "NRF24L01_MAL_def.h"

#include "stdint.h"
#include "stddef.h"
#include "driver/spi_master.h"
typedef struct 
{
  /* data */
  uint8_t CONFIG;                 //CONFIG寄存器
  uint8_t EN_AA;                  //自动应答使能寄存器 EN_AA
  uint8_t SETUP_AW;               //通信地址长度
  uint8_t EN_RX_ADDR;             //RX地址使能寄存器
  uint8_t SETUP_RETR;             //自动重发设置寄存器
  uint8_t RF_CH;                  //射频频率设置寄存器
  uint8_t RF_SETUP;               //射频配置寄存器


}NRF24L01_InitTypeDef;

typedef struct 
{
  /* data */
  uint8_t *TX_ADDR;             //发送地址设置寄存器
  uint8_t *RX_ADDR_P0;          //数据通道0接收地址寄存器
  uint8_t *RX_ADDR_P1;
  uint8_t *RX_ADDR_P2;
  uint8_t *RX_ADDR_P3;
  uint8_t *RX_ADDR_P4;
  uint8_t *RX_ADDR_P5;
}TxRx_AddrTypeDef;

typedef struct 
{
  /* data */
  uint8_t RX_PW_P0;               //接收通道0有效数据宽度设置寄存器
  uint8_t RX_PW_P1;
  uint8_t RX_PW_P2;
  uint8_t RX_PW_P3;
  uint8_t RX_PW_P4;
  uint8_t RX_PW_P5;
}Rx_PwTypeDef;

typedef enum
{
  nrf_transmit_ok,
  nrf_transmit_error,
  nrf_receive_ok,
  nrf_receive_error
}NRF24L01_ErrorTypeDef;

typedef enum
{
  nrf_read,
  nrf_write
}NRF24L01_RWTypeDef;

typedef enum                  //信道质量
{
  Channel_good,
  Channel_congestion
}CHEAK_CDTypeDef;



typedef struct
{
  /* data */
  NRF24L01_InitTypeDef Init;
  TxRx_AddrTypeDef TxRx_Addr;
  Rx_PwTypeDef Rx_Pw;             //接收通道有效数据宽度设置寄存器
  NRF24L01_RWTypeDef RW_Set;  //读或写操作
  uint8_t cmd_data;
  uint8_t *TxRx_Data;
  uint8_t DataSize;     //读或写数据量
  uint8_t Status;       //状态寄存器
  NRF24L01_ErrorTypeDef errorcode;
}NRF24L01_HandleTypeDef;

typedef struct 
{
  NRF24L01_HandleTypeDef Handle;
  NRF_WorkStatusTypeDef WorkStatus;  //程序状态   
  NRF_LockTypeDef Lock;           //程序锁
}NRF24L01_ConfigTypeDef;


/** @defgroup nrf24l01 reg addr
  * @{
  */
#define  regconfig    0x00U 
#define  en_aa        0x01U 
#define  en_rx_addr   0x02U 
#define  setup_aw     0x03U
#define  setup_retr   0x04U 
#define  rf_ch        0x05U 
#define  rf_setup     0x06U 
#define  nrfstatus    0x07U 
#define  observe_tx   0x08U
#define  cd_check     0x09U
#define  rx_addr_p0   0x0AU 
#define  rx_addr_p1   0x0BU 
#define  rx_addr_p2   0x0CU 
#define  rx_addr_p3   0x0DU 
#define  rx_addr_p4   0x0EU 
#define  rx_addr_p5   0x0FU 
#define  tx_addr      0x10U
#define  rx_pw_p0     0x11U
#define  rx_pw_p1     0x12U
#define  rx_pw_p2     0x13U
#define  rx_pw_p3     0x14U
#define  rx_pw_p4     0x15U
#define  rx_pw_p5     0x16U
/**
  * @}
  */
/** @defgroup nrf24l01 cmd
  * @{
  */
#define   r_register    0x00U
#define   w_register    0x20U
#define   r_rx_payload  0x61U
#define   w_tx_payload  0xA0U
#define   flush_tx      0xE1U
#define   flush_rx      0xE2U
#define   nop           0xFFU
/**
  * @}
  */


void (*Spi_TxRx)(spi_transaction_t *t); 

extern NRF24L01_ConfigTypeDef nrfconfig;

NRF_WorkStatusTypeDef RW_NRF24L01_DATA(NRF24L01_HandleTypeDef *hnrf);
NRF_WorkStatusTypeDef NRF24L01_Config(NRF24L01_HandleTypeDef *hnrf);
// NRF_WorkStatusTypeDef NRF_Transmit(NRF24L01_HandleTypeDef *hnrf);
// NRF_WorkStatusTypeDef NRF_Receive(NRF24L01_HandleTypeDef *hnrf);
// NRF_WorkStatusTypeDef NRF_Transmit_Mode(NRF24L01_HandleTypeDef *hnrf);
// NRF_WorkStatusTypeDef NRF_Receive_Mode(NRF24L01_HandleTypeDef *hnrf);
irq8_t nrf_get_irq_state(void);
NRF_WorkStatusTypeDef Read_nrf_reg(uint8_t reg,uint8_t *regval);
NRF_WorkStatusTypeDef Write_nrf_reg(NRF24L01_HandleTypeDef *hnrf, uint8_t reg, uint8_t *regval,uint8_t datasize);
NRF_WorkStatusTypeDef Clean_nrf_fifo(uint8_t cmd);

void Clean_nrf_irq(irq8_t reg);
void Clean_nrf_tx_fifo(void);
void Clean_nrf_rx_fifo(void);
void NRF_Standby1(void);
void Power_Down(void);
void Power_Up(void);

#ifdef __cplusplus
}
#endif

#endif /* __NRF24L01_MAL_NRF */

/************************ (C) COPYRIGHT YHF *****END OF FILE****/
