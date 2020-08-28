/**
  ******************************************************************************
  * @file    NRF24L01.h
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
#ifndef __NRF24L01_H
#define __NRF24L01_H
#ifdef __cplusplus
 extern "C" {
#endif
//#include "NRF24L01_MAL_nrf.h"
#include "NRF24L01_MAL_nrf.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/list.h"
#include "freertos/stream_buffer.h"
//#include "NRF24L01_MAL_def.h"
//#include "gpio.h"
#include "driver/gpio.h"
#define PIN_NUM_CE   21
#define PIN_NUM_CE_MASK ((uint64_t) 1) << PIN_NUM_CE
#define PIN_NUM_IRQ  18
#define PIN_NUM_IRQ_MASK ((uint64_t) 1) << PIN_NUM_IRQ
#define ESP_INTR_FLAG_DEFAULT 0
#define pack_short  26
#define pack_long   32
#define CE_L  gpio_set_level(PIN_NUM_CE, 0)
#define CE_H  gpio_set_level(PIN_NUM_CE, 1)
#pragma pack (1)
typedef struct               //用于收发数据
{
  uint8_t nrf24l01_RW_cmd;
  char datapack[32];
}Conn_packetTypedef;


typedef struct              //用于建立连接
{
  uint8_t nrf24l01_RW_cmd; 
  uint8_t datacmd;
  uint32_t size;
  uint8_t client_id[10];
  uint8_t ClientServer_addr[5];
  uint8_t rf_zh;
  uint8_t servercmd;
  uint8_t login_flag;
  uint8_t topo_flag;
  Conn_packetTypedef *data_packet_p;
}ClientServerTypedef;
#pragma pack ()


// typedef enum              //数据包命令
// {
//   Heartbeat_pack,
//   Distribution_net_pack,
//   Data_pack
// }datacmdTypedef;


/*NRF_TransmitReceive任务发送和接收的不同状态*/
#define null_status     0x00
#define tx_config       0x01
#define rx_config       0x02
#define tx_configdata   0x03
#define tx_data_        0x04
#define rx_configdata   0x05
#define rx_data_        0x06
/*client模式*/
#define clientmode      0x01
#define servermode      0x00


extern xQueueHandle nrf_irq_queue;
extern xQueueHandle nrf_data_tx_queue;
extern xQueueHandle nrf_sign_queue;
extern xQueueHandle nrf_dataout_rx_queue;
extern xQueueHandle work_event_queue;
extern xQueueHandle nrf_clintserver_queue;
extern NRF24L01_HandleTypeDef hnrf_Default;
extern xQueueHandle nrf_dataconfig_queue;
extern StreamBufferHandle_t stream_buf;

void NRF_TransmitReceive(void);
void NRF24L01_Init(void);
void nrf_spi_conn_test(void);

NRF_WorkStatusTypeDef Tx_Config(ClientServerTypedef *config);
NRF_WorkStatusTypeDef Rx_Config(ClientServerTypedef *config);
NRF_WorkStatusTypeDef Tx_Data(Conn_packetTypedef *data);
NRF_WorkStatusTypeDef Rx_Data(Conn_packetTypedef *data);
#ifdef __cplusplus
} 
#endif
#endif /*__NRF24L01_H*/



/************************ (C) COPYRIGHT YHF *****END OF FILE****/
