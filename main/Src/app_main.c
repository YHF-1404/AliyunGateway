/* WiFi station Example
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "client_list.h"

#include "nrf_comm_use.h"
#include "LinkToAliyun.h"
#include "wifi_use.h"

#include "nrf24l01.h"

static const char *TAG = "app_main";

void app_main(void)
{
    Link_Aliyun();
    wifi_Init();

    // esp_log_level_set(TAG,ESP_LOG_DEBUG);
    xTaskCreate((void (*)(void *))nrf24l01_conn,"nrf24l01_conn",4096,NULL,10,&xHandle);
    vTaskSuspend(xHandle);  //挂起该任务
    while(1)
    {
        /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
        EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                pdTRUE,
                pdFALSE,
                portMAX_DELAY);
        
        /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
        * happened. */
        if (bits & WIFI_CONNECTED_BIT) 
        {
            ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                    EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
            //vTaskDelay(1000 / portTICK_PERIOD_MS);
            int32_t res = demo_mqtt_start(&mqtt_handle);        //启动mqqt
            if (res < 0) 
            {
                printf("demo_mqtt_start failed\n");
                abort();
            }
            else
            {
                vTaskResume(xHandle);       //运行任务
                if(xHandle1 != NULL)
                {
                    vTaskResume(xHandle1);       //运行任务
                }

            }
            
        } 
        else if (bits & WIFI_FAIL_BIT) 
        {
            ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                    EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);


        } 
        else 
        {
            ESP_LOGE(TAG, "UNEXPECTED EVENT");
        }
    }
    // ClientServerTypedef dsds = 
    // {
    //     .client_id = "yuyu",
    //     .nrf24l01_RW_cmd = 0xAA
    // };
    // ClientServerTypedef psps = 
    // {
    //     .client_id = "tytty",
    //     .nrf24l01_RW_cmd = 0xAA
    // };
    // ClientServerTypedef asas = 
    // {
    //     .client_id = "qwws",
    //     .nrf24l01_RW_cmd = 0xAA
    // };
    // head = InitList(head);
    // head = CutAllListItemData(head);
    // head = Found_ModifyListItemData(head,&dsds);
    // head = Found_ModifyListItemData(head,&psps);
    // head = Found_ModifyListItemData(head,&asas);
    // ESP_LOGI(TAG,"1");
    // CutListItemDataNum(head);
    // head = CutAllListItemData(head);
    // head = CutOneListItemData(head,"yuyu");
    // head = RW_cmdDeleItem(head);
    // ESP_LOGI(TAG,"2");
    // head = CutAllListItemData(head);
    // // head = FoundListItemData(head,"qwws");
    // // head->clientserver.nrf24l01_RW_cmd = 0xBB;
    // head = CutOneListItemData(head,"qwws");
    // head = RW_cmdDeleItem(head);
    // head = CleanAllRW_cmd(head);
    // ESP_LOGI(TAG,"3");
    // head = CutAllListItemData(head);
    // // head = FoundListItemData(head,"tytty");
    // // head->clientserver.nrf24l01_RW_cmd = 0xBB;
    // head = CutOneListItemData(head,"tytty");
    // head = RW_cmdDeleItem(head);
    // ESP_LOGI(TAG,"4");
    // head = CutAllListItemData(head);
    // while (1)
    // {
    //     sleap(1);
    //     head = CutAllListItemData(head);
    // }

}
