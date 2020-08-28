/**
  ******************************************************************************
  * @file    nrf_comm_use.c
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
//#include "NRF24L01_MAL_nrf.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//#include "rom/gpio.h"
#include "driver/gpio.h"

#include "nrf_comm_use.h"
#include "nrf24l01.h"
#include "esp_log.h"
#include "string.h"
//#include "infra_defs.h"
#include "cJSON.h"
#include "nvs_use.h"
//#include "freertos/stream_buffer.h"
#include "client_list.h"
#include "aiot_subdev_api.h"
#include "LinkToAliyun.h"
#include "aiot_state_api.h"
#include "spi_master_use.h"
// static const char *TAG = "nrf_comm_use";
#define sliencecheck "sliencecheck"
TaskHandle_t xHandle;
TaskHandle_t xHandle1;
SemaphoreHandle_t xSemaphore = NULL;
uint8_t dataeventl = 0;
datacmdTypedef Normal_communication_tx(ClientServerTypedef *clienserver);
datacmdTypedef Normal_communication_rx(ClientServerTypedef *clienserver);
//NRF_WorkStatusTypeDef Detection_channel(ClientServerTypedef *clienserver);
datacmdTypedef Universal_channel_monitor(ClientServerTypedef *clienserver);
datacmdTypedef Universal_channel_broadcast(ClientServerTypedef *clienserver);
void RF_channel_generation(ClientServerTypedef *clienserver);
ClientServerTypedef address_generation(void);
sc_statusTypedef Heartbeat_pack_duty(ClientServerTypedef *clienserver);
void analysis_data(ClientServerTypedef *clienserver,char *data,uint16_t data_len);
sc_statusTypedef To_configure_client_network(ClientServerTypedef *clienserver);
sc_statusTypedef To_conn_server_network(ClientServerTypedef *clienserver);
sc_statusTypedef Heartbeat_pack_Generate(ClientServerTypedef *clienserver);
void Logout_callback(ClientList_Typedef *head);
void Login_Aliyun(ClientServerTypedef *clienserver);
void Conn_State(void)
{
    cleanTopo_cb = Logout_callback;      
    while(1)
    {
        xSemaphoreTake( xSemaphore, portMAX_DELAY );                // 获得资源的使用权
        head = CleanAllRW_cmd(head);            //清除心跳标志
        head = CutAllListItemData(head);
        //TAG = "Conn_State";
        //ESP_LOGI(TAG,"data_cmd:%x",head->clientserver.nrf24l01_RW_cmd);
        xSemaphoreGive( xSemaphore );       //释放资源的使用权        
        vTaskDelay(35000 / portTICK_PERIOD_MS);
        xSemaphoreTake( xSemaphore, portMAX_DELAY );                // 获得资源的使用权

        head = RW_cmdDeleItem(head);        //把死掉的client设备从列表里面清除
        //head = CutAllListItemData(head);
        xSemaphoreGive( xSemaphore );       //释放资源的使用权    
    }
}

void Logout_callback(ClientList_Typedef *head)
{
    char PRODUCT_KEY_[IOTX_PRODUCT_KEY_LEN + 1] = {0};
    char PRODUCT_SECRET_[IOTX_PRODUCT_SECRET_LEN + 1] = {0};
    char DEVICE_NAME_[IOTX_DEVICE_NAME_LEN + 1] = {0};
    char DEVICE_SECRET_[IOTX_DEVICE_SECRET_LEN + 1] = {0};
    HAL_GetProductKey_(PRODUCT_KEY_,(char *)head->clientserver.client_id);        //nvs中取出三元组
    HAL_GetProductSecret_(PRODUCT_SECRET_,(char *)head->clientserver.client_id);
    HAL_GetDeviceName_(DEVICE_NAME_,(char *)head->clientserver.client_id);
    HAL_GetDeviceSecret_(DEVICE_SECRET_,(char *)head->clientserver.client_id);

    aiot_subdev_dev_t aiot_subdev = 
    {
        .device_name = DEVICE_NAME_,
        .device_secret = DEVICE_SECRET_,
        .product_key = PRODUCT_KEY_,
        .product_secret = PRODUCT_SECRET_
    };
    aiot_subdev_send_batch_logout(subdev_handle, &aiot_subdev, 1); //登出
    //aiot_subdev_send_topo_delete(subdev_handle, &aiot_subdev, 1);   //注销TOPO
}

void Login_Aliyun(ClientServerTypedef *clienserver)
{
    static const char *TAG = "Login_Aliyun";
    if(clienserver->login_flag == 0)
    {
        char PRODUCT_KEY_[IOTX_PRODUCT_KEY_LEN + 1] = {0};
        char PRODUCT_SECRET_[IOTX_PRODUCT_SECRET_LEN + 1] = {0};
        char DEVICE_NAME_[IOTX_DEVICE_NAME_LEN + 1] = {0};
        char DEVICE_SECRET_[IOTX_DEVICE_SECRET_LEN + 1] = {0};
        HAL_GetProductKey_(PRODUCT_KEY_,(char *)clienserver->client_id);        //nvs中取出三元组
        HAL_GetProductSecret_(PRODUCT_SECRET_,(char *)clienserver->client_id);
        HAL_GetDeviceName_(DEVICE_NAME_,(char *)clienserver->client_id);
        HAL_GetDeviceSecret_(DEVICE_SECRET_,(char *)clienserver->client_id);

        aiot_subdev_dev_t aiot_subdev = 
        {
            .device_name = DEVICE_NAME_,
            .device_secret = DEVICE_SECRET_,
            .product_key = PRODUCT_KEY_,
            .product_secret = PRODUCT_SECRET_
        };
        int32_t res = aiot_subdev_send_batch_login(subdev_handle, &aiot_subdev, 1); //登入   
        if (res < STATE_SUCCESS) 
        {
            printf("aiot_subdev_send_batch_login failed, res: -0x%04X\n", -res);
            clienserver->login_flag = 0;
        }
        else
        {
            clienserver->login_flag = 1;
            //ESP_LOGI(TAG,"head->clientserver.login_flag :%d",head->clientserver.login_flag ); 
        }
        
    }

    //aiot_subdev_send_topo_delete(subdev_handle, &aiot_subdev, 1);   //注销TOPO
}

void nrf24l01_conn(void)
{
    static const char *TAG = "nrf24l01_conn";

    ClientServerTypedef clienserverrx;
    ClientServerTypedef *clienserverrx_p = &clienserverrx;
    //sizeof(ClientServerTypedef);
    Conn_packetTypedef conn_packet;
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    SPI_Init();
    NRF24L01_Init();

    HAL_Setconncheck_("1",sliencecheck);
    xSemaphore = xSemaphoreCreateMutex();        /* 创建互斥信号量 */
    xTaskCreate((void (*)(void *))Conn_State,"Conn_State",3072,NULL,10,xHandle1);
    xTaskCreate((void (*)(void *))NRF_TransmitReceive,"NRF_TransmitReceive",3500,NULL,10,NULL);
   // xTaskCreate(NRF_Transmit,"NRF_Transmit",2048,NULL,10,NULL);
    //xTaskCreate(Package_data,"Package_data",2048,NULL,10,NULL);

    //ClientList_Typedef *devicelist = NULL;          //初始化client设备链表
    head = InitList(head);
    //devicelist = head;

    //DataBuf_ListTypedef *datalist = NULL;           //初始化数据FIFO


    clienserverrx.data_packet_p = &conn_packet;
    uint8_t clientServerMode = servermode;
    sc_status = monitorsta;
    ESP_LOGI(TAG,"clienserverrx.data_packet_p addr:%p",clienserverrx_p);
    xQueueSendToBack(nrf_dataconfig_queue,&clienserverrx_p,portMAX_DELAY);
    while(1)
    {
        if(dataeventl == 1)
        {
            dataeventl = 0;
            sc_status = communSta_tx;
            //clienserverrx_p->nrf24l01_RW_cmd = 0xAA;        //刷新
            clienserverrx_p->datacmd = Data_pack;
            ESP_LOGI(TAG,"--------------------------------");
        }
        else
        {
            sc_status = monitorsta;
        }
        
            // ESP_LOGI(TAG,"dataevent:%d",*dataeventl); 
            // printf("dataevent_addr:%p\n",dataeventl);
        switch (sc_status)
        {
            case monitorsta:
                ESP_LOGI(TAG,"monitorsta"); 
                datacmd = Universal_channel_monitor(clienserverrx_p);
                sc_status = null_mode;
            break;
            case broadcast:
                ESP_LOGI(TAG,"broadcast"); 
                datacmd = Universal_channel_broadcast(clienserverrx_p);
                sc_status = null_mode;
            break;
            case communSta_tx:
                ESP_LOGI(TAG,"communSta_tx"); 
                datacmd = Normal_communication_tx(clienserverrx_p);
                sc_status = null_mode;
            break;
            case communSta_rx:
                ESP_LOGI(TAG,"communSta_rx"); 
                datacmd = Normal_communication_rx(clienserverrx_p);
                sc_status = null_mode;
            break;
            case DeteChanSta:
                ESP_LOGI(TAG,"DeteChanSta"); 
                //Detection_channel(clienserverrx_p);
                sc_status = null_mode;
            break;     
            case ConfNet:
                ESP_LOGI(TAG,"ConfNet"); 
                //To_configure_client_network(clienserverrx_p);
                sc_status = null_mode;
            break;     
            default: vTaskDelay(10 / portTICK_PERIOD_MS);
            break;
        }
        switch (datacmd)
        {
            case Heartbeat_pack:
                
                if(clientServerMode == servermode)
                {
                    sc_status = Heartbeat_pack_duty(clienserverrx_p);
                }
                else if(clientServerMode == clientmode)
                {
                    sc_status = Heartbeat_pack_Generate(clienserverrx_p);
                }
                datacmd = null_pack;
                /* code */
            break;
            case Distribution_net_pack:
                if(clientServerMode == servermode)
                {
                    sc_status = To_configure_client_network(clienserverrx_p);
                }
                else if(clientServerMode == clientmode)
                {
                    sc_status = To_conn_server_network(clienserverrx_p);
                }
                datacmd = null_pack;
            break;        
            case Data_pack:
                sc_status = monitorsta;

                datacmd = null_pack;
            break;     
            default:
            break;
        }
    }
}

sc_statusTypedef Heartbeat_pack_Generate(ClientServerTypedef *clienserver)
{
    static const char *TAG = "Heartbeat_pack_Generate";
    ESP_LOGI(TAG,"clienserver->client_id:%s",clienserver->client_id); 
    clienserver->nrf24l01_RW_cmd = 0xAA;
    //head = Found_ModifyListItemData(head,clienserver);      //更新client信息
    if(clienserver->size > 0)
    {
        return communSta_tx;
    }
    else
    {
        return broadcast;
    }
}

sc_statusTypedef Heartbeat_pack_duty(ClientServerTypedef *clienserver)
{
    static const char *TAG = "Heartbeat_pack_duty";
    //ESP_LOGI(TAG,"clienserver->client_id:%s",clienserver->client_id);
    // ESP_LOGI(TAG,"clienserver->nrf24l01_RW_cmd:%x",clienserver->nrf24l01_RW_cmd);
    clienserver->nrf24l01_RW_cmd = 0xAA;
    head = FoundListItemData(head,(char *)clienserver->client_id);
    if(head->error == 1)
    {
        head->error = 0;
        clienserver->login_flag = 0;
    }
    Login_Aliyun(clienserver);
    head = Found_ModifyListItemData(head,clienserver);      //更新client信息
    head->Buflist = InitDataList(head->Buflist);        //初始化数据FIFO
    // ESP_LOGI(TAG,"clienserver->login_flag:%d",clienserver->login_flag);

    if(clienserver->size > 0)
    {
        return communSta_rx;
    }
    else
    {
        return monitorsta;
    }
}

void RF_channel_generation(ClientServerTypedef *clienserver)
{
    static const char *TAG = "RF_channel_generation";
    NRF24L01_HandleTypeDef hnrfx = hnrf_Default;
    NRF24L01_HandleTypeDef hnrfcp = hnrf_Default;
    //uint8_t nrfsign = 0;
    uint8_t nrfreg = 0;
    uint8_t i = 0;
    //ClientServerTypedef clienserver;
    Read_nrf_reg(regconfig,&nrfreg);
    hnrfcp.Init.CONFIG = nrfreg;

    nrfreg = 0;  
    hnrfcp.Init.RF_CH = clienserver->rf_zh;
    memcpy(hnrfcp.TxRx_Addr.TX_ADDR, clienserver->ClientServer_addr, sizeof(clienserver->ClientServer_addr));
    memcpy(hnrfcp.TxRx_Addr.RX_ADDR_P0, clienserver->ClientServer_addr, sizeof(clienserver->ClientServer_addr));
    hnrfx.Init.RF_CH = 3;
    while(1)
    {
        while((nrfreg == 0) && (i < 10))
        {

            NRF24L01_Config(&hnrfx);
            vTaskDelay(1 / portTICK_PERIOD_MS);
            Read_nrf_reg(cd_check,&nrfreg);
            vTaskDelay(1 / portTICK_PERIOD_MS);
            i++;
        }
        if(i >= 10)
        {
            NRF24L01_Config(&hnrfcp);
            clienserver->rf_zh = hnrfx.Init.RF_CH;
            break;
        }
        else
        {
            nrfreg = 0;
            i = 0;
            hnrfx.Init.RF_CH = hnrfx.Init.RF_CH + 1;
            if(hnrfx.Init.RF_CH >= 0x7f)
            {
                NRF24L01_Config(&hnrfcp);
                hnrfx.Init.RF_CH = 3;
                clienserver->rf_zh = hnrfx.Init.RF_CH;
                break;
            }
        }
    }
    ESP_LOGI(TAG,"cd_check:%x",nrfreg);    
}

ClientServerTypedef address_generation(void)
{
    static const char *TAG = "address_geration";
    uint32_t rand32addr = esp_random();
    uint8_t i;

    ClientServerTypedef clienserver;
    ESP_LOGI(TAG,"rand32addr:%x",rand32addr); 
    for(i = 0;i < 4;i++)
    {
        clienserver.ClientServer_addr[i] = ((uint8_t *)&rand32addr)[i];
        ESP_LOGI(TAG,"clienserver->ClientServer_addr[%d]:%x",i,clienserver.ClientServer_addr[i]); 
    }
    clienserver.ClientServer_addr[4] = 0xFF;
    return clienserver;
}

void analysis_data(ClientServerTypedef *clienserver,char *data,uint16_t data_len)
{
    static const char *TAG = "analysis_data";
    cJSON *root = NULL, *datalabel= NULL;
    char devicename[33]={0};
    TAG = "analysis_data";
    //ESP_LOGI(TAG,"jsondata,len:%d: %s", data_len,data);
    /* Parse Root */
    root = cJSON_Parse(data);
    if (!root)
    {
        ESP_LOGE(TAG,"JSON Parse Error");
    }
    datalabel = cJSON_GetObjectItem(root, "DeviceName");
    if(datalabel)
    {
        memcpy(clienserver->client_id,datalabel->valuestring,strlen(datalabel->valuestring));
        memcpy(devicename, datalabel->valuestring, strlen(datalabel->valuestring));
        HAL_SetDeviceName_(datalabel->valuestring,devicename);

        datalabel = cJSON_GetObjectItem(root, "DeviceSecret");
        if(datalabel)
        {
            HAL_SetDeviceSecret_(datalabel->valuestring,devicename);
        }
        datalabel = cJSON_GetObjectItem(root, "ProductKey");
        if(datalabel)
        {
            HAL_SetProductKey_(datalabel->valuestring,devicename);
        }
        datalabel = cJSON_GetObjectItem(root, "ProductSecret");
        if(datalabel)
        {
            HAL_SetProductSecret_(datalabel->valuestring,devicename);
        }
    }  
    else
    {
        printf("JSON:%s\n",cJSON_Print(root));

    }

    cJSON_Delete(root);
    // char ffff[65]={0};
    // HAL_GetDeviceName_(ffff,devicename);
    // ESP_LOGI(TAG,"HAL_GetDeviceName_:%s",ffff);
    // HAL_GetDeviceSecret_(ffff,devicename);
    // ESP_LOGI(TAG,"HAL_GetDeviceSecret_:%s",ffff);
    // HAL_GetProductKey_(ffff,devicename);
    // ESP_LOGI(TAG,"HAL_GetProductKey_:%s",ffff);
    // HAL_GetProductSecret_(ffff,devicename);
    // ESP_LOGI(TAG,"HAL_GetProductSecret_:%s",ffff);
}

sc_statusTypedef To_configure_client_network(ClientServerTypedef *clienserver)
{
    static const char *TAG = "To_configure_client_network";
    uint8_t nrfsign = 0;
    uint32_t i = 0;
    size_t xReceivedBytes = 0;
    char PRODUCT_KEY_[IOTX_PRODUCT_KEY_LEN + 1] = {0};
    char PRODUCT_SECRET_[IOTX_PRODUCT_SECRET_LEN + 1] = {0};
    char DEVICE_NAME_[IOTX_DEVICE_NAME_LEN + 1] = {0};
    char DEVICE_SECRET_[IOTX_DEVICE_SECRET_LEN + 1] = {0};
    //uint16_t datasize = 0;
    //uint8_t aaaa[32]={0};
    char *receive_Triad = NULL;
    receive_Triad = (char *)calloc(1,clienserver->size + 1);
    char *receive_Triad_p = receive_Triad;
    // clienserver->servercmd = tx_config;      //标记,表示该数据包为发送配置数据

    clienserver->servercmd = rx_config;      
    xQueueReceive(nrf_sign_queue,&nrfsign,portMAX_DELAY);
    if(nrfsign == 0x02)ESP_LOGI(TAG,"配置接收模式成功");  

    clienserver->servercmd = rx_data_;     

    i = clienserver->size;
    vTaskDelay(100 / portTICK_PERIOD_MS);
    while(i)
    {
        xReceivedBytes = xStreamBufferReceive( stream_buf,( void * ) receive_Triad_p,clienserver->size,2000 / portTICK_RATE_MS );
        if(xReceivedBytes > 0)
        {
            ESP_LOGI(TAG,"Json,len:%d:%s",xReceivedBytes,(char *)receive_Triad_p);  
            receive_Triad_p = receive_Triad_p + xReceivedBytes;
            i = i - xReceivedBytes;
        }
    }
    xQueueReceive(nrf_sign_queue,&nrfsign,portMAX_DELAY);
    if(nrfsign == 0x20)ESP_LOGI(TAG,"接收数据成功！");  
    analysis_data(clienserver,receive_Triad,clienserver->size);
    ESP_LOGI(TAG,"DEVICE_NAME ID:%s",clienserver->client_id); 
    xSemaphoreTake( xSemaphore, portMAX_DELAY );                // 获得资源的使用权
    clienserver->nrf24l01_RW_cmd = 0xAA;
    head = Found_ModifyListItemData(head,clienserver);
    head = CutAllListItemData(head);
    head = CutOneListItemData(head,(char *)clienserver->client_id);
    head->Buflist = InitDataList(head->Buflist);        //初始化数据FIFO
    xSemaphoreGive( xSemaphore );       //释放资源的使用权

    HAL_GetProductKey_(PRODUCT_KEY_,(char *)clienserver->client_id);        //nvs中取出三元组
    HAL_GetProductSecret_(PRODUCT_SECRET_,(char *)clienserver->client_id);
    HAL_GetDeviceName_(DEVICE_NAME_,(char *)clienserver->client_id);
    HAL_GetDeviceSecret_(DEVICE_SECRET_,(char *)clienserver->client_id);

    aiot_subdev_dev_t aiot_subdev = 
    {
        .device_name = DEVICE_NAME_,
        .device_secret = DEVICE_SECRET_,
        .product_key = PRODUCT_KEY_,
        .product_secret = PRODUCT_SECRET_
    };
    int32_t res = aiot_subdev_send_topo_add(subdev_handle, &aiot_subdev, 1);     //向云端添加topo关系
    if (res < STATE_SUCCESS) 
    {
        printf("aiot_subdev_send_topo_add failed, res: -0x%04X\n", -res);
        clienserver->topo_flag = 0;
    }
    else
    {
        clienserver->topo_flag = 1;        
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
    clienserver->servercmd = tx_config;      
    xQueueReceive(nrf_sign_queue,&nrfsign,portMAX_DELAY);
    if(nrfsign == 0x01)ESP_LOGI(TAG,"配置发送模式成功");  
    vTaskDelay(100 / portTICK_PERIOD_MS);
    while(dataeventl != 1)
    {
        clienserver->servercmd = tx_configdata;      
        xQueueReceive(nrf_sign_queue,&nrfsign,5000 / portTICK_PERIOD_MS);
        if(nrfsign == 0x03)
        {
            ESP_LOGI(TAG,"发送配置数据成功！");
            ESP_LOGI(TAG,"clienserver.nrf24l01_RW_cmd:%x",clienserver->nrf24l01_RW_cmd);
            ESP_LOGI(TAG,"clienserver.datacmd:%x",clienserver->datacmd);
            ESP_LOGI(TAG,"clienserver.rf_zh:%d",clienserver->rf_zh);
            ESP_LOGI(TAG,"clienserver.size:%d",clienserver->size);
            ESP_LOGI(TAG,"clienserver.servercmd:%x",clienserver->servercmd);
            ESP_LOGI(TAG,"clienserver->topo_flag:%x",clienserver->topo_flag);
            ESP_LOGI(TAG,"clienserver.ClientServer_addr[4]:%x",clienserver->ClientServer_addr[4]);
            ESP_LOGI(TAG,"clienserver.ClientServer_addr[3]:%x",clienserver->ClientServer_addr[3]);
            ESP_LOGI(TAG,"clienserver.ClientServer_addr[2]:%x",clienserver->ClientServer_addr[2]);
            ESP_LOGI(TAG,"clienserver.ClientServer_addr[1]:%x",clienserver->ClientServer_addr[1]);
            ESP_LOGI(TAG,"clienserver.ClientServer_addr[0]:%x",clienserver->ClientServer_addr[0]);
            break;
        }
        //vTaskDelay(5000 / portTICK_PERIOD_MS);
    } 
    ESP_LOGI(TAG,"---------------------------");
    res = aiot_subdev_send_batch_login(subdev_handle, &aiot_subdev, 1);;     //该设备登录云端
    if (res < STATE_SUCCESS) 
    {
        printf("aiot_subdev_send_batch_login failed, res: -0x%04X\n", -res);
        clienserver->login_flag = 0;
    }    
    else
    {
        clienserver->login_flag = 1;        
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
    clienserver->servercmd = tx_config;      
    xQueueReceive(nrf_sign_queue,&nrfsign,portMAX_DELAY);
    if(nrfsign == 0x01)ESP_LOGI(TAG,"配置发送模式成功");  
    vTaskDelay(100 / portTICK_PERIOD_MS);
    while(dataeventl != 1)
    {
        clienserver->servercmd = tx_configdata;      
        xQueueReceive(nrf_sign_queue,&nrfsign,5000 / portTICK_PERIOD_MS);
        if(nrfsign == 0x03)
        {
            ESP_LOGI(TAG,"发送配置数据成功！");
            ESP_LOGI(TAG,"clienserver.nrf24l01_RW_cmd:%x",clienserver->nrf24l01_RW_cmd);
            ESP_LOGI(TAG,"clienserver.datacmd:%x",clienserver->datacmd);
            ESP_LOGI(TAG,"clienserver.rf_zh:%d",clienserver->rf_zh);
            ESP_LOGI(TAG,"clienserver.size:%d",clienserver->size);
            ESP_LOGI(TAG,"clienserver.servercmd:%x",clienserver->servercmd);
            ESP_LOGI(TAG,"clienserver->topo_flag:%x",clienserver->topo_flag);
            ESP_LOGI(TAG,"clienserver.ClientServer_addr[4]:%x",clienserver->ClientServer_addr[4]);
            ESP_LOGI(TAG,"clienserver.ClientServer_addr[3]:%x",clienserver->ClientServer_addr[3]);
            ESP_LOGI(TAG,"clienserver.ClientServer_addr[2]:%x",clienserver->ClientServer_addr[2]);
            ESP_LOGI(TAG,"clienserver.ClientServer_addr[1]:%x",clienserver->ClientServer_addr[1]);
            ESP_LOGI(TAG,"clienserver.ClientServer_addr[0]:%x",clienserver->ClientServer_addr[0]);
            break;
        }
        //vTaskDelay(5000 / portTICK_PERIOD_MS);
    } 
  

    free(receive_Triad);
    receive_Triad = NULL;
    return monitorsta;
}

sc_statusTypedef To_conn_server_network(ClientServerTypedef *clienserver)
{
    static const char *TAG = "To_conn_server_network";
    uint8_t nrfsign = 0;
    //uint8_t i;
    size_t xReceivedBytes;
    char PRODUCT_KEY_[IOTX_PRODUCT_KEY_LEN + 1] = {0};
    char PRODUCT_SECRET_[IOTX_PRODUCT_SECRET_LEN + 1] = {0};
    char DEVICE_NAME_[IOTX_DEVICE_NAME_LEN + 1] = {0};
    char DEVICE_SECRET_[IOTX_DEVICE_SECRET_LEN + 1] = {0};

    cJSON *root = NULL;
    HAL_GetProductKey_(PRODUCT_KEY_,"aliyun-key");
    HAL_GetProductSecret_(PRODUCT_SECRET_,"aliyun-key");
    HAL_GetDeviceName_(DEVICE_NAME_,"aliyun-key");
    HAL_GetDeviceSecret_(DEVICE_SECRET_,"aliyun-key");

    root = cJSON_CreateObject();        //创建根数据对象

    cJSON_AddStringToObject(root,"DeviceName",DEVICE_NAME_);
    cJSON_AddStringToObject(root,"DeviceSecret",DEVICE_SECRET_);
    cJSON_AddStringToObject(root,"ProductKey",PRODUCT_KEY_);
    cJSON_AddStringToObject(root,"ProductSecret",PRODUCT_SECRET_);
    char *out = cJSON_Print(root);

    ESP_LOGI(TAG,"json_len:%d:%s",strlen(out) ,out);


    clienserver->size = strlen(out);
    RF_channel_generation(clienserver);         //查找干净的射频通道
    memcpy(clienserver->ClientServer_addr, address_generation().ClientServer_addr, sizeof(clienserver->ClientServer_addr));
    memcpy(clienserver->client_id,DEVICE_NAME_,strlen(DEVICE_NAME_));
    ESP_LOGI(TAG,"strlen(DEVICE_NAME):%d",strlen(DEVICE_NAME_)); 
    // for(i = 0;i < 5;i++)
    // {
    //     clienserver->ClientServer_addr[i] = address_generation().ClientServer_addr[i];  //生成４０bit随机地址
    // }
    vTaskDelay(50 / portTICK_PERIOD_MS);
    while(1)
    {
        clienserver->servercmd = tx_configdata;      
        xQueueReceive(nrf_sign_queue,&nrfsign,5000 / portTICK_PERIOD_MS);
        if(nrfsign == 0x03)
        {
            ESP_LOGI(TAG,"发送配置数据成功！");
            ESP_LOGI(TAG,"clienserver.nrf24l01_RW_cmd:%x",clienserver->nrf24l01_RW_cmd);
            ESP_LOGI(TAG,"clienserver.datacmd:%x",clienserver->datacmd);
            ESP_LOGI(TAG,"clienserver.rf_zh:%d",clienserver->rf_zh);
            ESP_LOGI(TAG,"clienserver.size:%d",clienserver->size);
            ESP_LOGI(TAG,"clienserver.servercmd:%x",clienserver->servercmd);
            ESP_LOGI(TAG,"clienserver->topo_flag:%x",clienserver->topo_flag);
            ESP_LOGI(TAG,"clienserver.ClientServer_addr[4]:%x",clienserver->ClientServer_addr[4]);
            ESP_LOGI(TAG,"clienserver.ClientServer_addr[3]:%x",clienserver->ClientServer_addr[3]);
            ESP_LOGI(TAG,"clienserver.ClientServer_addr[2]:%x",clienserver->ClientServer_addr[2]);
            ESP_LOGI(TAG,"clienserver.ClientServer_addr[1]:%x",clienserver->ClientServer_addr[1]);
            ESP_LOGI(TAG,"clienserver.ClientServer_addr[0]:%x",clienserver->ClientServer_addr[0]);
            break;
        }
        //vTaskDelay(5000 / portTICK_PERIOD_MS);
    } 
    vTaskDelay(50 / portTICK_PERIOD_MS);
    clienserver->servercmd = tx_config;      
    xQueueReceive(nrf_sign_queue,&nrfsign,portMAX_DELAY);
    if(nrfsign == 0x01)ESP_LOGI(TAG,"配置发送模式成功");     
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    clienserver->servercmd = tx_data_;   
    xReceivedBytes = xStreamBufferSend( stream_buf,( void * ) out,strlen(out),5000 / portTICK_RATE_MS );
    xQueueReceive(nrf_sign_queue,&nrfsign,portMAX_DELAY);
    if(nrfsign == 0x08)ESP_LOGI(TAG,"发送数据成功！");
    clienserver->size = 0;              //发送完数据记得清零
    cJSON_Delete(root);
    clienserver->servercmd = rx_config;      
    xQueueReceive(nrf_sign_queue,&nrfsign,portMAX_DELAY);
    if(nrfsign == 0x02)ESP_LOGI(TAG,"配置接收模式成功");  
    clienserver->servercmd = rx_configdata;      
    while(1)
    {
        xQueueReceive(nrf_sign_queue,&nrfsign,500 / portTICK_PERIOD_MS);
        if(nrfsign == 0x10)
        {
            ESP_LOGI(TAG,"接收配置数据成功！");
            ESP_LOGI(TAG,"clienserver.nrf24l01_RW_cmd:%x",clienserver->nrf24l01_RW_cmd);
            ESP_LOGI(TAG,"clienserver.datacmd:%x",clienserver->datacmd);
            ESP_LOGI(TAG,"clienserver.rf_zh:%d",clienserver->rf_zh);
            ESP_LOGI(TAG,"clienserver.size:%d",clienserver->size);
            ESP_LOGI(TAG,"clienserver.servercmd:%x",clienserver->servercmd);
            ESP_LOGI(TAG,"clienserver->login_flag:%x",clienserver->login_flag);
            ESP_LOGI(TAG,"clienserver->topo_flag:%x",clienserver->topo_flag);
            ESP_LOGI(TAG,"clienserver.ClientServer_addr[4]:%x",clienserver->ClientServer_addr[4]);
            ESP_LOGI(TAG,"clienserver.ClientServer_addr[3]:%x",clienserver->ClientServer_addr[3]);
            ESP_LOGI(TAG,"clienserver.ClientServer_addr[2]:%x",clienserver->ClientServer_addr[2]);
            ESP_LOGI(TAG,"clienserver.ClientServer_addr[1]:%x",clienserver->ClientServer_addr[1]);
            ESP_LOGI(TAG,"clienserver.ClientServer_addr[0]:%x",clienserver->ClientServer_addr[0]);
            break;
        }
    }
    if(clienserver->topo_flag == 0)
    {
        ESP_LOGE(TAG,"连接网关失败，topo关系添加失败！");
    }
    else
    {
        ESP_LOGI(TAG,"连接网关成功，topo关系添加成功！");
        HAL_Setconncheck_("1",sliencecheck);       
    }
    return broadcast;      
}

datacmdTypedef Universal_channel_broadcast(ClientServerTypedef *clienserver)
{
    static const char *TAG = "广播模式";
    uint8_t i = 0;
    uint8_t nrfsign = 0;
    char conncheck[]="0";
    char DEVICE_NAME_[IOTX_DEVICE_NAME_LEN + 1] = {0};
    ClientServerTypedef clienservertemp;
    NRF24L01_HandleTypeDef hnrfx = hnrf_Default;
    clienservertemp = *clienserver;
    clienserver->datacmd = 0;
    clienserver->size = 0;
    clienserver->rf_zh = 2;           //24000MHz + rf_zh ,max:255
    for(i = 0; i < 5; i++)
    {
        clienserver->ClientServer_addr[i] = hnrfx.TxRx_Addr.TX_ADDR[i]; //配置接收地址
    }
    /*默认通道配置*/
    //clienserver->Reserved = 0x00;      //清空标记
    clienserver->servercmd = tx_config;      //标记,表示该数据包为发送配置数据
    xQueueReceive(nrf_sign_queue,&nrfsign,portMAX_DELAY);
    if(nrfsign == 0x01)ESP_LOGI(TAG,"配置发送模式成功");     
    //clienserver->servercmd = tx_configdata;      
    
    ESP_LOGI(TAG,"Universal_channel_broadcast!");

    HAL_Getconncheck_(conncheck,sliencecheck);
    ESP_LOGI(TAG,"conncheck:%s",conncheck);
    if(strcmp(conncheck,"1") == 0)
    {
        //clienserver->datacmd = Heartbeat_pack;
        HAL_GetDeviceName_(DEVICE_NAME_,"aliyun-key");
        RF_channel_generation(clienserver);         //查找干净的射频通道
        clienservertemp.rf_zh = clienserver->rf_zh;
        *clienserver = clienservertemp;
        memcpy(clienserver->ClientServer_addr, address_generation().ClientServer_addr, sizeof(clienserver->ClientServer_addr));
        memcpy(clienserver->client_id,DEVICE_NAME_,strlen(DEVICE_NAME_));

        while(1)
        {
            clienserver->servercmd = tx_configdata;      
            xQueueReceive(nrf_sign_queue,&nrfsign,5000 / portTICK_PERIOD_MS);
            if(nrfsign == 0x03)
            {
                clienserver->datacmd = null_pack;
                // ESP_LOGI(TAG,"发送配置数据成功！");
                // ESP_LOGI(TAG,"clienserver.nrf24l01_RW_cmd:%x",clienserver->nrf24l01_RW_cmd);
                // ESP_LOGI(TAG,"clienserver.datacmd:%x",clienserver->datacmd);
                // ESP_LOGI(TAG,"clienserver.rf_zh:%d",clienserver->rf_zh);
                // ESP_LOGI(TAG,"clienserver.size:%d",clienserver->size);
                // ESP_LOGI(TAG,"clienserver.servercmd:%x",clienserver->servercmd);
                // ESP_LOGI(TAG,"clienserver->login_flag:%x",clienserver->login_flag);
                // ESP_LOGI(TAG,"clienserver->topo_flag:%x",clienserver->topo_flag);
                // ESP_LOGI(TAG,"clienserver.ClientServer_addr[4]:%x",clienserver->ClientServer_addr[4]);
                // ESP_LOGI(TAG,"clienserver.ClientServer_addr[3]:%x",clienserver->ClientServer_addr[3]);
                // ESP_LOGI(TAG,"clienserver.ClientServer_addr[2]:%x",clienserver->ClientServer_addr[2]);
                // ESP_LOGI(TAG,"clienserver.ClientServer_addr[1]:%x",clienserver->ClientServer_addr[1]);
                // ESP_LOGI(TAG,"clienserver.ClientServer_addr[0]:%x",clienserver->ClientServer_addr[0]);
                break;
            }
            //vTaskDelay(5000 / portTICK_PERIOD_MS);
        }
    }
    else
    {
        clienserver->datacmd = Distribution_net_pack;
    }

    return clienserver->datacmd;
}
datacmdTypedef Universal_channel_monitor(ClientServerTypedef *clienserver)
{
    uint8_t i;
    uint8_t nrfsign = 0;
    static const char *TAG = "Universal_channel_monitor";
    NRF24L01_HandleTypeDef hnrfx = hnrf_Default;
    clienserver->datacmd = 0;
    clienserver->size = 0;
    clienserver->rf_zh = 2;           //24000MHz + rf_zh ,max:255
    for(i = 0; i < 5; i++)
    {
        clienserver->ClientServer_addr[i] = hnrfx.TxRx_Addr.TX_ADDR[i]; //配置接收地址
    }

    /*默认通道配置*/
    //clienserver->Reserved = 0x00;      //清空标记
    clienserver->servercmd = rx_config;      //标记,表示该数据包为接收配置数据
    //ESP_LOGI(TAG,"fffffffffffffffffffffffff");
    if(xQueueReceive(nrf_sign_queue,&nrfsign,1000 / portTICK_PERIOD_MS))
    {
        if(nrfsign == 0x02)
        {
            ESP_LOGI(TAG,"配置接收模式成功");  
            //vTaskDelay(500 / portTICK_PERIOD_MS);    
            clienserver->servercmd = rx_configdata;      
            //ESP_LOGI(TAG,"loginflag:%d",clienserver->login_flag);
            while((dataeventl) != 1)
            {
                xQueueReceive(nrf_sign_queue,&nrfsign,10 / portTICK_PERIOD_MS);
                if(nrfsign == 0x10)
                {
                    
                    ESP_LOGI(TAG,"接收配置数据成功！");
                    // ESP_LOGI(TAG,"clienserver.nrf24l01_RW_cmd:%x",clienserver->nrf24l01_RW_cmd);
                    // ESP_LOGI(TAG,"clienserver.datacmd:%x",clienserver->datacmd);
                    // ESP_LOGI(TAG,"clienserver.rf_zh:%d",clienserver->rf_zh);
                    // ESP_LOGI(TAG,"clienserver.size:%d",clienserver->size);
                    // ESP_LOGI(TAG,"clienserver.servercmd:%x",clienserver->servercmd);
                    // ESP_LOGI(TAG,"clienserver->login_flag:%x",clienserver->login_flag);
                    // ESP_LOGI(TAG,"clienserver->topo_flag:%x",clienserver->topo_flag);
                    // ESP_LOGI(TAG,"clienserver.ClientServer_addr[4]:%x",clienserver->ClientServer_addr[4]);
                    // ESP_LOGI(TAG,"clienserver.ClientServer_addr[3]:%x",clienserver->ClientServer_addr[3]);
                    // ESP_LOGI(TAG,"clienserver.ClientServer_addr[2]:%x",clienserver->ClientServer_addr[2]);
                    // ESP_LOGI(TAG,"clienserver.ClientServer_addr[1]:%x",clienserver->ClientServer_addr[1]);
                    // ESP_LOGI(TAG,"clienserver.ClientServer_addr[0]:%x",clienserver->ClientServer_addr[0]);
                    break;
                }
            }
        }

    }
    else
    {
        ESP_LOGI(TAG,"配置接收模式失败！"); 
    }
    
    return clienserver->datacmd;
}
// NRF_WorkStatusTypeDef Detection_channel(ClientServerTypedef *clienserver)
// {

//     return NRF_OK;
// }

datacmdTypedef Normal_communication_rx(ClientServerTypedef *clienserver)
{
    static char *TAG = "Normal_communication_rx";
    uint8_t nrfsign = 0;
    clienserver->size = 0;
    clienserver->servercmd = rx_config;      //标记,表示该数据包为接收配置数据
    if(xQueueReceive(nrf_sign_queue,&nrfsign,1000 / portTICK_PERIOD_MS))
    {
        if(nrfsign == 0x02)
        {
            ESP_LOGI(TAG,"配置接收模式成功");  
            // ESP_LOGI(TAG,"size:%d",clienserver->size);
            // ESP_LOGI(TAG,"clienserver.nrf24l01_RW_cmd:%x",clienserver->nrf24l01_RW_cmd);
            // ESP_LOGI(TAG,"clienserver.datacmd:%x",clienserver->datacmd);
            // ESP_LOGI(TAG,"clienserver.rf_zh:%d",clienserver->rf_zh);
            // ESP_LOGI(TAG,"clienserver.size:%d",clienserver->size);
            // ESP_LOGI(TAG,"clienserver->login_flag:%x",clienserver->login_flag);
            // ESP_LOGI(TAG,"clienserver->topo_flag:%x",clienserver->topo_flag);
            // ESP_LOGI(TAG,"clienserver.ClientServer_addr[4]:%x",clienserver->ClientServer_addr[4]);
            // ESP_LOGI(TAG,"clienserver.ClientServer_addr[3]:%x",clienserver->ClientServer_addr[3]);
            // ESP_LOGI(TAG,"clienserver.ClientServer_addr[2]:%x",clienserver->ClientServer_addr[2]);
            // ESP_LOGI(TAG,"clienserver.ClientServer_addr[1]:%x",clienserver->ClientServer_addr[1]);
            // ESP_LOGI(TAG,"clienserver.ClientServer_addr[0]:%x",clienserver->ClientServer_addr[0]);
            clienserver->servercmd = rx_configdata;      //标记,表示该数据包为接收配置数据

            if(xQueueReceive(nrf_sign_queue,&nrfsign,16000 / portTICK_PERIOD_MS))
            {
                if(nrfsign == 0x10)
                {

                    ESP_LOGI(TAG,"接收配置数据成功！ size:%d",clienserver->size);
                    // ESP_LOGI(TAG,"clienserver.nrf24l01_RW_cmd:%x",clienserver->nrf24l01_RW_cmd);
                    // ESP_LOGI(TAG,"clienserver.datacmd:%x",clienserver->datacmd);
                    // ESP_LOGI(TAG,"clienserver.rf_zh:%d",clienserver->rf_zh);
                    // ESP_LOGI(TAG,"clienserver.size:%d",clienserver->size);
                    // ESP_LOGI(TAG,"clienserver->login_flag:%x",clienserver->login_flag);
                    // ESP_LOGI(TAG,"clienserver->topo_flag:%x",clienserver->topo_flag);
                    // ESP_LOGI(TAG,"clienserver.ClientServer_addr[4]:%x",clienserver->ClientServer_addr[4]);
                    // ESP_LOGI(TAG,"clienserver.ClientServer_addr[3]:%x",clienserver->ClientServer_addr[3]);
                    // ESP_LOGI(TAG,"clienserver.ClientServer_addr[2]:%x",clienserver->ClientServer_addr[2]);
                    // ESP_LOGI(TAG,"clienserver.ClientServer_addr[1]:%x",clienserver->ClientServer_addr[1]);
                    // ESP_LOGI(TAG,"clienserver.ClientServer_addr[0]:%x",clienserver->ClientServer_addr[0]);

                    clienserver->servercmd = rx_data_;
                    uint16_t xReceivedBytes;
                    uint16_t i = clienserver->size;
                    char *receive_Triad = NULL;
                    receive_Triad = (char *)calloc(1,clienserver->size + 1);
                    char *receive_Triad_p = receive_Triad;
                    vTaskDelay(10 / portTICK_PERIOD_MS);
                    while(i)
                    {
                        xReceivedBytes = xStreamBufferReceive( stream_buf,( void * ) receive_Triad_p,clienserver->size,2000 / portTICK_RATE_MS );
                        if(xReceivedBytes > 0)
                        {
                            ESP_LOGI(TAG,"Json,len:%d:%s",xReceivedBytes,(char *)receive_Triad_p);  
                            receive_Triad_p = receive_Triad_p + xReceivedBytes;
                            i = i - xReceivedBytes;
                        }
                        else
                        {
                            break;
                        }
                        
                    }
                    //ESP_LOGI(TAG,"&&&&&&&&&&&&&&&&&&&&&&");
                    if(xQueueReceive(nrf_sign_queue,&nrfsign,100 / portTICK_PERIOD_MS))
                    {
                        if(nrfsign == 0x20)ESP_LOGI(TAG,"接收数据成功！");  
                        analysis_data(clienserver,receive_Triad,clienserver->size);
                        ESP_LOGI(TAG,"DEVICE_NAME ID:%s",clienserver->client_id); 
                    }
                    else
                    {
                        ESP_LOGI(TAG,"接收数据失败！");
                    }
                }
            }
        }
    }

    clienserver->servercmd = null_status;
    return clienserver->datacmd;
}

datacmdTypedef Normal_communication_tx(ClientServerTypedef *clienserver)
{
    static char *TAG = "Normal_communication_tx";
    uint8_t nrfsign = 0;
    uint8_t i = 5;
    head->clientserver.datacmd = clienserver->datacmd;
    *clienserver = head->clientserver;
    clienserver->servercmd = tx_config;      //标记,表示该数据包为发送配置数据
    if(xQueueReceive(nrf_sign_queue,&nrfsign,3000 / portTICK_PERIOD_MS))
    {
        if(nrfsign == 0x01)
        {
            ESP_LOGI(TAG,"配置发送模式成功");  

            //vTaskDelay(100 / portTICK_PERIOD_MS);
            while(i--)
            {
                clienserver->servercmd = tx_configdata;      
                if(xQueueReceive(nrf_sign_queue,&nrfsign,1000 / portTICK_PERIOD_MS))
                {
                    if(nrfsign == 0x03)
                    {
                        clienserver->datacmd = null_pack;
                        ESP_LOGI(TAG,"发送配置数据成功！");
                        //vTaskDelay(100 / portTICK_PERIOD_MS);
                        clienserver->servercmd = tx_data_;
                        head->Buflist = DataOutList(head->Buflist,&stream_buf);
                        xQueueReceive(nrf_sign_queue,&nrfsign,5000 / portTICK_PERIOD_MS);
                        if(nrfsign == 0x08)ESP_LOGI(TAG,"发送数据成功！");
                        //Normal_communication_rx(clienserver);
                        break;
                    }
                    if(nrfsign == 0x05)
                    {
                        ESP_LOGI(TAG,"clienserver.nrf24l01_RW_cmd:%x",clienserver->nrf24l01_RW_cmd);
                        ESP_LOGI(TAG,"clienserver.datacmd:%x",clienserver->datacmd);
                        ESP_LOGI(TAG,"clienserver.rf_zh:%d",clienserver->rf_zh);
                        ESP_LOGI(TAG,"clienserver.size:%d",clienserver->size);
                        ESP_LOGI(TAG,"clienserver.servercmd:%x",clienserver->servercmd);
                        ESP_LOGI(TAG,"clienserver->login_flag:%x",clienserver->login_flag);
                        ESP_LOGI(TAG,"clienserver->topo_flag:%x",clienserver->topo_flag);
                        ESP_LOGI(TAG,"clienserver.ClientServer_addr[4]:%x",clienserver->ClientServer_addr[4]);
                        ESP_LOGI(TAG,"clienserver.ClientServer_addr[3]:%x",clienserver->ClientServer_addr[3]);
                        ESP_LOGI(TAG,"clienserver.ClientServer_addr[2]:%x",clienserver->ClientServer_addr[2]);
                        ESP_LOGI(TAG,"clienserver.ClientServer_addr[1]:%x",clienserver->ClientServer_addr[1]);
                        ESP_LOGI(TAG,"clienserver.ClientServer_addr[0]:%x",clienserver->ClientServer_addr[0]);
                        vTaskDelay(1000 / portTICK_PERIOD_MS);
                    }
                }
            }
        }
    }

    clienserver->size = 0;              //发送完数据记得清零
    return Data_pack;
}
