/*!*****************************************************************************
 * file		mb_os.c
 * $Author: sunce.ding
 *******************************************************************************/

#include "mb_os.h"
#include "define.h"
#include "mb_core.h"
#include "mb_master.h"
#include "mb_master_app.h"
#include "mb_slave.h"
#include "mb_slave_app.h"
#include "memb.h"
#include "memory.h"
#include "osal.h"
#include "s_list.h"

#include "message.h"
#include "process.h"

#define OS_NON_PREEPTIVE 0
#define OS_PREEPTIVE     1

#if OS_TYPE == OS_NON_PREEPTIVE

#ifndef MB_TX_MEM_NUM
#define MB_TX_MEM_NUM 100
#endif

typedef struct MbProc
{
  MODBUS_CH *Pch;
  Process_t  TxProc;
  Process_t  RxProc;

  LIST_STRUCT(Tx_List);
  MEMB_STRUCT(Tx_Mem, MbTxEntry_t, MB_TX_MEM_NUM);

} MbProc_t;

static MbProc_t Proc[MODBUS_CFG_MAX_CH];

static bool IsMbInTxRx = false;

#define TX_CHECK        0
#define TX_RESP_TIMEOUT 1
//-----------------------------------------------------------------------------------------------------------
static u8 TxProcHandler(Process_t *process, MsgId_t msgId, MsgArg_t arg)
{
  MbProc_t    *proc = ContainerOf(process, MbProc_t, TxProc);
  MODBUS_CH   *pch = proc->Pch;
  MbTxEntry_t *entry;

  PROCESS_SCHEDULE_BEGIN()

  while (1)
  {

  TxIdle:
    PROCESS_WAIT_FOR_MSG(msgId == TX_CHECK);
    if (pch->txState == MB_TX_IDLE)
    {
      entry = (MbTxEntry_t *)List_Pop(proc->Tx_List);
      if (entry != NULL)
      {
        pch->TxDataNum = entry->TxDataNum;
        memcpy(pch->TxData, entry->TxData, MB_TX_BUF_SIZE);
        MBM_Tx(pch);
        Memb_Free(&proc->Tx_Mem, entry);

        Msg_SendLater(&proc->TxProc, TX_RESP_TIMEOUT, NULL, pch->RespTimeout);
        goto TxWaitingForResp;
      }
      else
      {
        goto TxIdle;
      }
    }
    else
    {
      Msg_ReSendLater(&proc->TxProc, TX_CHECK, NULL, 10);
      goto TxIdle;
    }

  TxWaitingForResp:
    pch->txState = MB_TX_WAITING_FOR_RESP;
    PROCESS_WAIT_FOR_MSG(msgId == TX_RESP_TIMEOUT || msgId == SYS_MSG_POLL_PROCESS);
    if (msgId == TX_RESP_TIMEOUT)
    {
      pch->RespError = MODBUS_ERR_TIME_OUT;
    }
    else if (msgId == SYS_MSG_POLL_PROCESS)
    {
      Msg_Cancel(&proc->TxProc, TX_RESP_TIMEOUT);
      pch->RespError = MODBUS_ERR_NONE;
      MBM_ParseResp(pch, &pch->RespError);
    }

    IsMbInTxRx = true;
    MBM_APP_RespCallback(pch);
    memset(pch->RespData, 0, MB_RX_BUF_SIZE);

    // still has packet for sending in Tx_List?
    if (!List_IsEmpty(proc->Tx_List))
      Msg_ReSendLater(&proc->TxProc, TX_CHECK, NULL, 10);

    pch->txState = MB_TX_IDLE;
    goto TxIdle;
  }

  PROCESS_SCHEDULE_END()
}
//-----------------------------------------------------------------------------------------------------------
static u8 RxProcHandler(Process_t *process, MsgId_t msgId, MsgArg_t arg)
{
  MbProc_t  *proc = ContainerOf(process, MbProc_t, RxProc);
  MODBUS_CH *pch = proc->Pch;

  PROCESS_SCHEDULE_BEGIN()

  while (1)
  {
    PROCESS_WAIT_FOR_MSG(msgId == SYS_MSG_POLL_PROCESS);
    MBS_RxTask(pch);
  }

  PROCESS_SCHEDULE_END()
}
//-----------------------------------------------------------------------------------------------------------
void MB_OS_Init(void)
{
  for (u8 i = 0; i < MODBUS_CFG_MAX_CH; i++)
  {
    MbProc_t *proc = Proc + i;
    proc->Pch = MB_ChTbl + i;
    LIST_STRUCT_INIT(proc, Tx_List);
    MEMB_STRUCT_INIT(proc, Tx_Mem, MbTxEntry_t, MB_TX_MEM_NUM);

    Process_InitStruct(&proc->TxProc, TxProcHandler, "Modbus Tx Proc");
    Process_Start(&proc->TxProc);

    Process_InitStruct(&proc->RxProc, RxProcHandler, "Modbus Rx Proc");
    Process_Start(&proc->RxProc);
  }
}
//-----------------------------------------------------------------------------------------------------------
void MB_OS_Exit(void)
{
  for (u8 i = 0; i < MODBUS_CFG_MAX_CH; i++)
  {
    MbProc_t *proc = Proc + i;

    LIST_STRUCT_INIT(proc, Tx_List);
    MEMB_STRUCT_INIT(proc, Tx_Mem, MbTxEntry_t, MB_TX_MEM_NUM);

    Process_Exit(&proc->TxProc);
    Process_Exit(&proc->RxProc);
  }
}
//-----------------------------------------------------------------------------------------------------------
void MB_OS_RxSignal(MODBUS_CH *pch)
{
  // send SYS_MSG_POLL_PROCESS to RxProc
  if (pch->MasterSlave == MODBUS_SLAVE)
    Process_Poll(&Proc[pch->Ch].RxProc);

  // send SYS_MSG_POLL_PROCESS to TxProc
  if (pch->MasterSlave == MODBUS_MASTER)
    Process_Poll(&Proc[pch->Ch].TxProc);

  IsMbInTxRx = true;
}
//-----------------------------------------------------------------------------------------------------------
void MBM_OS_RxWait(MODBUS_CH *pch, u16 *perr)
{
  *perr = MODBUS_ERR_NOT_PREEMPTIVE_OS;
}
//-----------------------------------------------------------------------------------------------------------
void MBM_OS_Tx(MODBUS_CH *pch)
{
  MbProc_t    *proc = Proc + pch->Ch;
  MbTxEntry_t *entry = Memb_Alloc(&proc->Tx_Mem);
  if (entry != NULL)
  {
    entry->TxDataNum = pch->TxDataNum;
    memcpy(entry->TxData, pch->TxData, MB_TX_BUF_SIZE);
    List_Add(proc->Tx_List, entry);
    Msg_ReSendLater(&proc->TxProc, TX_CHECK, NULL, 10);
  }
}
//-----------------------------------------------------------------------------------------------------------
MbTxEntry_t *MB_OS_GetTxPacket(MODBUS_CH *pch)
{
  MbProc_t *proc = Proc + pch->Ch;
  return (MbTxEntry_t *)List_Pop(proc->Tx_List);
}
//-----------------------------------------------------------------------------------------------------------
void MBM_OS_TxFlush(MODBUS_CH *pch, u8 slave_addr)
{
  MbProc_t    *proc = Proc + pch->Ch;
  MbTxEntry_t *entry = List_Head(proc->Tx_List);
  MbTxEntry_t *entryBackup = entry;

  while (entryBackup != NULL)
  {
    entryBackup = entry->next;
    if (entry->TxData[0] == slave_addr)
    {
      List_Remove(proc->Tx_List, entry);
      Memb_Free(&proc->Tx_Mem, entry);
    }
  }
}
//-----------------------------------------------------------------------------------------------------------
bool MB_OS_IsInTxRx(void)
{
  return IsMbInTxRx;
}
//-----------------------------------------------------------------------------------------------------------
void MB_OS_ResetTxRxFlag(void)
{
  IsMbInTxRx = false;
}

#else

static os_sem_t    *Master_Sem[MODBUS_CFG_MAX_CH];
static os_thread_t *Slave_Thread;
static os_mbox_t   *Slave_Mbox;
static void         Slave_Task(void *p_arg);
//-----------------------------------------------------------------------------------------------------------
static void MB_OS_InitMaster(void)
{
  for (u8 i = 0; i < MODBUS_CFG_MAX_CH; i++)
  {
    /* Create a semaphore for each channel   */
    Master_Sem[i] = os_sem_create(0);
  }
}
//-----------------------------------------------------------------------------------------------------------
static void MB_OS_InitSlave(void)
{
  Slave_Thread = os_thread_create("Modbus Rx Task", 20, 256, Slave_Task, NULL);

  Slave_Mbox = os_mbox_create(10);
}
//-----------------------------------------------------------------------------------------------------------
static void MB_OS_ExitMaster(void)
{
  for (u8 i = 0; i < MODBUS_CFG_MAX_CH; i++)
  {
    os_sem_destroy(Master_Sem[i]);
  }
}
//-----------------------------------------------------------------------------------------------------------
void MB_OS_ExitSlave(void)
{
  os_thread_destroy(Slave_Thread);

  os_mbox_destroy(Slave_Mbox);
}
//-----------------------------------------------------------------------------------------------------------
void MB_OS_Init(void)
{
  MB_OS_InitMaster();

  MB_OS_InitSlave();
}
//-----------------------------------------------------------------------------------------------------------
void MB_OS_Exit(void)
{
  MB_OS_ExitMaster();

  MB_OS_ExitSlave();
}
//-----------------------------------------------------------------------------------------------------------
static void Slave_Task(void *p_arg)
{
  MODBUS_CH *pch;

  while (true)
  {
    if (os_mbox_fetch(Slave_Mbox, &pch, OS_WAIT_FOREVER))
    {
      MBS_RxTask(pch);
    }
  }
}
//-----------------------------------------------------------------------------------------------------------
void MB_OS_RxSignal(MODBUS_CH *pch)
{
  switch (pch->MasterSlave)
  {
  case MODBUS_MASTER:
    os_sem_signal(Master_Sem[pch->Ch]);
    break;

  case MODBUS_SLAVE:
    os_mbox_post(Slave_Mbox, pch, OS_WAIT_FOREVER);
    break;

  default:
    break;
  }
}
//-----------------------------------------------------------------------------------------------------------
void MBM_OS_RxWait(MODBUS_CH *pch, u16 *perr)
{
  if (os_sem_wait(Master_Sem[pch->Ch], pch->RespTimeout))
    *perr = MODBUS_ERR_NONE;
  else
    *perr = MODBUS_ERR_TIME_OUT;
}
//-----------------------------------------------------------------------------------------------------------
void MBM_OS_Tx(MODBUS_CH *pch)
{
  MBM_Tx(pch);
}
//-----------------------------------------------------------------------------------------------------------
void MBM_OS_TxFlush(MODBUS_CH *pch, u8 slave_addr)
{
  // TODO
}
#endif