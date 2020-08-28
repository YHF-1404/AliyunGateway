/**
  ******************************************************************************
  * @file    nvs_use.h
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
#ifndef __NVS_USE_H
#define __NVS_USE_H
#ifdef __cplusplus
 extern "C" {
#endif
#define IOTX_SDK_VERSION                "3.0.1"
#define IOTX_ALINK_VERSION              "20"
#define IOTX_FIRMWARE_VERSION_LEN       (32)
#define IOTX_PRODUCT_KEY_LEN            (20)
#define IOTX_DEVICE_NAME_LEN            (32)
#define IOTX_DEVICE_SECRET_LEN          (64)
#define IOTX_DEVICE_ID_LEN              (64)
#define IOTX_PRODUCT_SECRET_LEN         (64)
#define IOTX_PARTNER_ID_LEN             (64)
#define IOTX_MODULE_ID_LEN              (64)
#define IOTX_NETWORK_IF_LEN             (160)
#define IOTX_FIRMWARE_VER_LEN           (32)
#define IOTX_URI_MAX_LEN                (135)

int HAL_GetProductKey_(char product_key[IOTX_PRODUCT_KEY_LEN],char *NVS_PRODUCT);
int HAL_GetDeviceName_(char device_name[IOTX_DEVICE_NAME_LEN],char *NVS_PRODUCT);
int HAL_GetDeviceSecret_(char device_secret[IOTX_DEVICE_SECRET_LEN],char *NVS_PRODUCT);
int HAL_Getconncheck_(char *check,char *NVS_PRODUCT);
int HAL_GetProductSecret_(char *product_key,char *NVS_PRODUCT);

int HAL_SetProductKey_(char *product_key,char *NVS_PRODUCT);
int HAL_SetProductSecret_(char *product_secret,char *NVS_PRODUCT);
int HAL_SetDeviceName_(char *device_name,char *NVS_PRODUCT);
int HAL_SetDeviceSecret_(char *device_secret,char *NVS_PRODUCT);
int HAL_Setconncheck_(char *check,char *NVS_PRODUCT);

#ifdef __cplusplus
} 
#endif
#endif /*__NVS_USE_H*/



/************************ (C) COPYRIGHT YHF *****END OF FILE****/
