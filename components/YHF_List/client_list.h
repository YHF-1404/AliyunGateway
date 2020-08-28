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
#ifndef __CLIENT_LIST_H
#define __CLIENT_LIST_H
#ifdef __cplusplus
 extern "C" {
#endif

// #include "nrf_comm_use.h"
#include "nrf24l01.h"

//ClientList_Typedef ClientList;
//双链表的创建
typedef struct DataBuf_ListTypedef
{
    struct DataBuf_ListTypedef *previous;
    struct DataBuf_ListTypedef *next;
    uint8_t error;
    uint32_t datasize;
    uint8_t *databuf;
}DataBuf_ListTypedef;




typedef struct ClientList_Typedef
{
    struct ClientList_Typedef *previous;
    struct ClientList_Typedef *next;
    DataBuf_ListTypedef *Buflist;
    uint8_t error;
    ClientServerTypedef clientserver;
}ClientList_Typedef;

extern ClientList_Typedef *head;
void (*cleanTopo_cb)(ClientList_Typedef *head);

ClientList_Typedef *InitList(ClientList_Typedef *head);

/*双向循环列表根据client_id输出该条目所有数据*/
ClientList_Typedef *CutOneListItemData(ClientList_Typedef *head,char *str_p);
/*双向循环列表输出所有条目的client_id*/
ClientList_Typedef *CutAllListItemData(ClientList_Typedef *head);
/*双向循环列表根据client_id查找并删除条目*/
ClientList_Typedef *Found_DeleteListItemData(ClientList_Typedef *head,char *str_p);
/*双向循环列表根据client_id查找并修改条目*/
ClientList_Typedef *Found_ModifyListItemData(ClientList_Typedef *head,ClientServerTypedef *ClientList_p);
/*双向循环列表添加条目*/
ClientList_Typedef *InsertListItem(ClientList_Typedef *head,ClientServerTypedef *ClientList_p);
ClientList_Typedef *DeleteAllListItem(ClientList_Typedef *head);
/*双向循环列表输出条目数量*/
uint16_t CutListItemDataNum(ClientList_Typedef *head);
/*双向循环列表清除所有条目的nrf24l01_RW_cmd*/
ClientList_Typedef *CleanAllRW_cmd(ClientList_Typedef *head);
/*双向循环列表根据nrf24l01_RW_cmd清除条目*/
ClientList_Typedef *RW_cmdDeleItem(ClientList_Typedef *head);


/*双向循环列表根据client_id查找条目*/
ClientList_Typedef *FoundListItemData(ClientList_Typedef *head,char *str_p);
/*双向循环列表删除条目
删除之后head指针指向前一个条目
*/
ClientList_Typedef *DeleteListItem(ClientList_Typedef *head);
/*双向循环列表修改条目*/
ClientList_Typedef *ModifyListItemData(ClientList_Typedef *head,ClientServerTypedef *ClientList_p);


/****************************************************************************************************************************/

/*列表初始化*/
DataBuf_ListTypedef *InitDataList(DataBuf_ListTypedef *Buflist); 
/*列表去初始化*/
DataBuf_ListTypedef *DelInitDataList(DataBuf_ListTypedef *Buflist); 
/*数据入链表*/
DataBuf_ListTypedef *DataIntoList(DataBuf_ListTypedef *Buflist,uint8_t *data,uint32_t datasize);
/*数据出链表进入流缓冲器*/
DataBuf_ListTypedef *DataOutList(DataBuf_ListTypedef *Buflist,StreamBufferHandle_t *streambuf);
/*获取条目数量*/
uint16_t GetDataItemNum(DataBuf_ListTypedef *Buflist);

#ifdef __cplusplus
}
#endif

#endif /*__CLIENT_LIST_H*/