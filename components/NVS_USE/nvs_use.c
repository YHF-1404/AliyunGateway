/**
  ******************************************************************************
  * @file    nvs_use.c
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

#include <stdio.h>

#include "nvs_flash.h"
#include "nvs.h"
//#include "infra_defs.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs_use.h"
#include <stdio.h>
#include <string.h>
//#include "sdkconfig.h"

#define MFG_PARTITION_NAME "fctry"
//#define NVS_PRODUCT "client"

static const char *TAG = "TriadData";

static bool s_part_init_flag;

static esp_err_t HAL_ProductParam_init(void)
{
    esp_err_t ret = ESP_OK;

    do {
        if (s_part_init_flag == false) {
            if ((ret = nvs_flash_init_partition(MFG_PARTITION_NAME)) != ESP_OK) {
                ESP_LOGE(TAG, "NVS Flash init %s failed, Please check that you have flashed fctry partition!!!", MFG_PARTITION_NAME);
                break;
            }

            s_part_init_flag = true;
        }
    } while (0);

    return ret;
}

static int HAL_GetProductParam(char *param_name, const char *param_name_str,char *NVS_PRODUCT)
{
    esp_err_t ret;
    size_t read_len = 0;
    nvs_handle handle;

    do {
        if (HAL_ProductParam_init() != ESP_OK) {
            break;
        }

        if (param_name == NULL) {
            ESP_LOGE(TAG, "%s param %s NULL", __func__, param_name);
            break;
        }

        ret = nvs_open_from_partition(MFG_PARTITION_NAME, NVS_PRODUCT, NVS_READONLY, &handle);

        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "%s nvs_open failed with %x", __func__, ret);
            break;
        }

        ret = nvs_get_str(handle, param_name_str, NULL, (size_t *)&read_len);

        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "%s nvs_get_str get %s failed with %x", __func__, param_name_str, ret);
            break;
        }

        ret = nvs_get_str(handle, param_name_str, param_name, (size_t *)&read_len);

        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "%s nvs_get_str get %s failed with %x", __func__, param_name_str, ret);
        } else {
            ESP_LOGV(TAG, "%s %s %s", __func__, param_name_str, param_name);
        }

        nvs_close(handle);
    } while (0);

    return read_len;
}
/**
 * @brief Get device name from user's system persistent storage
 *
 * @param [ou] device_name: array to store device name, max length is IOTX_DEVICE_NAME_LEN
 * @return the actual length of device name
 */
int HAL_GetDeviceName_(char device_name[IOTX_DEVICE_NAME_LEN + 1],char *NVS_PRODUCT)
{
    return HAL_GetProductParam(device_name, "DeviceName",NVS_PRODUCT);
}

/**
 * @brief Get device secret from user's system persistent storage
 *
 * @param [ou] device_secret: array to store device secret, max length is IOTX_DEVICE_SECRET_LEN
 * @return the actual length of device secret
 */
int HAL_GetDeviceSecret_(char device_secret[IOTX_DEVICE_SECRET_LEN + 1],char *NVS_PRODUCT)
{
    return HAL_GetProductParam(device_secret, "DeviceSecret",NVS_PRODUCT);
}

/**
 * @brief Get product key from user's system persistent storage
 *
 * @param [ou] product_key: array to store product key, max length is IOTX_PRODUCT_KEY_LEN
 * @return  the actual length of product key
 */
int HAL_GetProductKey_(char product_key[IOTX_PRODUCT_KEY_LEN + 1],char *NVS_PRODUCT)
{
    return HAL_GetProductParam(product_key, "ProductKey",NVS_PRODUCT);
}

int HAL_GetProductSecret_(char product_secret[IOTX_PRODUCT_SECRET_LEN + 1],char *NVS_PRODUCT)
{
    return HAL_GetProductParam(product_secret, "ProductSecret",NVS_PRODUCT);
}

int HAL_Getconncheck_(char *check,char *NVS_PRODUCT)
{
    return HAL_GetProductParam(check, "conncheck",NVS_PRODUCT);
}

/**
 * @brief Get firmware version
 *
 * @param [ou] version: array to store firmware version, max length is IOTX_FIRMWARE_VER_LEN
 * @return the actual length of firmware version
 */
// int HAL_GetFirmwareVersion_(char *version)
// {
//     if (!version) {
//         ESP_LOGE(TAG, "%s version is NULL", __func__);
//         return 0;
//     }

//     memset(version, 0, IOTX_FIRMWARE_VER_LEN);
//     int len = strlen(CONFIG_LINKKIT_FIRMWARE_VERSION);
//     if (len > IOTX_FIRMWARE_VER_LEN) {
//         len = 0;
//     } else {
//         memcpy(version, CONFIG_LINKKIT_FIRMWARE_VERSION, len);
//     }

//     return len;
// }

static int HAL_SetProductParam(char *param_name, const char *param_name_str,char *NVS_PRODUCT)
{
    esp_err_t ret;
    size_t write_len = 0;
    nvs_handle handle;

    do {
        if (HAL_ProductParam_init() != ESP_OK) {
            break;
        }

        if (param_name == NULL) {
            ESP_LOGE(TAG, "%s param %s NULL", __func__, param_name);
            break;
        }

        ret = nvs_open_from_partition(MFG_PARTITION_NAME, NVS_PRODUCT, NVS_READWRITE, &handle);

        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "%s nvs_open failed with %x", __func__, ret);
            break;
        }

        ret = nvs_set_str(handle, param_name_str, param_name);

        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "%s nvs_set_str set %s failed with %x", __func__, param_name_str, ret);
        } else {
            write_len = strlen(param_name);
            ESP_LOGV(TAG, "%s %s %s", __func__, param_name_str, param_name);
        }

        nvs_close(handle);
    } while (0);

    return write_len;
}

int HAL_SetDeviceName_(char *device_name,char *NVS_PRODUCT)
{
    return HAL_SetProductParam(device_name, "DeviceName",NVS_PRODUCT);
}

int HAL_SetDeviceSecret_(char *device_secret,char *NVS_PRODUCT)
{
    return HAL_SetProductParam(device_secret, "DeviceSecret",NVS_PRODUCT);
}

int HAL_SetProductKey_(char *product_key,char *NVS_PRODUCT)
{
    return HAL_SetProductParam(product_key, "ProductKey",NVS_PRODUCT);
}

int HAL_SetProductSecret_(char *product_secret,char *NVS_PRODUCT)
{
    return HAL_SetProductParam(product_secret, "ProductSecret",NVS_PRODUCT);
}

int HAL_Setconncheck_(char *check,char *NVS_PRODUCT)
{
    return HAL_SetProductParam(check, "conncheck",NVS_PRODUCT);
}
