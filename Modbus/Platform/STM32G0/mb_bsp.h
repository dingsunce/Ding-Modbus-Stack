/*!*****************************************************************************
 * file		mb_bsp.h
 * $Author: sunce.ding
 *******************************************************************************/

#ifndef MB_BSP_H
#define MB_BSP_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "mb_core.h"

  void MB_BSP_UartRxIntEnable(MODBUS_CH *pch); /* Enable  Rx interrupts */

  void MB_BSP_UartRxIntDisable(MODBUS_CH *pch); /* Disable Rx interrupts */

  void MB_BSP_UartTx(MODBUS_CH *pch, u8 c);

  void MB_BSP_UartTxIntEnable(MODBUS_CH *pch); /* Enable  Tx interrupts */

  void MB_BSP_UartTxIntDisable(MODBUS_CH *pch); /* Disable Tx interrupts */

  void MB_BSP_UartPortCfg(MODBUS_CH *pch, u32 baud, u8 parity);

  void MB_BSP_UartExit(void);

  void MB_BSP_TcpTx(MODBUS_CH *pch);
#ifdef __cplusplus
}
#endif

#endif
