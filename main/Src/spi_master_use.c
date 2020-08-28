/* SPI Master example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "soc/gpio_struct.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "spi_master_use.h"

static const char *TAG = "spi_master_use";


#define PIN_NUM_MISO 25
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  19
#define PIN_NUM_CS   22


spi_device_handle_t  H_SPI_2;
// static void send_line_finish(spi_device_handle_t spi)
// {
//     spi_transaction_t *rtrans;
//     esp_err_t ret;
//     //Wait for all 6 transactions to be done and get back the results.
//     for (int x=0; x<6; x++) {
//         ret=spi_device_get_trans_result(spi, &rtrans, portMAX_DELAY);
//         assert(ret==ESP_OK);
//         //We could inspect rtrans now if we received any info back. The LCD is treated as write-only, though.
//     }
// }
void nrf_spi_post_transfer_callback(spi_transaction_t *trans)
{
    static uint16_t i=0;
    //ESP_LOGI(TAG,"hhhhhhh[%d]", i);
    i++;
}
void Spi_TxRx_callback(spi_transaction_t *t)
{
    esp_err_t error;
    error = spi_device_polling_transmit(H_SPI_2, t);
    assert(error == 0);  //Transmit!
}

void SPI_Init()
{
    esp_err_t ret;

    spi_bus_config_t buscfg={
        .miso_io_num=PIN_NUM_MISO,
        .mosi_io_num=PIN_NUM_MOSI,
        .sclk_io_num=PIN_NUM_CLK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=0
    };
    spi_device_interface_config_t devcfg={

        .clock_speed_hz=6*1000*1000,            //Clock out at 6 MHz
        .mode=0,                                //SPI mode 0
        .spics_io_num=PIN_NUM_CS,               //CS pin
        .queue_size=7,                          //We want to be able to queue 7 transactions at a time
    //    .pre_cb=nrf_spi_pre_transfer_callback,  //Specify pre-transfer callback to handle D/C line
        .post_cb=nrf_spi_post_transfer_callback
    };
    //Initialize the SPI bus
    ret=spi_bus_initialize(HSPI_HOST, &buscfg, 1);
    ESP_ERROR_CHECK(ret);
    //Attach the LCD to the SPI bus
    ret=spi_bus_add_device(HSPI_HOST, &devcfg, &H_SPI_2);
    ESP_ERROR_CHECK(ret);

    Spi_TxRx = Spi_TxRx_callback;
}
