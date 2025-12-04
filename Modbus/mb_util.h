/*!*****************************************************************************
 * file		mb_util.h
 * $Author: sunce.ding
 *******************************************************************************/

#ifndef MB_UTIL_H
#define MB_UTIL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "mb_core.h"

  u8 *MB_ASCII_BinToHex(u8 value, u8 *pbuf);

  u8 MB_ASCII_HexToBin(u8 *phex);

  u8 MB_ASCII_RxCalcLRC(MODBUS_CH *pch);

  u8 MB_ASCII_TxCalcLRC(MODBUS_CH *pch);

  u16 MB_RTU_TxCalcCRC(MODBUS_CH *pch);

  u16 MB_RTU_RxCalcCRC(MODBUS_CH *pch);

#ifdef __cplusplus
}
#endif

#endif
