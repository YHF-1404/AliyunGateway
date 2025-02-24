/**
  ******************************************************************************
  * @file    LinkToAliyun.c
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
/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "esp_log.h"





#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include "aiot_state_api.h"
#include "aiot_sysdep_api.h"
#include "aiot_mqtt_api.h"
#include "aiot_dm_api.h"
#include "aiot_subdev_api.h"
#include "cJSON.h"
#include "nrf_comm_use.h"
#include "LinkToAliyun.h"
#include "client_list.h"
#include "nvs_use.h"

static const char *TAG = "LinkToAliyun";
/* 位于portfiles/aiot_port文件夹下的系统适配函数集合 */
extern aiot_sysdep_portfile_t g_aiot_sysdep_portfile;

/* 位于external/ali_ca_cert.c中的服务器证书 */
extern const char *ali_ca_cert;

static pthread_t g_mqtt_process_thread;
static pthread_t g_mqtt_recv_thread;
static uint8_t g_mqtt_process_thread_running = 0;
static uint8_t g_mqtt_recv_thread_running = 0;

void *subdev_handle = NULL;
void *mqtt_handle = NULL;



/* TODO: 如果要关闭日志, 就把这个函数实现为空, 如果要减少日志, 可根据code选择不打印
 *
 * 例如: [1577589489.033][LK-0317] mqtt_basic_demo&a13FN5TplKq
 *
 * 上面这条日志的code就是0317(十六进制), code值的定义见core/aiot_state_api.h
 *
 */

/* 日志回调函数, SDK的日志会从这里输出 */
int32_t demo_state_logcb(int32_t code, char *message)
{
    printf("%s", message);
    return 0;
}

/* MQTT事件回调函数, 当网络连接/重连/断开时被触发, 事件定义见core/aiot_mqtt_api.h */
void demo_mqtt_event_handler(void *handle, const aiot_mqtt_event_t *event, void *userdata)
{
    switch (event->type) {
        /* SDK因为用户调用了aiot_mqtt_connect()接口, 与mqtt服务器建立连接已成功 */
        case AIOT_MQTTEVT_CONNECT: {
            printf("AIOT_MQTTEVT_CONNECT\n");
            /* TODO: 处理SDK建连成功, 不可以在这里调用耗时较长的阻塞函数 */
        }
        break;

        /* SDK因为网络状况被动断连后, 自动发起重连已成功 */
        case AIOT_MQTTEVT_RECONNECT: {
            printf("AIOT_MQTTEVT_RECONNECT\n");
            /* TODO: 处理SDK重连成功, 不可以在这里调用耗时较长的阻塞函数 */
        }
        break;

        /* SDK因为网络的状况而被动断开了连接, network是底层读写失败, heartbeat是没有按预期得到服务端心跳应答 */
        case AIOT_MQTTEVT_DISCONNECT: {
            char *cause = (event->data.disconnect == AIOT_MQTTDISCONNEVT_NETWORK_DISCONNECT) ? ("network disconnect") :
                          ("heartbeat disconnect");
            printf("AIOT_MQTTEVT_DISCONNECT: %s\n", cause);
            /* TODO: 处理SDK被动断连, 不可以在这里调用耗时较长的阻塞函数 */
        }
        break;

        default: {

        }
    }
}

/* MQTT默认消息处理回调, 当SDK从服务器收到MQTT消息时, 且无对应用户回调处理时被调用 */
void demo_mqtt_default_recv_handler(void *handle, const aiot_mqtt_recv_t *packet, void *userdata)
{
    switch (packet->type) {
        case AIOT_MQTTRECV_HEARTBEAT_RESPONSE: {
            printf("heartbeat response\n");
            /* TODO: 处理服务器对心跳的回应, 一般不处理 */
        }
        break;

        case AIOT_MQTTRECV_SUB_ACK: {
            printf("suback, res: -0x%04X, packet id: %d, max qos: %d |订阅成功！\n",
                   -packet->data.sub_ack.res, packet->data.sub_ack.packet_id, packet->data.sub_ack.max_qos);
            /* TODO: 处理服务器对订阅请求的回应, 一般不处理 */
        }
        break;

        case AIOT_MQTTRECV_PUB: {
            printf("pub, qos: %d, topic: %.*s |接收成功！\n", packet->data.pub.qos, packet->data.pub.topic_len, packet->data.pub.topic);
            //printf("pub, payload: %.*s\n", packet->data.pub.payload_len, packet->data.pub.payload);
            /* TODO: 处理服务器下发的业务报文 */

        }
        break;

        case AIOT_MQTTRECV_PUB_ACK: {
            printf("puback, packet id: %d |发布成功！\n", packet->data.pub_ack.packet_id);
            /* TODO: 处理服务器对QoS1上报消息的回应, 一般不处理 */
        }
        break;

        default: {

        }
    }
}
/* MQTT离线消息处理回调, 可注册这个回调给SDK, 当SDK从服务器收到离线MQTT消息时进入 */
void demo_mqtt_offline_recv_handler(void *handle, const aiot_mqtt_recv_t *packet, void *userdata)
{
    if (AIOT_MQTTRECV_PUB != packet->type) {
        return;
    }

    /* TODO: 根据packet参数中的topic和payload编写业务逻辑 */
    printf("pub, qos: %d, topic: %.*s |离线数据接收成功！\n", packet->data.pub.qos, packet->data.pub.topic_len, packet->data.pub.topic);
    //printf("pub, payload: %.*s\n", packet->data.pub.payload_len, packet->data.pub.payload);
}











// uint8_t dataeventl = 0;
// uint8_t *dataevent_p = &dataevent_;

/* 用户数据接收处理回调函数 */
static void demo_dm_recv_handler(void *dm_handle, const aiot_dm_recv_t *recv, void *userdata)
{
    cJSON *root = NULL;
    TAG = "demo_dm_recv_handler";
    ESP_LOGI(TAG,"type = %d\r\n", recv->type);
    ESP_LOGI(TAG,"type = %d\r\n", recv->type);
    switch (recv->type) {

        /* 属性上报, 事件上报, 获取期望属性值或者删除期望属性值的应答 */
        case AIOT_DMRECV_GENERIC_REPLY: {
            printf("msg_id = %d, code = %d, data = %.*s, message = %.*s\r\n",
                   recv->data.generic_reply.msg_id,
                   recv->data.generic_reply.code,
                   recv->data.generic_reply.data_len,
                   recv->data.generic_reply.data,
                   recv->data.generic_reply.message_len,
                   recv->data.generic_reply.message);
        }
        break;

        /* 属性设置 */
        case AIOT_DMRECV_PROPERTY_SET: 
        {
            ESP_LOGI(TAG,"msg_id = %ld, params = %.*s\r\n",
                   (unsigned long)recv->data.property_set.msg_id,
                   recv->data.property_set.params_len,
                   recv->data.property_set.params);

            /* TODO: 以下代码演示如何对来自云平台的属性设置指令进行应答, 用户可取消注释查看演示效果 */
            char *param = (char *)calloc(1,recv->data.property_set.params_len +1);
            memcpy(param,recv->data.generic_reply.data,recv->data.property_set.params_len);
            root = cJSON_Parse(param);
            if (!root)
            {
                ESP_LOGE(TAG,"JSON Parse Error");
            }
            char *out = cJSON_Print(root);
            ESP_LOGI(TAG,"json_len:%d:%s",strlen(out) ,out);
            //cJSON *dm_data = cJSON_GetObjectItem(root, "LightSwitch");

            head = FoundListItemData(head,(char *)recv->device_name);
            if(head->error == 1)
            {
                head->error = 0;
                ESP_LOGE(TAG,"数据错误!");
            }
            else
            {
                head->Buflist = DataIntoList(head->Buflist,(uint8_t *)recv->data.property_set.params,recv->data.property_set.params_len);
                head->clientserver.size = recv->data.property_set.params_len;
                //dataevent_ = 1;
                // xQueueSend(work_event_queue,&dataevent_p,NULL);
                // printf("dataevent_addr:%p\n",&dataevent_);
                dataeventl = 1;
            }
            
            aiot_dm_msg_t msg;

            memset(&msg, 0, sizeof(aiot_dm_msg_t));
            msg.type = AIOT_DMMSG_PROPERTY_SET_REPLY;
            msg.data.property_set_reply.msg_id = recv->data.property_set.msg_id;
            msg.data.property_set_reply.code = 200;
            msg.data.property_set_reply.data = param;
            msg.product_key = recv->product_key;
            msg.device_name = recv->device_name;
            int32_t res = aiot_dm_send(dm_handle, &msg);
            if (res < 0) 
            {
                printf("aiot_dm_send failed\r\n");
            }
            
            
        }
        break;

        /* 异步服务调用 */
        case AIOT_DMRECV_ASYNC_SERVICE_INVOKE: {
            printf("msg_id = %ld, service_id = %s, params = %.*s\r\n",
                   (unsigned long)recv->data.async_service_invoke.msg_id,
                   recv->data.async_service_invoke.service_id,
                   recv->data.async_service_invoke.params_len,
                   recv->data.async_service_invoke.params);

            /* TODO: 以下代码演示如何对来自云平台的异步服务调用进行应答, 用户可取消注释查看演示效果
             *
             * 注意: 如果用户在回调函数外进行应答, 需要自行保存msg_id, 因为回调函数入参在退出回调函数后将被SDK销毁, 不可以再访问到
             */

            /*
            {
                aiot_dm_msg_t msg;

                memset(&msg, 0, sizeof(aiot_dm_msg_t));
                msg.type = AIOT_DMMSG_ASYNC_SERVICE_REPLY;
                msg.data.async_service_reply.msg_id = recv->data.async_service_invoke.msg_id;
                msg.data.async_service_reply.code = 200;
                msg.data.async_service_reply.service_id = "ToggleLightSwitch";
                msg.data.async_service_reply.data = "{\"dataA\": 20}";
                int32_t res = aiot_dm_send(dm_handle, &msg);
                if (res < 0) {
                    printf("aiot_dm_send failed\r\n");
                }
            }
            */
        }
        break;

        /* 同步服务调用 */
        case AIOT_DMRECV_SYNC_SERVICE_INVOKE: {
            printf("msg_id = %ld, rrpc_id = %s, service_id = %s, params = %.*s\r\n",
                   (unsigned long)recv->data.sync_service_invoke.msg_id,
                   recv->data.sync_service_invoke.rrpc_id,
                   recv->data.sync_service_invoke.service_id,
                   recv->data.sync_service_invoke.params_len,
                   recv->data.sync_service_invoke.params);

            /* TODO: 以下代码演示如何对来自云平台的同步服务调用进行应答, 用户可取消注释查看演示效果
             *
             * 注意: 如果用户在回调函数外进行应答, 需要自行保存msg_id和rrpc_id字符串, 因为回调函数入参在退出回调函数后将被SDK销毁, 不可以再访问到
             */

            
            {
                aiot_dm_msg_t msg;

                memset(&msg, 0, sizeof(aiot_dm_msg_t));
                msg.type = AIOT_DMMSG_SYNC_SERVICE_REPLY;
                msg.data.sync_service_reply.rrpc_id = recv->data.sync_service_invoke.rrpc_id;
                msg.data.sync_service_reply.msg_id = recv->data.sync_service_invoke.msg_id;
                msg.data.sync_service_reply.code = 200;
                msg.data.sync_service_reply.service_id = "SetLightSwitchTimer";
                msg.data.sync_service_reply.data = "";
                int32_t res = aiot_dm_send(dm_handle, &msg);
                if (res < 0) {
                    printf("aiot_dm_send failed\r\n");
                }
            }
            
        }
        break;

        /* 下行二进制数据 */
        case AIOT_DMRECV_RAW_DATA: {
            printf("raw data len = %d\r\n", recv->data.raw_data.data_len);
            /* TODO: 以下代码演示如何发送二进制格式数据, 若使用需要有相应的数据透传脚本部署在云端 */
            /*
            {
                aiot_dm_msg_t msg;
                uint8_t raw_data[] = {0x01, 0x02};

                memset(&msg, 0, sizeof(aiot_dm_msg_t));
                msg.type = AIOT_DMMSG_RAW_DATA;
                msg.data.raw_data.data = raw_data;
                msg.data.raw_data.data_len = sizeof(raw_data);
                aiot_dm_send(dm_handle, &msg);
            }
            */
        }
        break;

        /* 二进制格式的同步服务调用, 比单纯的二进制数据消息多了个rrpc_id */
        case AIOT_DMRECV_RAW_SYNC_SERVICE_INVOKE: {
            printf("raw sync service rrpc_id = %s, data_len = %d\r\n",
                   recv->data.raw_service_invoke.rrpc_id,
                   recv->data.raw_service_invoke.data_len);
        }
        break;

        default:
            break;
    }
}

/* 属性上报函数演示 */
int32_t demo_send_property_post(void *dm_handle, char *params)
{
    aiot_dm_msg_t msg;

    memset(&msg, 0, sizeof(aiot_dm_msg_t));
    msg.type = AIOT_DMMSG_PROPERTY_POST;
    msg.data.property_post.params = params;

    return aiot_dm_send(dm_handle, &msg);
}

/* 事件上报函数演示 */
int32_t demo_send_event_post(void *dm_handle, char *event_id, char *params)
{
    aiot_dm_msg_t msg;

    memset(&msg, 0, sizeof(aiot_dm_msg_t));
    msg.type = AIOT_DMMSG_EVENT_POST;
    msg.data.event_post.event_id = event_id;
    msg.data.event_post.params = params;

    return aiot_dm_send(dm_handle, &msg);
}

/* 演示了获取属性LightSwitch的期望值, 用户可将此函数加入到main函数中运行演示 */
int32_t demo_send_get_desred_requset(void *dm_handle)
{
    aiot_dm_msg_t msg;

    memset(&msg, 0, sizeof(aiot_dm_msg_t));
    msg.type = AIOT_DMMSG_GET_DESIRED;
    msg.data.get_desired.params = "[\"LightSwitch\"]";

    return aiot_dm_send(dm_handle, &msg);
}

/* 演示了删除属性LightSwitch的期望值, 用户可将此函数加入到main函数中运行演示 */
int32_t demo_send_delete_desred_requset(void *dm_handle)
{
    aiot_dm_msg_t msg;

    memset(&msg, 0, sizeof(aiot_dm_msg_t));
    msg.type = AIOT_DMMSG_DELETE_DESIRED;
    msg.data.get_desired.params = "{\"LightSwitch\":{}}";

    return aiot_dm_send(dm_handle, &msg);
}










void demo_subdev_recv_handler(void *handle, const aiot_subdev_recv_t *packet, void *user_data)
{
    switch (packet->type) {
        case AIOT_SUBDEVRECV_TOPO_ADD_REPLY:
        case AIOT_SUBDEVRECV_TOPO_DELETE_REPLY:
        case AIOT_SUBDEVRECV_TOPO_GET_REPLY:
        case AIOT_SUBDEVRECV_BATCH_LOGIN_REPLY: 
        case AIOT_SUBDEVRECV_BATCH_LOGOUT_REPLY: 
        case AIOT_SUBDEVRECV_SUB_REGISTER_REPLY:
        case AIOT_SUBDEVRECV_PRODUCT_REGISTER_REPLY: {
            printf("msgid        : %d\n", packet->data.generic_reply.msg_id);
            printf("code         : %d\n", packet->data.generic_reply.code);
            printf("product key  : %s\n", packet->data.generic_reply.product_key);
            printf("device name  : %s\n", packet->data.generic_reply.device_name);
            printf("message      : %s\n", (packet->data.generic_reply.message == NULL)?("NULL"):(packet->data.generic_reply.message));
            printf("data         : %s\n", packet->data.generic_reply.data);
        }
        break;
        case AIOT_SUBDEVRECV_TOPO_CHANGE_NOTIFY: {
            printf("msgid        : %d\n", packet->data.generic_notify.msg_id);
            printf("product key  : %s\n", packet->data.generic_notify.product_key);
            printf("device name  : %s\n", packet->data.generic_notify.device_name);
            printf("params       : %s\n", packet->data.generic_notify.params);
        }
        break;
        default: {

        }
    }
}







/* 执行aiot_mqtt_process的线程, 包含心跳发送和QoS1消息重发 */
void *demo_mqtt_process_thread(void *args)
{
    int32_t res = STATE_SUCCESS;

    while (g_mqtt_process_thread_running) {
        res = aiot_mqtt_process(args);
        if (res == STATE_USER_INPUT_EXEC_DISABLED) {
            break;
        }
        sleep(1);
    }
    return NULL;
}

/* 执行aiot_mqtt_recv的线程, 包含网络自动重连和从服务器收取MQTT消息 */
void *demo_mqtt_recv_thread(void *args)
{
    int32_t res = STATE_SUCCESS;

    while (g_mqtt_recv_thread_running) {
        res = aiot_mqtt_recv(args);
        if (res < STATE_SUCCESS) {
            if (res == STATE_USER_INPUT_EXEC_DISABLED) {
                break;
            }
            sleep(1);
        }
    }
    return NULL;
}

int32_t demo_mqtt_start(void **handle)
{
    char PRODUCT_KEY_[IOTX_PRODUCT_KEY_LEN + 1] = {0};
    char DEVICE_NAME_[IOTX_DEVICE_NAME_LEN + 1] = {0};
    char DEVICE_SECRET_[IOTX_DEVICE_SECRET_LEN + 1] = {0};
    int32_t     res = STATE_SUCCESS;
    void       *dm_handle = NULL;
    void       *mqtt_handle = NULL;
    char       *url = "iot-as-mqtt.cn-shanghai.aliyuncs.com"; /* 阿里云平台上海站点的域名后缀 */
    char        host[100] = {0}; /* 用这个数组拼接设备连接的云平台站点全地址, 规则是 ${productKey}.iot-as-mqtt.cn-shanghai.aliyuncs.com */
    uint16_t    port = 443;      /* 无论设备是否使用TLS连接阿里云平台, 目的端口都是443 */
    aiot_sysdep_network_cred_t cred; /* 安全凭据结构体, 如果要用TLS, 这个结构体中配置CA证书等参数 */

    /* TODO: 替换为自己设备的三元组 */
    HAL_GetProductKey_(PRODUCT_KEY_,"aliyun-key");
    HAL_GetDeviceName_(DEVICE_NAME_,"aliyun-key");
    HAL_GetDeviceSecret_(DEVICE_SECRET_,"aliyun-key");   
    char *product_key       = PRODUCT_KEY_;
    char *device_name       = DEVICE_NAME_;
    char *device_secret     = DEVICE_SECRET_;

    /* 创建SDK的安全凭据, 用于建立TLS连接 */
    memset(&cred, 0, sizeof(aiot_sysdep_network_cred_t));
    cred.option = AIOT_SYSDEP_NETWORK_CRED_SVRCERT_CA;  /* 使用RSA证书校验MQTT服务端 */
    cred.max_tls_fragment = 16384; /* 最大的分片长度为16K, 其它可选值还有4K, 2K, 1K, 0.5K */
    cred.sni_enabled = 1;                               /* TLS建连时, 支持Server Name Indicator */
    cred.x509_server_cert = ali_ca_cert;                 /* 用来验证MQTT服务端的RSA根证书 */
    cred.x509_server_cert_len = strlen(ali_ca_cert);     /* 用来验证MQTT服务端的RSA根证书长度 */

    /* 创建1个MQTT客户端实例并内部初始化默认参数 */
    mqtt_handle = aiot_mqtt_init();
    if (mqtt_handle == NULL) {
        printf("aiot_mqtt_init failed\n");
        return -1;
    }

    /* TODO: 如果以下代码不被注释, 则例程会用TCP而不是TLS连接云平台 */
    /*
    {
        memset(&cred, 0, sizeof(aiot_sysdep_network_cred_t));
        cred.option = AIOT_SYSDEP_NETWORK_CRED_NONE;
    }
    */

    snprintf(host, 100, "%s.%s", product_key, url);
    /* 配置MQTT服务器地址 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_HOST, (void *)host);
    /* 配置MQTT服务器端口 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_PORT, (void *)&port);
    /* 配置设备productKey */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_PRODUCT_KEY, (void *)product_key);
    /* 配置设备deviceName */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_DEVICE_NAME, (void *)device_name);
    /* 配置设备deviceSecret */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_DEVICE_SECRET, (void *)device_secret);
    /* 配置网络连接的安全凭据, 上面已经创建好了 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_NETWORK_CRED, (void *)&cred);
    /* 配置MQTT默认消息接收回调函数 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_RECV_HANDLER, (void *)demo_mqtt_default_recv_handler);
    /* 配置MQTT事件回调函数 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_EVENT_HANDLER, (void *)demo_mqtt_event_handler);



    /* 创建DATA-MODEL实例 */
    dm_handle = aiot_dm_init();
    if (dm_handle == NULL) {
        printf("aiot_dm_init failed");
        return -1;
    }
    /* 配置MQTT实例句柄 */
    aiot_dm_setopt(dm_handle, AIOT_DMOPT_MQTT_HANDLE, mqtt_handle);
    /* 配置消息接收处理回调函数 */
    aiot_dm_setopt(dm_handle, AIOT_DMOPT_RECV_HANDLER, (void *)demo_dm_recv_handler);







    /* 与服务器建立MQTT连接 */
    res = aiot_mqtt_connect(mqtt_handle);
    if (res < STATE_SUCCESS) {
        /* 尝试建立连接失败, 销毁MQTT实例, 回收资源 */
        aiot_mqtt_deinit(&mqtt_handle);
        printf("aiot_mqtt_connect failed: -0x%04X\n", -res);
        return -1;
    }

    /* 创建一个单独的线程, 专用于执行aiot_mqtt_process, 它会自动发送心跳保活, 以及重发QoS1的未应答报文 */
    g_mqtt_process_thread_running = 1;
    res = pthread_create(&g_mqtt_process_thread, NULL, demo_mqtt_process_thread, mqtt_handle);
    if (res < 0) {
        printf("pthread_create demo_mqtt_process_thread failed: %d\n", res);
        aiot_mqtt_deinit(&mqtt_handle);
        return -1;
    }

    /* 创建一个单独的线程用于执行aiot_mqtt_recv, 它会循环收取服务器下发的MQTT消息, 并在断线时自动重连 */
    g_mqtt_recv_thread_running = 1;
    res = pthread_create(&g_mqtt_recv_thread, NULL, demo_mqtt_recv_thread, mqtt_handle);
    if (res < 0) {
        printf("pthread_create demo_mqtt_recv_thread failed: %d\n", res);
         g_mqtt_process_thread_running = 0;
        pthread_join(g_mqtt_process_thread, NULL);
        aiot_mqtt_deinit(&mqtt_handle);
        return -1;
    }

    *handle = mqtt_handle;

    subdev_handle = aiot_subdev_init();
    if (subdev_handle == NULL) {
        printf("aiot_subdev_init failed\n");
        demo_mqtt_stop(&mqtt_handle);
        //return -1;
    }

    aiot_subdev_setopt(subdev_handle, AIOT_SUBDEVOPT_MQTT_HANDLE, mqtt_handle);
    aiot_subdev_setopt(subdev_handle, AIOT_SUBDEVOPT_RECV_HANDLER, demo_subdev_recv_handler);

    return 0;
}

int32_t demo_mqtt_stop(void **handle)
{
    int32_t res = STATE_SUCCESS;
    void *mqtt_handle = NULL;

    mqtt_handle = *handle;

    g_mqtt_process_thread_running = 0;
    g_mqtt_recv_thread_running = 0;
    pthread_join(g_mqtt_process_thread, NULL);
    pthread_join(g_mqtt_recv_thread, NULL);

    res = aiot_subdev_deinit(&subdev_handle);
    if (res < STATE_SUCCESS) 
    {
        printf("aiot_subdev_deinit failed: -0x%04X\n", res);
    }

    /* 断开MQTT连接 */
    res = aiot_mqtt_disconnect(mqtt_handle);
    if (res < STATE_SUCCESS) 
    {
        aiot_mqtt_deinit(&mqtt_handle);
        printf("aiot_mqtt_disconnect failed: -0x%04X\n", -res);
        return -1;
    }

    /* 销毁MQTT实例 */
    res = aiot_mqtt_deinit(&mqtt_handle);
    if (res < STATE_SUCCESS) 
    {
        printf("aiot_mqtt_deinit failed: -0x%04X\n", -res);
        return -1;
    }


    return 0;
}






void Link_Aliyun(void)
{



    //int32_t res = STATE_SUCCESS;

    /* 配置SDK的底层依赖 */
    aiot_sysdep_set_portfile(&g_aiot_sysdep_portfile);
    /* 配置SDK的日志输出 */
    aiot_state_set_logcb(demo_state_logcb);

    // res = demo_mqtt_start(&mqtt_handle);
    // if (res < 0) {
    //     printf("demo_mqtt_start failed\n");
    //     //return -1;
    // }

    // subdev_handle = aiot_subdev_init();
    // if (subdev_handle == NULL) {
    //     printf("aiot_subdev_init failed\n");
    //     demo_mqtt_stop(&mqtt_handle);
    //     //return -1;
    // }

    // aiot_subdev_setopt(subdev_handle, AIOT_SUBDEVOPT_MQTT_HANDLE, mqtt_handle);
    // aiot_subdev_setopt(subdev_handle, AIOT_SUBDEVOPT_RECV_HANDLER, demo_subdev_recv_handler);

    // aiot_subdev_dev_t g_subdev[] = {
    //     {
    //         "a16WUR9F3K9",
    //         "k_03",
    //         "f1baae833a8396f04f527de73a0c240c",
    //         "Sy0pobIwMVexJNdL"
    //     },
    //     {
    //         "a16WUR9F3K9",
    //         "k_02",
    //         "2310a0bafd33255e34089068567eadc8",
    //         "Sy0pobIwMVexJNdL"
    //     },
    //     {
    //         "a16WUR9F3K9",
    //         "k_01",
    //         "012c7577307deae7dd9581d58f2d2978",
    //         "Sy0pobIwMVexJNdL"
    //     }
    // };

    // ESP_LOGI(TAG,"-------------1--------------");
    // res = aiot_subdev_send_topo_add(subdev_handle, &g_subdev[0], 1);
    // if (res < STATE_SUCCESS) {
    //     printf("aiot_subdev_send_topo_add failed, res: -0x%04X\n", -res);
    //     // aiot_subdev_deinit(&subdev_handle);
    //     // demo_mqtt_stop(&mqtt_handle);
    //     //return -1;
    // }
    // ESP_LOGI(TAG,"-------------1t--------------");
    // aiot_subdev_send_batch_login(subdev_handle, &g_subdev[0], 1);
    // if (res < STATE_SUCCESS) {
    //     printf("aiot_subdev_send_batch_login failed, res: -0x%04X\n", -res);
    //     aiot_subdev_deinit(&subdev_handle);
    //     demo_mqtt_stop(&mqtt_handle);
    //     //return -1;
    // }
    // ESP_LOGI(TAG,"-------------2--------------");
    // sleep(2);
    // res = aiot_subdev_send_topo_add(subdev_handle, &g_subdev[1], 1);
    // if (res < STATE_SUCCESS) {
    //     printf("aiot_subdev_send_topo_add failed, res: -0x%04X\n", -res);
    //     // aiot_subdev_deinit(&subdev_handle);
    //     // demo_mqtt_stop(&mqtt_handle);
    //     //return -1;
    // }
    // ESP_LOGI(TAG,"-------------2t--------------");
    // aiot_subdev_send_batch_login(subdev_handle, &g_subdev[1], 1);
    // if (res < STATE_SUCCESS) {
    //     printf("aiot_subdev_send_batch_login failed, res: -0x%04X\n", -res);
    //     aiot_subdev_deinit(&subdev_handle);
    //     demo_mqtt_stop(&mqtt_handle);
    //     //return -1;
    // }
    // ESP_LOGI(TAG,"-------------3--------------");
    // sleep(2);
    // res = aiot_subdev_send_topo_add(subdev_handle, &g_subdev[2], 1);
    // if (res < STATE_SUCCESS) {
    //     printf("aiot_subdev_send_topo_add failed, res: -0x%04X\n", -res);
    //     // aiot_subdev_deinit(&subdev_handle);
    //     // demo_mqtt_stop(&mqtt_handle);
    //     //return -1;
    // }
    // ESP_LOGI(TAG,"-------------3t--------------");
    // aiot_subdev_send_batch_login(subdev_handle, &g_subdev[2], 1);
    // if (res < STATE_SUCCESS) {
    //     printf("aiot_subdev_send_batch_login failed, res: -0x%04X\n", -res);
    //     aiot_subdev_deinit(&subdev_handle);
    //     demo_mqtt_stop(&mqtt_handle);
    //     //return -1;
    // }
    // sleep(20);
    // aiot_subdev_send_topo_delete(subdev_handle, &g_subdev[1], 1);
    // aiot_subdev_send_topo_delete(subdev_handle, g_subdev, sizeof(g_subdev)/sizeof(aiot_subdev_dev_t));
    // if (res < STATE_SUCCESS) {
    //     printf("aiot_subdev_send_topo_delete failed, res: -0x%04X\n", -res);
    //     aiot_subdev_deinit(&subdev_handle);
    //     demo_mqtt_stop(&mqtt_handle);
    //     return -1;
    // }

    // sleep(2);



    // sleep(2);

    // aiot_subdev_send_sub_register(subdev_handle, g_subdev, sizeof(g_subdev)/sizeof(aiot_subdev_dev_t));
    // if (res < STATE_SUCCESS) {
    //     printf("aiot_subdev_send_sub_register failed, res: -0x%04X\n", -res);
    //     aiot_subdev_deinit(&subdev_handle);
    //     demo_mqtt_stop(&mqtt_handle);
    //     return -1;
    // }

    // sleep(2);

    // aiot_subdev_send_product_register(subdev_handle, g_subdev, sizeof(g_subdev)/sizeof(aiot_subdev_dev_t));
    // if (res < STATE_SUCCESS) {
    //     printf("aiot_subdev_send_product_register failed, res: -0x%04X\n", -res);
    //     aiot_subdev_deinit(&subdev_handle);
    //     demo_mqtt_stop(&mqtt_handle);
    //     return -1;
    // }

    // sleep(2);

    // ESP_LOGI(TAG,"-------------5--------------");
    // aiot_subdev_send_topo_get(subdev_handle);
    // if (res < STATE_SUCCESS) {
    //     printf("aiot_subdev_send_topo_get failed, res: -0x%04X\n", -res);
    //     aiot_subdev_deinit(&subdev_handle);
    //     demo_mqtt_stop(&mqtt_handle);
    //     //return -1;
    // }
    // ESP_LOGI(TAG,"-------------6--------------");

    // aiot_subdev_send_batch_logout(subdev_handle, &g_subdev[0], 1);
    // if (res < STATE_SUCCESS) {
    //     printf("aiot_subdev_send_batch_login failed, res: -0x%04X\n", -res);
    //     aiot_subdev_deinit(&subdev_handle);
    //     demo_mqtt_stop(&mqtt_handle);
    //     //return -1;
    // }
    // sleep(2);
    // ESP_LOGI(TAG,"-------------7--------------");
    // aiot_subdev_send_topo_delete(subdev_handle, &g_subdev[0], 1);

    //xTaskCreate((void (*)(void *))nrf24l01_conn,"nrf24l01_conn",3000,NULL,10,NULL);
    sleep(2);
    //xTaskCreate((void (*)(void *))nrf24l01_conn,"nrf24l01_conn",3000,NULL,10,NULL);
    // aiot_subdev_send_batch_logout(subdev_handle, g_subdev, sizeof(g_subdev)/sizeof(aiot_subdev_dev_t));
    // if (res < STATE_SUCCESS) {
    //     printf("aiot_subdev_send_batch_logout failed, res: -0x%04X\n", -res);
    //     aiot_subdev_deinit(&subdev_handle);
    //     demo_mqtt_stop(&mqtt_handle);
    //     return -1;
    // }

    // while(1) {
    //     sleep(1);
    // }

    // res = aiot_subdev_deinit(&subdev_handle);
    // if (res < STATE_SUCCESS) {
    //     printf("aiot_subdev_deinit failed: -0x%04X\n", res);
    // }

    // res = demo_mqtt_stop(&mqtt_handle);
    // if (res < 0) {
    //     printf("demo_start_stop failed\n");
    //     //return -1;
    // }

}


