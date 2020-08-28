/**
  ******************************************************************************
  * @file    LinkToAliyun.h
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
#ifndef __LINKTOALIYUN_H
#define __LINKTOALIYUN_H
#ifdef __cplusplus
 extern "C" {
#endif
extern void *subdev_handle;
extern void *mqtt_handle;
void Link_Aliyun(void);
int32_t demo_mqtt_start(void **handle);
int32_t demo_mqtt_stop(void **handle);
#ifdef __cplusplus
 }
#endif

#endif /*__LINKTOALIYUN_H*/