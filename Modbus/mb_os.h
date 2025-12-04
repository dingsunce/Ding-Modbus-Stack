/*!*****************************************************************************
 * file		mb_os.h
 * $Author: sunce.ding
 *******************************************************************************/

#ifndef MB_OS_H
#define MB_OS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "define.h"
#include "mb_core.h"
#include "s_list.h"

  typedef struct MbTxEntry
  {
    LIST_HEADER;
    u8  TxData[MB_TX_BUF_SIZE];
    u16 TxDataNum;
  } MbTxEntry_t;

  void MB_OS_Init(void);

  void MB_OS_Exit(void);

  void MB_OS_RxSignal(MODBUS_CH *pch);

  void MBM_OS_RxWait(MODBUS_CH *pch, u16 *perr);

  void MBM_OS_Tx(MODBUS_CH *pch);

  void MBM_OS_TxFlush(MODBUS_CH *pch, u8 slave_addr);

  MbTxEntry_t *MB_OS_GetTxPacket(MODBUS_CH *pch);

  bool MB_OS_IsInTxRx(void);

  void MB_OS_ResetTxRxFlag(void);

#ifdef __cplusplus
}
#endif

#endif
