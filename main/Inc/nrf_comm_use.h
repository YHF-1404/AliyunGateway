/**
  ******************************************************************************
  * @file    nrf_comm_use.h
  * @author  YHF
  * @brief   This file contains NRF24L01 config
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 YHF.
  * All rights reserved.</center></h2>
  *
  ******************************************************************************
  */
#ifndef __NRF_COMM_USE_H
#define __NRF_COMM_USE_H
#ifdef __cplusplus
 extern "C" {
#endif
#include "freertos/semphr.h"

#define event8_t  unsigned char

#define complete  0            //完成初始化
#define Control_event  1       //控制事件
#define evevtbock  9

typedef enum
{
    null_mode,
    monitorsta,         //通用通道监听
    broadcast,          //通用通道广播
    communSta_tx,       //正常通信 发送
    communSta_rx,       //正常通信　接收
    DeteChanSta,        //检测信道
    ConfNet             //给客户端配网
}sc_statusTypedef;

sc_statusTypedef sc_status;

typedef enum              //数据包命令
{
  null_pack,
  Heartbeat_pack,
  Distribution_net_pack,
  Data_pack
}datacmdTypedef;
datacmdTypedef datacmd;
extern TaskHandle_t xHandle1;
extern TaskHandle_t xHandle;
extern uint8_t dataeventl;
//extern SemaphoreHandle_t xSemaphore;
// void State_transition(void);
// void data_analysis(void);
// void Package_data(void);
// void Decapsulation_data(void);
//void Conn_State(void);
void nrf24l01_conn(void);
#ifdef __cplusplus
 }
#endif

#endif /*__NRF_COMM_USE_H*/