/**
  ******************************************************************************
  * @file    NRF24L01_MAL_nrf.c
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
#include "NRF24L01_MAL_nrf.h"
#include <string.h>
#include "nrf24l01.h"
#include "esp_log.h"
#include "freertos/task.h"
#include "freertos/FreeRTOS.h"

static const char *TAG = "NRF24L01_MAL_nrf";
//SemaphoreHandle_t xSemaphore = NULL;

NRF24L01_ConfigTypeDef nrfconfig;
NRF_WorkStatusTypeDef NRF24L01_Config(NRF24L01_HandleTypeDef *hnrf)
{
    static uint8_t addr_pw = 0;
    NRF_WorkStatusTypeDef errorcode = 0;
      /* Process Locked */
    __NRF_LOCK(&nrfconfig);

    /* Check the UART handle allocation */
    if (hnrf == NULL)
    {
        errorcode = NRF_ERROR;
        ESP_LOGI(TAG, "hnrf_error");
        goto error;
    }
    if (nrfconfig.WorkStatus == NRF_BUSY)
    {
        errorcode = NRF_BUSY;
        goto error;
    }
    nrfconfig.WorkStatus = NRF_BUSY;
    CE_L;
    if (hnrf->Init.CONFIG != nrfconfig.Handle.Init.CONFIG)
    {
        hnrf->cmd_data = w_register + regconfig;
        hnrf->RW_Set = nrf_write;
        hnrf->DataSize = 1;
        hnrf->TxRx_Data = &hnrf->Init.CONFIG;
        if(RW_NRF24L01_DATA(hnrf) == NRF_ERROR)
        {
            errorcode = NRF_ERROR;
            hnrf->errorcode = nrf_transmit_error;
            goto error;
        }
        else
        {
            hnrf->errorcode = nrf_transmit_ok;
            nrfconfig.Handle.Init.CONFIG = hnrf->Init.CONFIG;
        }
    }
    if (hnrf->Init.EN_AA != nrfconfig.Handle.Init.EN_AA)
    {
        hnrf->cmd_data = w_register + en_aa;
        hnrf->RW_Set = nrf_write;
        hnrf->DataSize = 1;
        hnrf->TxRx_Data = &hnrf->Init.EN_AA;
        if(RW_NRF24L01_DATA(hnrf) == NRF_ERROR)
        {
            errorcode = NRF_ERROR;
            hnrf->errorcode = nrf_transmit_error;
            goto error;
        }
        else
        {
            hnrf->errorcode = nrf_transmit_ok;
            nrfconfig.Handle.Init.EN_AA = hnrf->Init.EN_AA;
        }
    }
    if (hnrf->Init.EN_RX_ADDR != nrfconfig.Handle.Init.EN_RX_ADDR)
    {
        hnrf->cmd_data = w_register + en_rx_addr;
        hnrf->RW_Set = nrf_write;
        hnrf->DataSize = 1;
        hnrf->TxRx_Data = &hnrf->Init.EN_RX_ADDR;
        if(RW_NRF24L01_DATA(hnrf) == NRF_ERROR)
        {
            errorcode = NRF_ERROR;
            hnrf->errorcode = nrf_transmit_error;
            goto error;
        }
        else
        {
            hnrf->errorcode = nrf_transmit_ok;
            nrfconfig.Handle.Init.EN_RX_ADDR = hnrf->Init.EN_RX_ADDR;
        }
    }
    if (hnrf->Init.RF_CH != nrfconfig.Handle.Init.RF_CH)
    {
        hnrf->cmd_data = w_register + rf_ch;
        hnrf->RW_Set = nrf_write;
        hnrf->DataSize = 1;
        hnrf->TxRx_Data = &hnrf->Init.RF_CH;
        if(RW_NRF24L01_DATA(hnrf) == NRF_ERROR)
        {
            errorcode = NRF_ERROR;
            hnrf->errorcode = nrf_transmit_error;
            goto error;
        }
        else
        {
            hnrf->errorcode = nrf_transmit_ok;
            nrfconfig.Handle.Init.RF_CH = hnrf->Init.RF_CH;
        }
    }
    if (hnrf->Init.RF_SETUP != nrfconfig.Handle.Init.RF_SETUP)
    {
        hnrf->cmd_data = w_register + rf_setup;
        hnrf->RW_Set = nrf_write;
        hnrf->DataSize = 1;
        hnrf->TxRx_Data = &hnrf->Init.RF_SETUP;
        if(RW_NRF24L01_DATA(hnrf) == NRF_ERROR)
        {
            errorcode = NRF_ERROR;
            hnrf->errorcode = nrf_transmit_error;
            goto error;
        }
        else
        {
            hnrf->errorcode = nrf_transmit_ok;
            nrfconfig.Handle.Init.RF_SETUP = hnrf->Init.RF_SETUP;
        }
    }
    if (hnrf->Init.SETUP_AW != nrfconfig.Handle.Init.SETUP_AW)
    {
        hnrf->cmd_data = w_register + setup_aw;
        hnrf->RW_Set = nrf_write;
        hnrf->DataSize = 1;
        hnrf->TxRx_Data = &hnrf->Init.SETUP_AW;
        if(RW_NRF24L01_DATA(hnrf) == NRF_ERROR)
        {
            errorcode = NRF_ERROR;
            hnrf->errorcode = nrf_transmit_error;
            goto error;
        }
        else
        {
            if(hnrf->Init.SETUP_AW == 0x03) addr_pw = 5;
            hnrf->errorcode = nrf_transmit_ok;
            nrfconfig.Handle.Init.SETUP_AW = hnrf->Init.SETUP_AW;
        }
    }
    if (hnrf->Init.SETUP_RETR != nrfconfig.Handle.Init.SETUP_RETR)
    {
        hnrf->cmd_data = w_register + setup_retr;
        hnrf->RW_Set = nrf_write;
        hnrf->DataSize = 1;
        hnrf->TxRx_Data = &hnrf->Init.SETUP_RETR;
        if(RW_NRF24L01_DATA(hnrf) == NRF_ERROR)
        {
            errorcode = NRF_ERROR;
            hnrf->errorcode = nrf_transmit_error;
            goto error;
        }
        else
        {
            hnrf->errorcode = nrf_transmit_ok;
            nrfconfig.Handle.Init.SETUP_RETR = hnrf->Init.SETUP_RETR;
        }
    }
    /*接收通道有效数据宽度设置*/
    if (hnrf->Rx_Pw.RX_PW_P0 != nrfconfig.Handle.Rx_Pw.RX_PW_P0)
    {
        hnrf->cmd_data = w_register + rx_pw_p0;
        hnrf->RW_Set = nrf_write;
        hnrf->DataSize = 1;
        hnrf->TxRx_Data = &hnrf->Rx_Pw.RX_PW_P0;
        if(RW_NRF24L01_DATA(hnrf) == NRF_ERROR)
        {
            errorcode = NRF_ERROR;
            hnrf->errorcode = nrf_transmit_error;
            goto error;
        }
        else
        {
            hnrf->errorcode = nrf_transmit_ok;
            nrfconfig.Handle.Rx_Pw.RX_PW_P0 = hnrf->Rx_Pw.RX_PW_P0;
        }
    }
    if (hnrf->Rx_Pw.RX_PW_P1 != nrfconfig.Handle.Rx_Pw.RX_PW_P1)
    {
        hnrf->cmd_data = w_register + rx_pw_p1;
        hnrf->RW_Set = nrf_write;
        hnrf->DataSize = 1;
        hnrf->TxRx_Data = &hnrf->Rx_Pw.RX_PW_P1;
        if(RW_NRF24L01_DATA(hnrf) == NRF_ERROR)
        {
            errorcode = NRF_ERROR;
            hnrf->errorcode = nrf_transmit_error;
            goto error;
        }
        else
        {
            hnrf->errorcode = nrf_transmit_ok;
            nrfconfig.Handle.Rx_Pw.RX_PW_P1 = hnrf->Rx_Pw.RX_PW_P1;
        }
    }
    if (hnrf->Rx_Pw.RX_PW_P2 != nrfconfig.Handle.Rx_Pw.RX_PW_P2)
    {
        hnrf->cmd_data = w_register + rx_pw_p2;
        hnrf->RW_Set = nrf_write;
        hnrf->DataSize = 1;
        hnrf->TxRx_Data = &hnrf->Rx_Pw.RX_PW_P2;
        if(RW_NRF24L01_DATA(hnrf) == NRF_ERROR)
        {
            errorcode = NRF_ERROR;
            hnrf->errorcode = nrf_transmit_error;
            goto error;
        }
        else
        {
            hnrf->errorcode = nrf_transmit_ok;
            nrfconfig.Handle.Rx_Pw.RX_PW_P2 = hnrf->Rx_Pw.RX_PW_P2;
        }
    }
    if (hnrf->Rx_Pw.RX_PW_P3 != nrfconfig.Handle.Rx_Pw.RX_PW_P3)
    {
        hnrf->cmd_data = w_register + rx_pw_p3;
        hnrf->RW_Set = nrf_write;
        hnrf->DataSize = 1;
        hnrf->TxRx_Data = &hnrf->Rx_Pw.RX_PW_P3;
        if(RW_NRF24L01_DATA(hnrf) == NRF_ERROR)
        {
            errorcode = NRF_ERROR;
            hnrf->errorcode = nrf_transmit_error;
            goto error;
        }
        else
        {
            hnrf->errorcode = nrf_transmit_ok;
            nrfconfig.Handle.Rx_Pw.RX_PW_P3 = hnrf->Rx_Pw.RX_PW_P3;
        }
    }
    if (hnrf->Rx_Pw.RX_PW_P4 != nrfconfig.Handle.Rx_Pw.RX_PW_P4)
    {
        hnrf->cmd_data = w_register + rx_pw_p4;
        hnrf->RW_Set = nrf_write;
        hnrf->DataSize = 1;
        hnrf->TxRx_Data = &hnrf->Rx_Pw.RX_PW_P4;
        if(RW_NRF24L01_DATA(hnrf) == NRF_ERROR)
        {
            errorcode = NRF_ERROR;
            hnrf->errorcode = nrf_transmit_error;
            goto error;
        }
        else
        {
            hnrf->errorcode = nrf_transmit_ok;
            nrfconfig.Handle.Rx_Pw.RX_PW_P4 = hnrf->Rx_Pw.RX_PW_P4;
        }
    }
    if (hnrf->Rx_Pw.RX_PW_P5 != nrfconfig.Handle.Rx_Pw.RX_PW_P5)
    {
        hnrf->cmd_data = w_register + rx_pw_p5;
        hnrf->RW_Set = nrf_write;
        hnrf->DataSize = 1;
        hnrf->TxRx_Data = &hnrf->Rx_Pw.RX_PW_P5;
        if(RW_NRF24L01_DATA(hnrf) == NRF_ERROR)
        {
            errorcode = NRF_ERROR;
            hnrf->errorcode = nrf_transmit_error;
            goto error;
        }
        else
        {
            hnrf->errorcode = nrf_transmit_ok;
            nrfconfig.Handle.Rx_Pw.RX_PW_P5 = hnrf->Rx_Pw.RX_PW_P5;
        }
    }
    //ESP_LOGI(TAG,"TxRx_Addr.TX_ADDR[0]:%x",hnrf->TxRx_Addr.TX_ADDR[0]);
    //ESP_LOGI(TAG,"nrfconfig.Handle.TxRx_Addr.TX_ADDR:%x",nrfconfig.Handle.TxRx_Addr.TX_ADDR[0]);    
    /*发送＆接收通道地址设置*/ 
    // if (hnrf->TxRx_Addr.TX_ADDR != nrfconfig.Handle.TxRx_Addr.TX_ADDR)
    // {
        hnrf->cmd_data = w_register + tx_addr;
        hnrf->RW_Set = nrf_write;
        hnrf->DataSize = addr_pw;
        hnrf->TxRx_Data = hnrf->TxRx_Addr.TX_ADDR;
        //ESP_LOGI(TAG,"hnrfx.TxRx_Addr.TX_ADDR[0]:%x",hnrf->TxRx_Addr.TX_ADDR[0]);
        if(RW_NRF24L01_DATA(hnrf) == NRF_ERROR)
        {
            errorcode = NRF_ERROR;
            hnrf->errorcode = nrf_transmit_error;
            goto error;
        }
        else
        {
            hnrf->errorcode = nrf_transmit_ok;
            nrfconfig.Handle.TxRx_Addr.TX_ADDR = hnrf->TxRx_Addr.TX_ADDR;
        }
    // }
    // if (hnrf->TxRx_Addr.RX_ADDR_P0 != nrfconfig.Handle.TxRx_Addr.RX_ADDR_P0)
    // {
        hnrf->cmd_data = w_register + rx_addr_p0;
        hnrf->RW_Set = nrf_write;
        hnrf->DataSize = addr_pw;
        hnrf->TxRx_Data = hnrf->TxRx_Addr.RX_ADDR_P0;
        //ESP_LOGI(TAG,"hnrfx.TxRx_Addr.TX_ADDR[0]:%x",hnrf->TxRx_Addr.TX_ADDR[0]);
        if(RW_NRF24L01_DATA(hnrf) == NRF_ERROR)
        {
            errorcode = NRF_ERROR;
            hnrf->errorcode = nrf_transmit_error;
            goto error;
        }
        else
        {
            hnrf->errorcode = nrf_transmit_ok;
            nrfconfig.Handle.TxRx_Addr.RX_ADDR_P0 = hnrf->TxRx_Addr.RX_ADDR_P0;
        } 
    // }
    if (hnrf->TxRx_Addr.RX_ADDR_P1 != nrfconfig.Handle.TxRx_Addr.RX_ADDR_P1)
    {
        hnrf->cmd_data = w_register + rx_addr_p1;
        hnrf->RW_Set = nrf_write;
        hnrf->DataSize = addr_pw;
        hnrf->TxRx_Data = hnrf->TxRx_Addr.RX_ADDR_P1;
        if(RW_NRF24L01_DATA(hnrf) == NRF_ERROR)
        {
            errorcode = NRF_ERROR;
            hnrf->errorcode = nrf_transmit_error;
            goto error;
        }
        else
        {
            hnrf->errorcode = nrf_transmit_ok;
            nrfconfig.Handle.TxRx_Addr.RX_ADDR_P1 = hnrf->TxRx_Addr.RX_ADDR_P1;
        }
    }
    if (hnrf->TxRx_Addr.RX_ADDR_P2 != nrfconfig.Handle.TxRx_Addr.RX_ADDR_P2)
    {
        hnrf->cmd_data = w_register + rx_addr_p2;
        hnrf->RW_Set = nrf_write;
        hnrf->DataSize = addr_pw;
        hnrf->TxRx_Data = hnrf->TxRx_Addr.RX_ADDR_P2;
        if(RW_NRF24L01_DATA(hnrf) == NRF_ERROR)
        {
            errorcode = NRF_ERROR;
            hnrf->errorcode = nrf_transmit_error;
            goto error;
        }
        else
        {
            hnrf->errorcode = nrf_transmit_ok;
            nrfconfig.Handle.TxRx_Addr.RX_ADDR_P2 = hnrf->TxRx_Addr.RX_ADDR_P2;
        }
    }
    if (hnrf->TxRx_Addr.RX_ADDR_P3 != nrfconfig.Handle.TxRx_Addr.RX_ADDR_P3)
    {
        hnrf->cmd_data = w_register + rx_addr_p3;
        hnrf->RW_Set = nrf_write;
        hnrf->DataSize = addr_pw;
        hnrf->TxRx_Data = hnrf->TxRx_Addr.RX_ADDR_P3;
        if(RW_NRF24L01_DATA(hnrf) == NRF_ERROR)
        {
            errorcode = NRF_ERROR;
            hnrf->errorcode = nrf_transmit_error;
            goto error;
        }
        else
        {
            hnrf->errorcode = nrf_transmit_ok;
            nrfconfig.Handle.TxRx_Addr.RX_ADDR_P3 = hnrf->TxRx_Addr.RX_ADDR_P3;
        }
    }
    if (hnrf->TxRx_Addr.RX_ADDR_P4 != nrfconfig.Handle.TxRx_Addr.RX_ADDR_P4)
    {
        hnrf->cmd_data = w_register + rx_addr_p4;
        hnrf->RW_Set = nrf_write;
        hnrf->DataSize = addr_pw;
        hnrf->TxRx_Data = hnrf->TxRx_Addr.RX_ADDR_P4;
        if(RW_NRF24L01_DATA(hnrf) == NRF_ERROR)
        {
            errorcode = NRF_ERROR;
            hnrf->errorcode = nrf_transmit_error;
            goto error;
        }
        else
        {
            hnrf->errorcode = nrf_transmit_ok;
            nrfconfig.Handle.TxRx_Addr.RX_ADDR_P4 = hnrf->TxRx_Addr.RX_ADDR_P4;
        }
    }
    if (hnrf->TxRx_Addr.RX_ADDR_P5 != nrfconfig.Handle.TxRx_Addr.RX_ADDR_P5)
    {
        hnrf->cmd_data = w_register + rx_addr_p5;
        hnrf->RW_Set = nrf_write;
        hnrf->DataSize = addr_pw;
        hnrf->TxRx_Data = hnrf->TxRx_Addr.RX_ADDR_P5;
        if(RW_NRF24L01_DATA(hnrf) == NRF_ERROR)
        {
            errorcode = NRF_ERROR;
            hnrf->errorcode = nrf_transmit_error;
            goto error;
        }
        else
        {
            hnrf->errorcode = nrf_transmit_ok;
            nrfconfig.Handle.TxRx_Addr.RX_ADDR_P5 = hnrf->TxRx_Addr.RX_ADDR_P5;
        }
    }

    nrfconfig.WorkStatus = NRF_OK;
    error:

      /* Process Locked */
    __NRF_UNLOCK(&nrfconfig);
    return errorcode;
  
}

// NRF_WorkStatusTypeDef NRF_Transmit_Mode(NRF24L01_HandleTypeDef *hnrf)
// {
//     hnrf->Init.CONFIG = hnrf->Init.CONFIG & 0xFE;
//     if(NRF24L01_Config(hnrf) == NRF_ERROR) return NRF_ERROR;
//     ESP_LOGI(TAG,"set NRF_Transmit_Mode");
//     //vTaskDelay(1 / portTICK_PERIOD_MS);
//     return NRF_OK;
// }


// NRF_WorkStatusTypeDef NRF_Receive_Mode(NRF24L01_HandleTypeDef *hnrf)
// {
//     hnrf->Init.CONFIG = hnrf->Init.CONFIG | 0x01;
//     if(NRF24L01_Config(hnrf) == NRF_ERROR) return NRF_ERROR;
//     ESP_LOGE(TAG,"set NRF_Receive_Mode");
//     CE_H;

//     //vTaskDelay(1 / portTICK_PERIOD_MS);
//     return NRF_OK;
// }



NRF_WorkStatusTypeDef Read_nrf_reg(uint8_t reg,uint8_t *regval)
{
    NRF24L01_HandleTypeDef hnrfx;
    hnrfx.cmd_data = r_register + reg;
    hnrfx.RW_Set = nrf_read;
    hnrfx.DataSize = 1;
    hnrfx.TxRx_Data = regval;    
    if(RW_NRF24L01_DATA(&hnrfx) == NRF_ERROR)return NRF_ERROR; 
    return NRF_OK;
}

NRF_WorkStatusTypeDef Write_nrf_reg(NRF24L01_HandleTypeDef *hnrf, uint8_t reg, uint8_t *regval,uint8_t datasize)
{
    //CE_L;
    hnrf->cmd_data = w_register + reg;
    hnrf->TxRx_Data = regval;  
    hnrf->DataSize = datasize;  
    hnrf->RW_Set = nrf_write;
    if(RW_NRF24L01_DATA(hnrf) == NRF_ERROR) return NRF_ERROR;
    return NRF_OK;
}

irq8_t nrf_get_irq_state(void)
{
    irq8_t regval;
    Read_nrf_reg(nrfstatus,&regval);
    return regval;
}

NRF_WorkStatusTypeDef Clean_nrf_fifo(uint8_t cmd)
{
    CE_L;
    uint8_t regval = 0xFF;
    NRF24L01_HandleTypeDef hnrfx;
    hnrfx.cmd_data = cmd;
    hnrfx.RW_Set = nrf_write;
    hnrfx.DataSize = 1;  
    hnrfx.TxRx_Data = &regval;
    if(RW_NRF24L01_DATA(&hnrfx) == NRF_ERROR)return NRF_ERROR; 
    return NRF_OK;
}
void Clean_nrf_tx_fifo(void)
{
    if(Clean_nrf_fifo(flush_tx) == NRF_ERROR)
    {
        ESP_LOGE(TAG,"Clean_nrf_tx_fifo failed!");
    }
}

void Clean_nrf_rx_fifo(void)
{
    if(Clean_nrf_fifo(flush_rx) == NRF_ERROR)
    {
        ESP_LOGE(TAG,"Clean_nrf_rx_fifo failed!");
    }
}

void Clean_nrf_irq(irq8_t reg)
{
    NRF24L01_HandleTypeDef hnrfx;
    if(Write_nrf_reg(&hnrfx,nrfstatus,&reg,1) == NRF_ERROR)
    {
        ESP_LOGE(TAG,"Clean_nrf_irq failed!");
    }
}

void NRF_Standby1(void)
{
    CE_L;
    Clean_nrf_tx_fifo();
}

void Power_Down(void)
{
    NRF24L01_HandleTypeDef hnrfx;
    hnrfx = nrfconfig.Handle;
    hnrfx.Init.CONFIG = hnrfx.Init.CONFIG & 0xFD;
    //hnrfx.Init.CONFIG = hnrfx.Init.CONFIG & 0xFD;
    if(NRF24L01_Config(&hnrfx) == NRF_ERROR)
    {
        ESP_LOGE(TAG,"Power_Down failed!");
    }
}
void Power_Up(void)
{
    NRF24L01_HandleTypeDef hnrfx;
    hnrfx = nrfconfig.Handle;
    hnrfx.Init.CONFIG = hnrfx.Init.CONFIG | 0x02;
    //hnrfx.Init.CONFIG = hnrfx.Init.CONFIG & 0xFD;
    if(NRF24L01_Config(&hnrfx) == NRF_ERROR)
    {
        ESP_LOGE(TAG,"Power_Down failed!");
    }
}



/**
  * @brief  nrf24l01读＆写＆命令数据函数.
  * @param  hnrf: 通信设备句柄 
  * @retval nrf24l01工作状态
  */
NRF_WorkStatusTypeDef RW_NRF24L01_DATA(NRF24L01_HandleTypeDef *hnrf)
{
    uint8_t *txbuf_temp = NULL;
    uint8_t *rxbuf_temp = NULL;
    uint8_t *txbuf_temp_p = NULL;
    uint8_t *rxbuf_temp_p = NULL;
    uint8_t DataSize_t = 0;
    //xSemaphoreTake( xSemaphore, portMAX_DELAY );                // 获得资源的使用权
    if(hnrf->TxRx_Data == NULL) goto error; 
    txbuf_temp = (uint8_t *)calloc(1,hnrf->DataSize + 1);
    rxbuf_temp = (uint8_t *)calloc(1,hnrf->DataSize + 1);
    txbuf_temp_p = txbuf_temp;
    rxbuf_temp_p = rxbuf_temp;
    *txbuf_temp_p = hnrf->cmd_data;
    DataSize_t = hnrf->DataSize + 1;
    if(hnrf->RW_Set == nrf_write)
    {
      while(DataSize_t--)
      {
        //ESP_LOGI(TAG,"writeDate:%x",*txbuf_temp_p);
        txbuf_temp_p++;
        *txbuf_temp_p = *hnrf->TxRx_Data; 
        hnrf->TxRx_Data++;
      }
    }
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length = 8*(hnrf->DataSize + 1);      // 8 bits
    t.tx_buffer = txbuf_temp;               //The data is the cmd itself
    t.rx_buffer = rxbuf_temp;
    Spi_TxRx(&t);
    hnrf->Status = rxbuf_temp[0];

    DataSize_t = hnrf->DataSize;
    if(hnrf->RW_Set == nrf_read)
    {
      while(DataSize_t--)
      {
        rxbuf_temp_p++;
        *hnrf->TxRx_Data = *rxbuf_temp_p;
        //ESP_LOGI(TAG,"ReadDate:%x",*hnrf->TxRx_Data);
        hnrf->TxRx_Data++;
      }
    }
    else if(hnrf->RW_Set != nrf_write)
    {
      ESP_LOGI(TAG,"RW_Set error");
    }
    free((void*)txbuf_temp);
    free((void*)rxbuf_temp);
    txbuf_temp = NULL;
    rxbuf_temp = NULL;
    //xSemaphoreGive( xSemaphore );       //释放资源的使用权
    return NRF_OK;
    error:
    free((void*)txbuf_temp);
    free((void*)rxbuf_temp);
    txbuf_temp = NULL;
    rxbuf_temp = NULL;
    //xSemaphoreGive( xSemaphore );       //释放资源的使用权
    return NRF_ERROR;
}        


/************************ (C) COPYRIGHT YHF *****END OF FILE****/
