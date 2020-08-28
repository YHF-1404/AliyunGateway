/**
  ******************************************************************************
  * @file    NRF24L01_MAL_def.h
  * @author  YHF
  * @brief    
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 YHF.
  * All rights reserved.</center></h2>
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __NRF24L01_MAL_DEF
#define __NRF24L01_MAL_DEF

#ifdef __cplusplus
 extern "C" {
#endif
// #include "/usr/include/x86_64-linux-gnu/bits/stdint-uintn.h"
// #include "/usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h"
//#define NULL (void*)0
/** @addtogroup Exported_macro
  * @{
  */
// #define SET_BIT(REG, BIT)     ((REG) |= (BIT))

// #define CLEAR_BIT(REG, BIT)   ((REG) &= ~(BIT))

// #define READ_BIT(REG, BIT)    ((REG) & (BIT))

// #define CLEAR_REG(REG)        ((REG) = (0x0))
#define irq8_t unsigned char
/**
  * @}
  */

/** 
  * @brief  NRF24L01 Status structures definition  
  */  
typedef enum 
{
  NRF_OK       = 0x00U,
  NRF_ERROR    = 0x01U,
  NRF_BUSY     = 0x02U,
  NRF_TIMEOUT  = 0x03U
} NRF_WorkStatusTypeDef;

/** 
  * @brief  NRF Lock structures definition  
  */
typedef enum 
{
  NRF_UNLOCKED = 0x00U,
  NRF_LOCKED   = 0x01U  
} NRF_LockTypeDef;


/*
nrf24l01状态寄存器状态
*/
#define  TX_FULL    0x01U        //TX_FIFO寄存器满标志
#define  Rx_P0      0x00U        //接收数据通道000~111: 数据通道号
#define  Rx_P2      0x02U
#define  Rx_P3      0x04U
#define  Rx_P4      0x06U
#define  Rx_P5      0x08U
#define  Rx_P6      0x0AU
#define  Rx_FIFO    0x0EU        //>111: RX_FIFO寄存器空
#define  MAX_RT			0x10U		    //数据包重发次数超过设定值
#define  TX_DS		  0x20U			  //数据发送完成
#define  RX_DR		  0x40U			  //数据接收完成



#define __NRF_LOCK(__HANDLE__)                                           \
                              do{                                        \
                                  if((__HANDLE__)->Lock == NRF_LOCKED)   \
                                  {                                      \
                                      return NRF_BUSY;                    \
                                  }                                      \
                                  else                                   \
                                  {                                      \
                                      (__HANDLE__)->Lock = NRF_LOCKED;    \
                                  }                                      \
                                }while (0U)
#define __NRF_UNLOCK(__HANDLE__)                                          \
                                do{                                       \
                                    (__HANDLE__)->Lock = NRF_UNLOCKED;    \
                                  }while (0U)


#ifdef __cplusplus
}
#endif

#endif /* __NRF24L01_MAL_DEF */

/************************ (C) COPYRIGHT YHF *****END OF FILE****/
