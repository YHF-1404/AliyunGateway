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
#include "client_list.h"
#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "esp_log.h"
// #include "aiot_subdev_api.h"
// #include "LinkToAliyun.h"
// #include "aiot_state_api.h"
// #include "nvs_use.h"
char *TAG = "client_list";
ClientList_Typedef *head = NULL;

/*双向循环列表初始化*/
ClientList_Typedef *InitList(ClientList_Typedef *head) 
{
    uint8_t i = 2;
    ClientList_Typedef *head_p = NULL;
    ClientList_Typedef *head_cp = NULL;
    head = (ClientList_Typedef *)calloc(1,sizeof(ClientList_Typedef));
    head_p = head;
    head_cp = head;
    while(i--)
    {      
        head = (ClientList_Typedef*)calloc(1,sizeof(ClientList_Typedef));
        head_p->next = head;
        head->previous = head_p;
        head_p = head;
    }
    head->next = head_cp;
    head_cp->previous = head;
    return head;
}

/*双向循环列表添加条目*/
ClientList_Typedef *InsertListItem(ClientList_Typedef *head,ClientServerTypedef *ClientList_p)
{
    if(head == NULL)
    {
        return head;
    }
    ClientList_Typedef *listtemp = (ClientList_Typedef *)calloc(1,sizeof(ClientList_Typedef));
    ClientList_Typedef *head_n = head->next;
    head->next = listtemp;
    listtemp->previous = head;
    listtemp->next = head_n;
    head_n->previous = listtemp;
    head = listtemp;
    head->clientserver = *ClientList_p;
    return head;
}
/*双向循环列表删除条目
删除之后head指针指向前一个条目
*/
ClientList_Typedef *DeleteListItem(ClientList_Typedef *head)
{
    if(head == NULL)
    {
        return head;
    }
    ClientList_Typedef *listtemp = head->previous;    
    head->previous->next = head->next;
    head->next->previous = head->previous;
    free(head);
    head = listtemp;
    return head;
}

ClientList_Typedef *DeleteAllListItem(ClientList_Typedef *head)
{
    if(head == NULL)
    {
        return head;
    }
    ClientList_Typedef *listtemp = head->previous;    
    while(listtemp != head->previous)
    {
        head->previous->next = head->next;
        head->next->previous = head->previous;
        head = head->previous;
        free(head);
    }
    return head;
}

/*双向循环列表根据client_id查找条目*/
ClientList_Typedef *FoundListItemData(ClientList_Typedef *head,char *str_p)
{
    if(head == NULL)
    {
        return head;
    }
    ClientList_Typedef *listtemp = head;
    while(strcmp(str_p,(char *)(head->clientserver.client_id)) != 0)
    {
        head = head->next;
        if(head == listtemp)
        {
            printf("没有该条目！:%s\r\n",str_p);
            head->error = 1;
            break;
        }
    }
    return head;
}
/*双向循环列表修改条目*/
ClientList_Typedef *ModifyListItemData(ClientList_Typedef *head,ClientServerTypedef *ClientList_p)
{
    if(head == NULL)
    {
        return head;
    }
    head->clientserver = *ClientList_p;
    return head;
}

/*双向循环列表根据client_id查找并修改条目，如果没有该条目便添加*/
ClientList_Typedef *Found_ModifyListItemData(ClientList_Typedef *head,ClientServerTypedef *ClientList_p)
{
    if(head == NULL)
    {
        return head;
    }
    ClientList_Typedef *listtemp = FoundListItemData(head,(char *)(ClientList_p->client_id));
    if(listtemp->error == 1)
    {
        listtemp->error = 0;
        printf("添加条目:%s\r\n",ClientList_p->client_id);
        return InsertListItem(listtemp,ClientList_p);
    }
    else
    {
        return ModifyListItemData(listtemp,ClientList_p);
    }
}

/*双向循环列表根据client_id查找并删除条目*/
ClientList_Typedef *Found_DeleteListItemData(ClientList_Typedef *head,char *str_p)
{
    if(head == NULL)
    {
        return head;
    }
    ClientList_Typedef *listtemp = FoundListItemData(head,str_p);
    if(listtemp->error == 1)
    {
        return listtemp;
    }
    else
    {
        return DeleteListItem(listtemp);
    }
}

/*双向循环列表输出所有条目的client_id*/
ClientList_Typedef *CutAllListItemData(ClientList_Typedef *head)
{
    if(head == NULL)
    {
        return head;
    }
    uint16_t i = 0;
    ClientList_Typedef *listtemp = head->previous;
    while(head != listtemp)
    {
        if(head->clientserver.client_id[0] != 0)
        {
            i++;
            printf("client_id[%d]:\t%s\n",i,head->clientserver.client_id);
        }
        head = head->next;
    }
    if(head->clientserver.client_id[0] != 0)
    {
        i++;
        printf("client_id[%d]:\t%s\n",i,head->clientserver.client_id);
    }
    if(i == 0)
    {
        printf("No Item!\n");
    }
    return head;
}
/*双向循环列表清除所有条目的nrf24l01_RW_cmd*/
ClientList_Typedef *CleanAllRW_cmd(ClientList_Typedef *head)
{
    if(head == NULL)
    {
        return head;
    }
    ClientList_Typedef *listtemp = head->previous;
    while(head != listtemp)
    {
        if(head->clientserver.client_id[0] != 0)
        {
            head->clientserver.nrf24l01_RW_cmd = 0xBB;
        }
        head = head->next;
    }
    return head;
}




/*双向循环列表根据nrf24l01_RW_cmd清除条目*/
ClientList_Typedef *RW_cmdDeleItem(ClientList_Typedef *head)
{
    if(head == NULL)
    {
        return head;
    }
    ClientList_Typedef *listtemp = head->previous;
    while(head != listtemp)
    {
        if(head->clientserver.client_id[0] != 0)
        {
            if(head->clientserver.nrf24l01_RW_cmd != 0xAA)
            {
                cleanTopo_cb(head);
                head->Buflist = DelInitDataList(head->Buflist);
                head = DeleteListItem(head);
            }
        }
        head = head->next;
    }
    if(head->clientserver.client_id[0] != 0)
    {
        if(head->clientserver.nrf24l01_RW_cmd != 0xAA)
        {
            cleanTopo_cb(head);
            head->Buflist = DelInitDataList(head->Buflist);
            head = DeleteListItem(head);
        }
    }
    return head;
}

/*双向循环列表输出条目数量*/
uint16_t CutListItemDataNum(ClientList_Typedef *head)
{
    if(head == NULL)
    {
        return 0;
    }
    uint16_t i = 0;
    ClientList_Typedef *listtemp = head->previous;
    while(head != listtemp)
    {
        if(head->clientserver.client_id[0] != 0)
        {
            i++;
        }
        head = head->next;
    }
    if(head->clientserver.client_id[0] != 0)
    {
        i++;
    }
    printf("ListItemNum:%d\r\n",i);
    return i;
}

/*双向循环列表根据client_id输出该条目所有数据*/
ClientList_Typedef *CutOneListItemData(ClientList_Typedef *head,char *str_p)
{
    if(head == NULL)
    {
        return head;
    }
    ClientList_Typedef *listtemp = FoundListItemData(head,str_p);
    if(listtemp->error == 1)
    {
        return listtemp;
    }
    else
    {
        char *TAG = "CutOneListItemData";
        ESP_LOGI(TAG,"clienserver.nrf24l01_RW_cmd:%x",listtemp->clientserver.nrf24l01_RW_cmd);
        ESP_LOGI(TAG,"listtemp->clientserver.client_id:%s",listtemp->clientserver.client_id);
        ESP_LOGI(TAG,"clienserver.datacmd:%x",listtemp->clientserver.datacmd);
        ESP_LOGI(TAG,"clienserver.rf_zh:%d",listtemp->clientserver.rf_zh);
        ESP_LOGI(TAG,"clienserver.size:%d",listtemp->clientserver.size);
        ESP_LOGI(TAG,"clienserver.servercmd:%x",listtemp->clientserver.servercmd);
        ESP_LOGI(TAG,"clienserver->login_flag:%x",listtemp->clientserver.login_flag);
        ESP_LOGI(TAG,"clienserver->topo_flag:%x",listtemp->clientserver.topo_flag);
        ESP_LOGI(TAG,"clienserver.ClientServer_addr[4]:%x",listtemp->clientserver.ClientServer_addr[4]);
        ESP_LOGI(TAG,"clienserver.ClientServer_addr[3]:%x",listtemp->clientserver.ClientServer_addr[3]);
        ESP_LOGI(TAG,"clienserver.ClientServer_addr[2]:%x",listtemp->clientserver.ClientServer_addr[2]);
        ESP_LOGI(TAG,"clienserver.ClientServer_addr[1]:%x",listtemp->clientserver.ClientServer_addr[1]);
        ESP_LOGI(TAG,"clienserver.ClientServer_addr[0]:%x",listtemp->clientserver.ClientServer_addr[0]);        
    }
    return head;
}

/****************************************************************************************************************************/

/*列表初始化*/
DataBuf_ListTypedef *InitDataList(DataBuf_ListTypedef *Buflist) 
{
    Buflist = (DataBuf_ListTypedef *)calloc(1,sizeof(DataBuf_ListTypedef));
    Buflist->datasize = 0;
    Buflist->previous = NULL;
    Buflist->next = NULL;
    Buflist->databuf = NULL;

    return Buflist;
}

/*数据入链表*/
DataBuf_ListTypedef *DataIntoList(DataBuf_ListTypedef *Buflist,uint8_t *data,uint32_t datasize) 
{
    if(Buflist == NULL)
    {
        return Buflist;
    }
    if(data == NULL||datasize == 0)
    {
        Buflist->error = 1;
        char *TAG = "DataIntoList";
        ESP_LOGE(TAG,"输入数据错误 ");     
        return Buflist;             
    }
    DataBuf_ListTypedef *bufheadtemp = Buflist->previous;
    while(Buflist != NULL)           //回溯到链表尾巴
    {
        bufheadtemp = Buflist;
        Buflist = Buflist->next;
    }
    Buflist = (DataBuf_ListTypedef *)calloc(1,sizeof(DataBuf_ListTypedef));
    bufheadtemp->next = Buflist;
    Buflist->previous = bufheadtemp;
    Buflist->next = NULL;
    Buflist->datasize = datasize;
    Buflist->databuf = (uint8_t *)calloc(1,datasize + 1);
    memcpy(Buflist->databuf,data,datasize);
    return Buflist;
}
/*数据出链表进入流缓冲器*/
DataBuf_ListTypedef *DataOutList(DataBuf_ListTypedef *Buflist,StreamBufferHandle_t *streambuf)
{
    if(Buflist == NULL)
    {
        return Buflist;
    }
    DataBuf_ListTypedef *bufheadtemp = Buflist->next;
    while(Buflist != NULL)      //回溯到链表头部
    {
        bufheadtemp = Buflist;
        Buflist = Buflist->previous;
    }
    if(bufheadtemp->next == NULL)
    {
        bufheadtemp->error = 1;
        TAG = "DataOutList";
        ESP_LOGE(TAG,"链表为空... ");     
        return bufheadtemp;      
    }
    Buflist = bufheadtemp->next;
    if(Buflist->datasize == 0)
    {
        TAG = "DataOutList";
        ESP_LOGE(TAG,"数据错误！ ");  
        return Buflist; 
    }
    //size_t xReceivedBytes = 
    xStreamBufferSend( *streambuf,( void * ) Buflist->databuf,Buflist->datasize,5000 / portTICK_RATE_MS );
    TAG = "DataOutList";
    ESP_LOGI(TAG,"dataout:%s ",Buflist->databuf); 
    //ESP_LOGI(TAG,"size:%d",Buflist->datasize);  
    if(Buflist->next != NULL)
    {
        Buflist->previous->next = Buflist->next;
        Buflist->next->previous = Buflist->previous;
    }
    else
    {
        Buflist->previous->next = NULL;
        Buflist->next = NULL;
        Buflist->previous = NULL;
    }
    
    free(Buflist->databuf);
    Buflist->databuf = NULL;
    free(Buflist);
    Buflist = bufheadtemp;

    return Buflist;
}

/*列表去初始化*/
DataBuf_ListTypedef *DelInitDataList(DataBuf_ListTypedef *Buflist)
{
    if(Buflist == NULL)
    {
        return Buflist;
    }
    DataBuf_ListTypedef *bufheadtemp = NULL;
    while(Buflist->previous != NULL)      //回溯到链表头部
    {
        Buflist = Buflist->previous;
    }    
    while(Buflist != NULL)
    {
        free(Buflist->databuf);
        Buflist->databuf = NULL;
        Buflist->error = 0;
        Buflist->datasize = 0;
        bufheadtemp = Buflist;
        Buflist = Buflist->next;
        bufheadtemp->previous = NULL;
        bufheadtemp->next = NULL;
        free(bufheadtemp);
        bufheadtemp = NULL;
    }
    return Buflist;
}
/*获取条目数量*/
uint16_t GetDataItemNum(DataBuf_ListTypedef *Buflist)
{
    uint16_t Num = 0;
    if(Buflist == NULL)
    {
        return Num;
    }
    while(Buflist->previous != NULL)      //回溯到链表头部
    {
        Buflist = Buflist->previous;
    }
    while(Buflist->next != NULL)           //回溯到链表尾巴
    {
        Buflist = Buflist->next;
        Num++;
    }
    return Num;
}