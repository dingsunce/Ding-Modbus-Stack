/*!*****************************************************************************
 * file		mb_util.c
 * $Author: sunce.ding
 *******************************************************************************/

#include "mb_core.h"
//-----------------------------------------------------------------------------------------------------------
u8 *MB_ASCII_BinToHex(u8 value, u8 *pbuf)
{
  u8 nibble;

  nibble = (value >> 4) & 0x0F; /* Upper Nibble                                       */
  if (nibble <= 9)
    *pbuf++ = (u8)(nibble + '0');
  else
    *pbuf++ = (u8)(nibble - 10 + 'A');

  nibble = value & 0x0F; /* Lower Nibble                                       */
  if (nibble <= 9)
    *pbuf++ = (u8)(nibble + '0');
  else
    *pbuf++ = (u8)(nibble - 10 + 'A');

  return (pbuf);
}
//-----------------------------------------------------------------------------------------------------------
u8 MB_ASCII_HexToBin(u8 *phex)
{
  u8 value;
  u8 high;
  u8 low;

  high = *phex;
  phex++;

  low = *phex;
  /* Upper Nibble                                       */
  if (high <= '9')
    value = (u8)(high - '0');
  else if (high <= 'F')
    value = (u8)(high - 'A' + 10);
  else
    value = (u8)(high - 'a' + 10);

  value <<= 4;

  /* Lower Nibble                                       */
  if (low <= '9')
    value += (u8)(low - '0');
  else if (low <= 'F')
    value += (u8)(low - 'A' + 10);
  else
    value += (u8)(low - 'a' + 10);

  return (value);
}
//-----------------------------------------------------------------------------------------------------------
u8 MB_ASCII_RxCalcLRC(MODBUS_CH *pch)
{
  u8  lrc;
  u16 len;
  u8 *pblock;

  pblock = (u8 *)&pch->RxData[0];
  len = pch->RxDataNum;
  lrc = 0;

  while (len-- > 0)
  {
    lrc += *pblock;
    pblock++;
  }

  lrc = ~lrc + 1; /* Two complement the binary sum                      */
  return (lrc);
}
//-----------------------------------------------------------------------------------------------------------
u8 MB_ASCII_TxCalcLRC(MODBUS_CH *pch)
{
  u8  lrc;
  u16 len;
  u8 *pblock;

  pblock = (u8 *)&pch->TxData[0];
  len = pch->TxDataNum;

  lrc = 0;
  while (len-- > 0)
  {
    lrc += *pblock;
    pblock++;
  }

  lrc = ~lrc + 1; /* Two complement the binary sum                      */
  return (lrc);
}
//-----------------------------------------------------------------------------------------------------------
u16 MB_RTU_RxCalcCRC(MODBUS_CH *pch)
{
  u16  crc;
  u8   shiftctr;
  bool flag;
  u16  len;
  u8  *pblock;

  pblock = (u8 *)&pch->RxData[0];
  len = pch->RxDataNum;
  crc = 0xFFFF;
  while (len > 0)
  {

    len--;
    crc ^= (u16)*pblock++;
    shiftctr = 8;
    do
    {
      flag = (crc & 0x0001) ? true : false;
      crc >>= 1;

      if (flag == true)
      {                           /* If (bit shifted out of rightmost bit was a 1)            */
        crc ^= MODBUS_CRC16_POLY; /* Exclusive OR the CRC with the generating polynomial. */
      }

      shiftctr--;
    } while (shiftctr > 0);
  }

  return (crc);
}
//-----------------------------------------------------------------------------------------------------------
u16 MB_RTU_TxCalcCRC(MODBUS_CH *pch)
{
  u16  crc;
  u8   shiftctr;
  bool flag;
  u16  len;
  u8  *pblock;

  pblock = (u8 *)&pch->TxData[0];
  len = pch->TxDataNum;
  crc = 0xFFFF;
  while (len > 0)
  {

    len--;
    crc ^= (u16)*pblock++;
    shiftctr = 8;

    do
    {
      flag = (crc & 0x0001) ? true : false;
      crc >>= 1;
      if (flag == true)
      {                           /* If (bit shifted out of rightmost bit was a 1)           */
        crc ^= MODBUS_CRC16_POLY; /* Exclusive OR the CRC with the generating polynomial */
      }

      shiftctr--;
    } while (shiftctr > 0);
  }

  return (crc);
}
