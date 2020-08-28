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
#include <string.h>
#include "nrf24l01.h"
#include "esp_log.h"
#include "driver/spi_master.h"
//#include "freertos/stream_buffer.h"




//#include "semphr.h"

static const char *TAG = "nrf24l01";

StreamBufferHandle_t stream_buf;
//List_t stream_buf;

xQueueHandle nrf_irq_queue = NULL;
//xQueueHandle irq_event_queue = NULL;
xQueueHandle nrf_data_tx_queue = NULL;
//xQueueHandle nrf_dataconfig_tx_queue = NULL;
xQueueHandle nrf_dataout_rx_queue = NULL;
xQueueHandle nrf_sign_queue = NULL;
xQueueHandle work_event_queue = NULL;
xQueueHandle nrf_clintserver_queue = NULL;
xQueueHandle nrf_dataconfig_queue = NULL;

uint8_t Tx_ADDR_Default[] = {0xAA,0xff,0xff,0xff,0xff};
uint8_t Rx_ADDR_P0_Default[] = {0xAA,0xff,0xff,0xff,0xff};
NRF24L01_HandleTypeDef hnrf_Default;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    uint8_t sta = 0;
    if(gpio_num == PIN_NUM_IRQ)
    {
        sta = nrf_get_irq_state();
        Clean_nrf_irq(sta);//清中断
        xQueueSendToBackFromISR(nrf_irq_queue, &sta, NULL);
    }
}



void NRF24L01_Init(void)
{
    gpio_config_t nrf_irq;
    nrf_irq.pin_bit_mask = PIN_NUM_IRQ_MASK;
    nrf_irq.mode = GPIO_MODE_INPUT;
    nrf_irq.pull_up_en = GPIO_PULLUP_ENABLE;
    nrf_irq.pull_down_en = GPIO_PULLDOWN_DISABLE;
    nrf_irq.intr_type = GPIO_INTR_NEGEDGE;
    gpio_config(&nrf_irq);

    gpio_config_t nrf_ce;
    nrf_ce.pull_up_en = GPIO_PULLUP_DISABLE;
    nrf_ce.pull_down_en = GPIO_PULLUP_DISABLE;
    nrf_ce.pin_bit_mask = PIN_NUM_CE_MASK;
    nrf_ce.mode = GPIO_MODE_OUTPUT;
    nrf_irq.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&nrf_ce);
    //gpio_set_direction(PIN_NUM_CE, GPIO_MODE_OUTPUT);
    //create a queue to handle gpio event from isr

    nrf_irq_queue = xQueueCreate(3, sizeof(uint8_t));
    //irq_event_queue = xQueueCreate(10,sizeof(uint8_t));
    nrf_data_tx_queue = xQueueCreate(40,sizeof(uint64_t));
    nrf_dataconfig_queue = xQueueCreate(10,sizeof(uint64_t));
    nrf_dataout_rx_queue = xQueueCreate(40,sizeof(uint64_t));
    work_event_queue = xQueueCreate(10, sizeof(uint8_t));
    nrf_clintserver_queue = xQueueCreate(40, sizeof(ClientServerTypedef));
    nrf_sign_queue = xQueueCreate(1,sizeof(uint8_t));
    stream_buf = xStreamBufferCreate( 32000,1 );
    //xTaskCreate(nrf_irq_task,"nrf_irg_task",2048,NULL,10,NULL);


        //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(PIN_NUM_IRQ, gpio_isr_handler, (void*) PIN_NUM_IRQ);

    vTaskDelay(100 / portTICK_PERIOD_MS);
    nrf_spi_conn_test();


    hnrf_Default.Init.CONFIG = 0x0f;                //默认处于接收模式
    hnrf_Default.Init.EN_AA = 0x01;                 //接收数据后，只允许频道0自动应答
    hnrf_Default.Init.EN_RX_ADDR = 0x01;            //只允许频道0接收数据
    hnrf_Default.Init.SETUP_RETR = 0x1A;            //自动重发延迟为500+86us，重发次数10次
    hnrf_Default.Init.RF_CH = 0x02;                 //设置通道通信频率，工作通道频率可由以下公式计算得出：Fo=（2400+RF-CH）MHz.并且射频收发器工作的频率范围从2.400-2.525GHz
    hnrf_Default.Init.RF_SETUP = 0x0f;              //设置发射速率为2MHZ，发射功率为最大值0dB
    hnrf_Default.Init.SETUP_AW = 0x03;              //配置通信地址的长度，默认值时0x03,即地址长度为5字节
    hnrf_Default.TxRx_Addr.TX_ADDR = Tx_ADDR_Default;
    hnrf_Default.TxRx_Addr.RX_ADDR_P0 = Rx_ADDR_P0_Default;
    hnrf_Default.Rx_Pw.RX_PW_P0 = 0x20;             //接收通道0有效数据宽度设置为32bit
        
    NRF24L01_Config(&hnrf_Default);
    Power_Down();
    //ESP_LOGE(TAG,"NRF_power!");
    vTaskDelay(100 / portTICK_PERIOD_MS);
    Power_Up();
    vTaskDelay(100 / portTICK_PERIOD_MS);
    Clean_nrf_irq(0x70);//清中断


    //uint8_t event = 0;
    //xQueueSendToBack(work_event_queue, &event ,NULL);
}






void nrf_spi_conn_test(void)
{
    uint8_t i;
    uint8_t pt[5]={0x00,0x11,0x22,0x33,0x44};
    uint8_t pr[5]={0};
    NRF24L01_HandleTypeDef hnrfx;
    hnrfx = hnrf_Default;
    CE_L;
    hnrfx.cmd_data = w_register + rx_addr_p0;
    hnrfx.RW_Set = nrf_write;
    hnrfx.DataSize = 5;
    hnrfx.TxRx_Data = pt;
    RW_NRF24L01_DATA(&hnrfx); 
    vTaskDelay(1 / portTICK_PERIOD_MS);
    hnrfx.cmd_data = r_register + rx_addr_p0;
    hnrfx.RW_Set = nrf_read;
    hnrfx.DataSize = 5;
    hnrfx.TxRx_Data = pr;    
    RW_NRF24L01_DATA(&hnrfx); 
    for(i=0;i<5;i++)
    {
      if(pt[i] != pr[i])
      {
        ESP_LOGI(TAG,"pr[%d]:%x",i,pr[i]);
        ESP_LOGI(TAG,"nrf_spi_conn_error");
      }
      else ESP_LOGI(TAG,"nrf_spi_conn_successfull");
    }
}



void NRF_TransmitReceive(void)
{
    NRF24L01_HandleTypeDef hnrfx = hnrf_Default;


    ClientServerTypedef clientserver_receive;           //用于存储接收到的配置数据
    //ClientServerTypedef *clientserve_receive = NULL;
    //Conn_packetTypedef *conn_packet = NULL;
    ClientServerTypedef *clientserverconfig = NULL;   
    ClientServerTypedef *clientserverconfig__= NULL;  
    uint32_t data_pack = 0;
    uint16_t data_pack_last = 0;
    irq8_t sta = 0;
    uint8_t sign = 1;
    uint8_t stream_size = 0;
    uint8_t errornum = 0;
    uint8_t nrfreg = 0;
    while(1)
    {
        xQueueReceive(nrf_dataconfig_queue, &clientserverconfig, 10 / portTICK_RATE_MS);//用于传递地址
        clientserverconfig__ = clientserverconfig;
        // TAG = "TR";
        // ESP_LOGI(TAG,"运行状态,datapack:%d",data_pack);
        while(clientserverconfig->servercmd == tx_config)   //配置发送地址，通道
        {
            TAG = "Tx配置";
            clientserverconfig->servercmd = null_status;
            data_pack_last = clientserverconfig->size % pack_long;
            data_pack = (clientserverconfig->size / pack_long) + ((clientserverconfig->size % pack_long > 0) ? 1:0);
            //clientserver_receive = *clientserverconfig;
            hnrfx.TxRx_Addr.TX_ADDR = clientserverconfig->ClientServer_addr;
            hnrfx.TxRx_Addr.RX_ADDR_P0 = clientserverconfig->ClientServer_addr;
            hnrfx.Init.RF_CH = clientserverconfig->rf_zh;
            hnrfx.Init.CONFIG = hnrfx.Init.CONFIG & 0xFE;  //发送
            NRF24L01_Config(&hnrfx);
            // Read_nrf_reg(regconfig,&nrfreg);
            // ESP_LOGI(TAG,"regconfig:%x",nrfreg);
            // Read_nrf_reg(en_aa,&nrfreg);
            // ESP_LOGI(TAG,"en_aa:%x",nrfreg);
            // Read_nrf_reg(en_rx_addr,&nrfreg);
            // ESP_LOGI(TAG,"en_rx_addr:%x",nrfreg);
            // Read_nrf_reg(setup_aw,&nrfreg);
            // ESP_LOGI(TAG,"setup_aw地址宽度:%x",nrfreg);
            // Read_nrf_reg(setup_retr,&nrfreg);
            // ESP_LOGI(TAG,"setup_retr:%x",nrfreg);
            // Read_nrf_reg(rf_ch,&nrfreg);
            // ESP_LOGI(TAG,"rf_ch:%d",nrfreg);
            // Read_nrf_reg(rf_setup,&nrfreg);
            // ESP_LOGI(TAG,"rf_setup:%x",nrfreg);
            // Read_nrf_reg(nrfstatus,&nrfreg);
            // ESP_LOGI(TAG,"nrfstatus:%x",nrfreg);
            // Read_nrf_reg(observe_tx,&nrfreg);
            // ESP_LOGI(TAG,"observe_tx:%x",nrfreg);
            // Read_nrf_reg(cd_check,&nrfreg);
            // ESP_LOGI(TAG,"cd_check:%x",nrfreg);
            // Read_nrf_reg(rx_addr_p0,&nrfreg);
            // ESP_LOGI(TAG,"rx_addr_p0:%x",nrfreg);
            // Read_nrf_reg(tx_addr,&nrfreg);
            // ESP_LOGI(TAG,"tx_addr:%x",nrfreg);
            xQueueReset(nrf_irq_queue);
            Clean_nrf_tx_fifo();
            sign = 0x01;
            xQueueOverwrite(nrf_sign_queue, &sign);

            //ESP_LOGI(TAG,"clientserverconfig->servercmd:%x",clientserverconfig->servercmd);
            
        }
        while(clientserverconfig->servercmd == rx_config)   //配置接收地址，通道
        {
            TAG = "Rx配置";
            //ESP_LOGI(TAG,"uuuuuuuuuuuuuuuuuuuuuuuuuu");
            clientserverconfig->servercmd = null_status;
            data_pack_last = clientserverconfig->size % pack_long;
            data_pack = (clientserverconfig->size / pack_long) + ((clientserverconfig->size % pack_long > 0) ? 1:0);
            //clientserver_receive = *clientserverconfig;
            hnrfx.TxRx_Addr.TX_ADDR = clientserverconfig->ClientServer_addr;
            hnrfx.TxRx_Addr.RX_ADDR_P0 = clientserverconfig->ClientServer_addr;
            //ESP_LOGI(TAG,"clientserverconfig->ClientServer_addr[0]:%x",clientserverconfig->ClientServer_addr[0]);
            //ESP_LOGI(TAG,"hnrfx.TxRx_Addr.TX_ADDR[0]:%x",hnrfx.TxRx_Addr.TX_ADDR[0]);
            hnrfx.Init.RF_CH = clientserverconfig->rf_zh;
            hnrfx.Init.CONFIG = hnrfx.Init.CONFIG | 0x01; //接收
            NRF24L01_Config(&hnrfx);
            // Read_nrf_reg(regconfig,&nrfreg);
            // ESP_LOGI(TAG,"regconfig:%x",nrfreg);
            // Read_nrf_reg(en_aa,&nrfreg);
            // ESP_LOGI(TAG,"en_aa:%x",nrfreg);
            // Read_nrf_reg(en_rx_addr,&nrfreg);
            // ESP_LOGI(TAG,"en_rx_addr:%x",nrfreg);
            // Read_nrf_reg(setup_aw,&nrfreg);
            // ESP_LOGI(TAG,"setup_aw地址宽度:%x",nrfreg);
            // Read_nrf_reg(setup_retr,&nrfreg);
            // ESP_LOGI(TAG,"setup_retr:%x",nrfreg);
            // Read_nrf_reg(rf_ch,&nrfreg);
            // ESP_LOGI(TAG,"rf_ch:%d",nrfreg);
            // Read_nrf_reg(rf_setup,&nrfreg);
            // ESP_LOGI(TAG,"rf_setup:%x",nrfreg);
            // Read_nrf_reg(nrfstatus,&nrfreg);
            // ESP_LOGI(TAG,"nrfstatus:%x",nrfreg);
            // Read_nrf_reg(observe_tx,&nrfreg);
            // ESP_LOGI(TAG,"observe_tx:%x",nrfreg);
            // Read_nrf_reg(cd_check,&nrfreg);
            // ESP_LOGI(TAG,"cd_check:%x",nrfreg);
            // Read_nrf_reg(rx_addr_p0,&nrfreg);
            // ESP_LOGI(TAG,"rx_addr_p0:%x",nrfreg);
            // Read_nrf_reg(tx_addr,&nrfreg);
            // ESP_LOGI(TAG,"tx_addr:%x",nrfreg);
            xQueueReset(nrf_irq_queue);
            Clean_nrf_rx_fifo();
            sign = 0x02;
            xQueueOverwrite(nrf_sign_queue, &sign);

            //ESP_LOGI(TAG,"clientserverconfig->servercmd:%x",clientserverconfig->servercmd);
            
        }
        while(clientserverconfig->servercmd == tx_configdata)           ////进入发送模式发送配置数据
        {
            TAG = "Tx";
            ESP_LOGI(TAG,"data_pack_tx:%d",data_pack);
            clientserverconfig->nrf24l01_RW_cmd = w_tx_payload;
            Tx_Config(clientserverconfig);  
            CE_H;
            if(xQueueReceive(nrf_irq_queue, &sta, 2000 / portTICK_RATE_MS))
            {
                if(sta & TX_DS)
                {
                    clientserverconfig->servercmd = null_status; 
                    //sta = 0;
                    errornum = 0;
                    Clean_nrf_tx_fifo();
                    sign = 0x03;
                    xQueueOverwrite(nrf_sign_queue, &sign);
                    ESP_LOGI(TAG,"NRF_Transmit success!");
                }
                else if(sta & MAX_RT)
                {
                    errornum++;
                    TAG = "Tx";
                    ESP_LOGE(TAG,"严重丢包！，sta:%x",sta);
                    Clean_nrf_tx_fifo();
                    //sign = 0x04;
                    //xQueueOverwrite(nrf_sign_queue, &sign);
                    // if(errornum >= 100)
                    // {
                        clientserverconfig->servercmd = null_status; 
                        Clean_nrf_tx_fifo();
                        xQueueReset(nrf_irq_queue);
                        sign = 0x05;
                        xQueueOverwrite(nrf_sign_queue, &sign);
                        ESP_LOGE(TAG,"失去连接！");
                    //     errornum = 0;
                    break;
                }        
                else
                {
                    Clean_nrf_irq(0x70);//清中断
                    clientserverconfig->servercmd = null_status; 
                    TAG = "Tx";
                    ESP_LOGE(TAG,"中断数据异常！，sta:%x",sta);
                }
                       
            }
            else
            {
                Clean_nrf_irq(0x70);//清中断
                Power_Down();
                ESP_LOGE(TAG,"NRF24l01 error!");
                vTaskDelay(100 / portTICK_PERIOD_MS);
                Power_Up();
                vTaskDelay(100 / portTICK_PERIOD_MS);
                clientserverconfig->servercmd = null_status; 
            } 
            //ESP_LOGE(TAG,"clientserverconfig->servercmd:%x",clientserverconfig->servercmd);
        }
        while(data_pack && (clientserverconfig->servercmd == tx_data_))
        {
            clientserverconfig->data_packet_p->nrf24l01_RW_cmd = w_tx_payload;
            size_t xReceivedBytes = xStreamBufferReceive( stream_buf,
                                                        ( void * ) clientserverconfig->data_packet_p->datapack ,
                                                        sizeof( clientserverconfig->data_packet_p->datapack ),
                                                        2000 / portTICK_RATE_MS );
            if(xReceivedBytes == 0)
            {
                clientserverconfig->servercmd = null_status; 
                if(data_pack != 0)
                {
                    TAG = "Tx";
                    ESP_LOGE(TAG,"数据未到达！");
                }
            }
            while(xReceivedBytes > 0)
            {
                //CE_L;
                Tx_Data(clientserverconfig->data_packet_p);  
                CE_H;

                if(xQueueReceive(nrf_irq_queue, &sta, 2000 / portTICK_RATE_MS))
                {
                    if(sta & TX_DS)
                    {
                        TAG = "Tx";
                        //ESP_LOGI(TAG,"正确数据包:%d",data_pack);   
                        data_pack--;
                        //sign = 0x06;
                        //xQueueOverwrite(nrf_sign_queue, &sign);
                        xReceivedBytes = 0;
                        errornum = 0;
                        //Clean_nrf_tx_fifo();
                        //ESP_LOGI(TAG,"NRF_Transmit success!");
                    }
                    else if(sta & MAX_RT)
                    {
                        errornum++;
                        //sign = 0x07;
                        //xQueueOverwrite(nrf_sign_queue, &sign);
                        TAG = "Tx";
                        ESP_LOGE(TAG,"严重丢包！，sta:%x",sta);
                        Clean_nrf_tx_fifo();
                        // if(errornum >= 100)
                        // {
                        //     sign = 0x08;
                        //     xQueueOverwrite(nrf_sign_queue, &sign);
                            clientserverconfig->servercmd = null_status;
                            xReceivedBytes = 0;
                            Clean_nrf_tx_fifo();
                            xQueueReset(nrf_irq_queue);
                            ESP_LOGE(TAG,"失去连接！");
                        //     errornum = 0;
                        // }
                    }        
                    else
                    {
                        Clean_nrf_irq(0x70);//清中断
                        clientserverconfig->servercmd = null_status; 
                        TAG = "Tx";
                        ESP_LOGE(TAG,"中断数据异常！，sta:%x",sta);
                    }
                        
                }
                else
                {
                    Clean_nrf_irq(0x70);//清中断
                    Power_Down();
                    ESP_LOGE(TAG,"NRF24l01 error!");
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                    Power_Up();
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                }
            }
            ESP_LOGE(TAG,"发送数据．．．");
            if(data_pack == 0)
            {
                clientserverconfig->size = 0;
                clientserverconfig->servercmd = null_status;               
                sign = 0x08;
                xQueueOverwrite(nrf_sign_queue, &sign);  

            }
        }


        while((data_pack == 0) && (clientserverconfig->servercmd == rx_configdata))           ////进入接收模式接收配置数据
        {
            CE_H;
            if(xQueueReceive(nrf_irq_queue, &sta, 2000 / portTICK_RATE_MS))
            {
                if(sta & RX_DR)
                {   
                    clientserver_receive.nrf24l01_RW_cmd = r_rx_payload;
                    Rx_Config(&clientserver_receive);           //读取nrf24l01接收到的数据

                    clientserverconfig = clientserverconfig__;   //恢复指针
                    if(clientserver_receive.ClientServer_addr[4] != 0xFF)
                    {
                        ESP_LOGI(TAG,"接收配置数据错误！");   
                        break;
                    }
                    clientserver_receive.data_packet_p = clientserverconfig->data_packet_p;
                    clientserver_receive.login_flag = clientserverconfig->login_flag;
                    //ESP_LOGI(TAG,"login_flag:%d",clientserver_receive.login_flag); 
                    // if(clientserverconfig->Reserved == clientmode)
                    // {
                    //     clientserver_receive.servercmd = clientserver_receive.clientcmd;
                    //     clientserver_receive.Reserved = clientmode;
                    //     *clientserverconfig = clientserver_receive;
                    // }
                    // else
                    // {
                        clientserver_receive.servercmd = null_status;
                        *clientserverconfig = clientserver_receive;
                    //}
                    data_pack_last = clientserverconfig->size % pack_long;
                    data_pack = (clientserverconfig->size / pack_long) + ((clientserverconfig->size % pack_long > 0) ? 1:0);                  
                    clientserverconfig->servercmd = null_status; 
                    sign = 0x10;
                    xQueueOverwrite(nrf_sign_queue, &sign);  //发消息给数据处理任务              
                    TAG = "Rx";
                    //ESP_LOGI(TAG,"接收配置数据成功！");                   

                    break;
                }
                else 
                {
                    Clean_nrf_irq(0x70);//清中断
                    TAG = "Rx";
                    ESP_LOGE(TAG,"中断数据异常！，sta:%x",sta);
                }
            }
            else
            {
                Clean_nrf_irq(0x70);//清中断
                // Power_Down();
                // TAG = "Rx";
                // ESP_LOGE(TAG,"NRF24l01 error!");
                // vTaskDelay(100 / portTICK_PERIOD_MS);
                // Power_Up();
                // vTaskDelay(100 / portTICK_PERIOD_MS);
            }
            
            ESP_LOGI(TAG,"接收配置数据中...");
            // ESP_LOGI(TAG,"clientserverconfig->Reserved[0]:%x",clientserverconfig->Reserved[0]);
            // ESP_LOGI(TAG,"clientserverconfig地址:%p",&clientserverconfig->Reserved);
        }

        while(data_pack && (clientserverconfig->servercmd == rx_data_))
        {
            CE_H;
            if(xQueueReceive(nrf_irq_queue, &sta, 2000 / portTICK_RATE_MS))
            {
                if(sta & RX_DR)
                {
                    sta = 0;
                    clientserverconfig->data_packet_p->nrf24l01_RW_cmd = r_rx_payload;
                    Rx_Data(clientserverconfig->data_packet_p);           //读取nrf24l01接收到的数据  
                    TAG = "数据接收";
                     
                    ESP_LOGI(TAG,"正确数据包:%d",data_pack);    
                          
                    data_pack--; 
                    if((data_pack == 0)&&(data_pack_last > 0))
                    {
                        stream_size = data_pack_last;
                    }
                    else
                    {
                        stream_size = sizeof( clientserverconfig->data_packet_p->datapack );
                    }
  
                    size_t xBytesSent = xStreamBufferSend( stream_buf,
                                                        ( void * ) clientserverconfig->data_packet_p->datapack,
                                                        stream_size, 
                                                        2000 / portTICK_RATE_MS);
                    if(xBytesSent != (stream_size))
                    {
                        TAG = "Rx";
                        ESP_LOGI(TAG,"数据阻塞超时stream_size:%d",stream_size); 
                        ESP_LOGI(TAG,"数据阻塞超时xBytesSent:%d",xBytesSent); 
                    }
                    else
                    {

                        //ESP_LOGI(TAG,"Json,len:%d:%s",xBytesSent,clientserverconfig->data_packet_p->datapack);          
                    }
                    //clientserverconfig->data_packet_p++;
                    //xQueueSendToBack(nrf_dataout_rx_queue,&,portMAX_DELAY);  //发消息给数据处理任务 
                }
                else
                {
                    Clean_nrf_irq(0x70);//清中断
                    clientserverconfig->servercmd = null_status; 
                    TAG = "数据接收";
                    ESP_LOGE(TAG,"中断数据异常！，sta:%x",sta);
                }
            }
            TAG = "Rx";
            ESP_LOGI(TAG,"接收数据中...");
            if(data_pack == 0)
            {
                clientserverconfig->size = 0;
                clientserverconfig->servercmd = null_status; 
                sign = 0x20;
                xQueueOverwrite(nrf_sign_queue, &sign);  //发消息给数据处理任务    
            }
        }

        //error:
    }
}


/**
  * @brief  nrf24l01读＆写数据函数.
  * @param  hnrf: 通信设备句柄 
  * @retval nrf24l01工作状态
  */
NRF_WorkStatusTypeDef Tx_Config(ClientServerTypedef *config)
{
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length = 8*(pack_long + 1);      // 8 bits
    t.tx_buffer = (uint8_t *)config;               //The data is the cmd itself
    //t.rx_buffer = rxbuf_temp;
    Spi_TxRx(&t);

    return NRF_OK;
}  

NRF_WorkStatusTypeDef Rx_Config(ClientServerTypedef *config)
{
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length = 8*(pack_long + 1);      // 8 bits
    t.tx_buffer = (uint8_t *)config;               //The data is the cmd itself
    t.rx_buffer = (uint8_t *)config;
    Spi_TxRx(&t);
     return NRF_OK;
}   

NRF_WorkStatusTypeDef Tx_Data(Conn_packetTypedef *data)
{
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length = 8*(pack_long + 1);      // 8 bits
    t.tx_buffer = (uint8_t *)data;               //The data is the cmd itself
    // t.rx_buffer = rxbuf_temp;
    Spi_TxRx(&t);

    return NRF_OK;
}   

NRF_WorkStatusTypeDef Rx_Data(Conn_packetTypedef *data)
{
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length = 8*(pack_long + 1);      // 8 bits
    t.tx_buffer = (uint8_t *)data;               //The data is the cmd itself
    t.rx_buffer = (uint8_t *)data;
    Spi_TxRx(&t);

    return NRF_OK;
}   