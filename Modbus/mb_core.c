/*!*****************************************************************************
 * file		mb_core.c
 * $Author: sunce.ding
 *******************************************************************************/

#include "mb_core.h"
#include "mb_bsp.h"
#include "mb_def.h"
#include "mb_master.h"
#include "mb_os.h"
#include "mb_slave.h"
#include "mb_util.h"

MODBUS_CH MB_ChTbl[MODBUS_CFG_MAX_CH];
//-----------------------------------------------------------------------------------------------------------
void MB_Init(void)
{
  u8         ch;
  MODBUS_CH *pch;

  pch = &MB_ChTbl[0];
  for (ch = 0; ch < MODBUS_CFG_MAX_CH; ch++)
  {
    pch->Ch = ch;
    pch->NodeAddr = 1;
    pch->T_Id = 0;
    pch->MasterSlave = MODBUS_SLAVE;
    pch->Mode = MODBUS_MODE_NONE;
    pch->TxBufNum = 0;
    pch->TxDataNum = 0;
    pch->txState = MB_TX_IDLE;
    memset(pch->TxBuf, 0, MB_TX_BUF_SIZE);
    memset(pch->TxData, 0, MB_TX_BUF_SIZE);

    pch->TxBufPtr = &pch->TxBuf[0];
    pch->RxBufNum = 0;
    pch->RxDataNum = 0;
    pch->RxBufPtr = &pch->RxBuf[0];
    memset(pch->RxBuf, 0, MB_RX_BUF_SIZE);
    memset(pch->RxData, 0, MB_RX_BUF_SIZE);

    pch->RespDataNum = 0;
    pch->RespError = MODBUS_ERR_NONE;
    pch->RespTimeout = 0;
    pch->RxBufPtr = &pch->RxBuf[0];
    memset(pch->RespData, 0, MB_RX_BUF_SIZE);

    MBS_StatInit(pch);
    pch++;
  }

  MB_OS_Init();
}
//-----------------------------------------------------------------------------------------------------------
void MB_Exit(void)
{
  MB_BSP_UartExit();
  MB_OS_Exit();
}

/*
 * If baud rate is > 19200 then 3p5 should be 1750 us
 * If less or equal to 19200 we calculate the values.
 *
 * Calculate the time for 1 character in us
 * t (us) = bits / (baud / 10 ^ 6) =>
 * t (us) = (bits * 10 ^ 6) / baud
 *
 * For modbus RTU, 1 character is always 11 bits.
 */
static u32 MB_UART_CharTime(u32 baud)
{
  u32 char_time;

  if (baud > 19200)
    char_time = 500;
  else
    char_time = (11 * 1000 * 1000) / baud;

  return char_time;
}
//-----------------------------------------------------------------------------------------------------------
MODBUS_CH *MB_UART_Cfg(u8 ch_no, u8 node_addr, u8 master_slave, u8 Mode, u32 resp_Timeout, u32 baud,
                       u8 parity)
{
  MODBUS_CH *pch;

  if (ch_no < MODBUS_CFG_MAX_CH)
  {
    pch = &MB_ChTbl[ch_no];
    pch->BaudRate = baud;
    pch->Parity = parity;
    pch->MasterSlave = master_slave;
    pch->Mode = Mode;
    pch->NodeAddr = node_addr;

    pch->RTU_Timeout = 35 * MB_UART_CharTime(baud) / 10;
    pch->RespTimeout = resp_Timeout;

    MB_BSP_UartPortCfg(pch, baud, parity);
    return (pch);
  }
  else
  {
    return ((MODBUS_CH *)0);
  }
}
//-----------------------------------------------------------------------------------------------------------
MODBUS_CH *MB_TCP_Cfg(u8 ch_no, u8 node_addr, u8 master_slave, u32 resp_Timeout)
{
  MODBUS_CH *pch;

  if (ch_no < MODBUS_CFG_MAX_CH)
  {
    pch = &MB_ChTbl[ch_no];
    pch->MasterSlave = master_slave;
    pch->Mode = MODBUS_MODE_TCP;
    pch->NodeAddr = node_addr;

    pch->RespTimeout = resp_Timeout;

    return (pch);
  }
  else
  {
    return ((MODBUS_CH *)0);
  }
}
//-----------------------------------------------------------------------------------------------------------
static void MB_RTU_RxByte(MODBUS_CH *pch, u8 rx_byte)
{
  if (pch->RxBufNum < MB_RX_BUF_SIZE)
  {
    *pch->RxBufPtr++ = rx_byte;
    pch->RxBufNum++;
  }
}
//-----------------------------------------------------------------------------------------------------------
static bool MB_RTU_RxVerify(MODBUS_CH *pch)
{
  u8 *prx_data;
  u8 *pmsg;
  u16 rx_size;
  u16 crc;

  pmsg = &pch->RxBuf[0];
  rx_size = pch->RxBufNum;

  if (rx_size >= MODBUS_RTU_MIN_MSG_SIZE)
  {
    if (rx_size <= MB_RX_BUF_SIZE)
    {
      prx_data = &pch->RxData[0];
      // Transfer the node address
      *prx_data++ = *pmsg++;
      rx_size--;

      // Transfer the function code
      *prx_data++ = *pmsg++;
      rx_size--;

      // Transfer the data
      pch->RxDataNum = 2;
      while (rx_size > 2)
      {
        *prx_data++ = *pmsg++;
        pch->RxDataNum++;
        rx_size--;
      }

      // Transfer the CRC over.  It's LSB first, then MSB.
      crc = (u16)*pmsg++;
      crc += (u16)*pmsg << 8;
      pch->RxCRC = crc;
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}
//-----------------------------------------------------------------------------------------------------------
static bool MB_ASCII_RxVerify(MODBUS_CH *pch)
{
  u8 *pmsg;
  u8 *prx_data;
  u16 rx_size;

  pmsg = &pch->RxBuf[0];
  rx_size = pch->RxBufNum;
  prx_data = &pch->RxData[0];

  if ((rx_size & 0x01) && (rx_size > MODBUS_ASCII_MIN_MSG_SIZE) &&
      (pmsg[0] == MODBUS_ASCII_START_FRAME_CHAR) && (pmsg[rx_size - 2] == MODBUS_ASCII_END_FRAME_CHAR1) &&
      (pmsg[rx_size - 1] == MODBUS_ASCII_END_FRAME_CHAR2))
  {
    // Take away for the ':', CR, and LF
    rx_size -= 3;

    // Point past the ':' to the address.
    pmsg++;
    pch->RxDataNum = 0;

    while (rx_size > 2)
    {
      *prx_data++ = MB_ASCII_HexToBin(pmsg);
      pmsg += 2;
      rx_size -= 2;
      pch->RxDataNum++;
    }

    pch->RxCRC = (u16)MB_ASCII_HexToBin(pmsg);
    return true;
  }
  else
  {
    return false;
  }
}
//-----------------------------------------------------------------------------------------------------------
static void MB_ASCII_RxByte(MODBUS_CH *pch, u8 rx_byte)
{
  if (rx_byte == ':')
  {
    pch->RxBufPtr = &pch->RxBuf[0];
    pch->RxBufNum = 0;
  }

  if (pch->RxBufNum < MB_RX_BUF_SIZE)
  {
    *pch->RxBufPtr++ = rx_byte;
    pch->RxBufNum++;
  }

  if (rx_byte == MODBUS_ASCII_END_FRAME_CHAR2)
  {
    if (MB_ASCII_RxVerify(pch))
      MB_OS_RxSignal(pch);

    pch->RxBufPtr = &pch->RxBuf[0];
    pch->RxBufNum = 0;
  }
}
//-----------------------------------------------------------------------------------------------------------
void MB_UART_RxTimeout(MODBUS_CH *pch)
{
  if (pch->Mode == MODBUS_MODE_RTU)
  {
    if (MB_RTU_RxVerify(pch))
      MB_OS_RxSignal(pch);

    pch->RxBufPtr = &pch->RxBuf[0];
    pch->RxBufNum = 0;
  }
}
//-----------------------------------------------------------------------------------------------------------
void MB_UART_RxByte(MODBUS_CH *pch, u8 rx_byte)
{
  switch (pch->Mode)
  {
  case MODBUS_MODE_ASCII:
    MB_ASCII_RxByte(pch, rx_byte & 0x7F);
    break;

  case MODBUS_MODE_RTU:
    MB_RTU_RxByte(pch, rx_byte);
    break;

  default:
    break;
  }
}
//-----------------------------------------------------------------------------------------------------------
static void MB_UART_TxStart(MODBUS_CH *pch)
{
  MB_BSP_UartRxIntDisable(pch);
  MB_BSP_UartTxIntEnable(pch);
  MB_UART_TxByte(pch);
}
//-----------------------------------------------------------------------------------------------------------
void MB_UART_TxByte(MODBUS_CH *pch)
{
  u8 c;

  if (pch->TxBufNum > 0)
  {
    pch->TxBufNum--;
    c = *pch->TxBufPtr++;
    MB_BSP_UartTx(pch, c);
  }
  else
  {
    pch->TxBufPtr = &pch->TxBuf[0];
    MB_BSP_UartTxIntDisable(pch);
    MB_BSP_UartRxIntEnable(pch);
  }
}
//-----------------------------------------------------------------------------------------------------------
void MB_ASCII_Tx(MODBUS_CH *pch)
{
  u8 *ptx_data;
  u8 *pbuf;
  u16 i;
  u16 tx_bytes;
  u8  lrc;

  ptx_data = &pch->TxData[0];
  pbuf = &pch->TxBuf[0];
  *pbuf++ = MODBUS_ASCII_START_FRAME_CHAR;
  tx_bytes = 1;
  i = (u8)pch->TxDataNum;

  while (i > 0)
  {
    pbuf = MB_ASCII_BinToHex(*ptx_data++, pbuf);
    tx_bytes += 2;
    i--;
  }

  lrc = MB_ASCII_TxCalcLRC(pch);
  pbuf = MB_ASCII_BinToHex(lrc, pbuf);
  *pbuf++ = MODBUS_ASCII_END_FRAME_CHAR1;
  *pbuf++ = MODBUS_ASCII_END_FRAME_CHAR2;
  tx_bytes += 4;

  pch->TxBufNum = tx_bytes;
  pch->TxBufPtr = &pch->TxBuf[0];
  MB_UART_TxStart(pch);
}
//-----------------------------------------------------------------------------------------------------------
void MB_RTU_Tx(MODBUS_CH *pch)
{
  u8 *ptx_data;
  u8 *pbuf;
  u8  i;
  u16 tx_bytes;
  u16 crc;

  tx_bytes = 0;
  pbuf = &pch->TxBuf[0];
  ptx_data = &(pch->TxData[0]);
  i = (u8)pch->TxDataNum;

  while (i > 0)
  {
    *pbuf++ = *ptx_data++;
    tx_bytes++;
    i--;
  }

  crc = MB_RTU_TxCalcCRC(pch);
  *pbuf++ = (u8)(crc & 0x00FF);
  *pbuf = (u8)(crc >> 8);
  tx_bytes += 2;

  pch->TxBufNum = tx_bytes;
  pch->TxBufPtr = &pch->TxBuf[0];
  MB_UART_TxStart(pch);
}
//-----------------------------------------------------------------------------------------------------------
void MB_TCP_Tx(MODBUS_CH *pch)
{
  u8 *ptx_data;
  u8 *pbuf;
  u16 i;
  u16 tx_bytes;

  ptx_data = &pch->TxData[0];
  pbuf = &pch->TxBuf[0];

  *pbuf++ = (u8)((pch->T_Id >> 8) & 0x00FF);
  *pbuf++ = (u8)(pch->T_Id & 0x00FF);

  *pbuf++ = (u8)((MB_TCP_PROTOCOL_ID >> 8) & 0x00FF);
  *pbuf++ = (u8)(MB_TCP_PROTOCOL_ID & 0x00FF);

  *pbuf++ = (u8)((pch->TxDataNum >> 8) & 0x00FF);
  *pbuf++ = (u8)(pch->TxDataNum & 0x00FF);

  tx_bytes = 6;

  i = (u8)pch->TxDataNum;

  while (i > 0)
  {
    *pbuf++ = *ptx_data++;
    tx_bytes++;
    i--;
  }

  pch->TxBufNum = tx_bytes;
  pch->TxBufPtr = &pch->TxBuf[0];
  MB_BSP_TcpTx(pch);
}
//-----------------------------------------------------------------------------------------------------------
bool MB_TCP_Rxhandle(MODBUS_CH *pch)
{
  u8 *prx_data;
  u8 *pmsg;
  u16 rx_size;
  u16 tid = 0;
  u16 pid = 0;
  u16 len = 0;

  pmsg = &pch->RxBuf[0];
  rx_size = pch->RxBufNum;

  if (rx_size >= MODBUS_TCP_MIN_MSG_SIZE)
  {
    if (rx_size <= MB_RX_BUF_SIZE)
    {
      tid = pmsg[MB_TCP_TID] << 8;
      tid |= pmsg[MB_TCP_TID + 1];

      pid = pmsg[MB_TCP_PID] << 8;
      pid |= pmsg[MB_TCP_PID + 1];

      len = pmsg[MB_TCP_LEN] << 8;
      len |= pmsg[MB_TCP_LEN + 1];
      
      pmsg = pmsg + 6;
      rx_size = rx_size - 6;

      if (pid != MB_TCP_PROTOCOL_ID)
        return false;

      if (len != rx_size)
        return false;

      if(pch->MasterSlave == MODBUS_MASTER)
      {
        if(tid != pch->T_Id)
          return false;
      }
      
      pch->T_Id = tid;
      prx_data = &pch->RxData[0];
      pch->RxDataNum = 0;
      while (rx_size > 0)
      {
        *prx_data++ = *pmsg++;
        pch->RxDataNum++;
        rx_size--;
      }

      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}
//-----------------------------------------------------------------------------------------------------------
void MB_TCP_Rx(MODBUS_CH *pch)
{
  if (pch->Mode == MODBUS_MODE_TCP)
  {
    if (MB_TCP_Rxhandle(pch))
      MB_OS_RxSignal(pch);

    pch->RxBufPtr = &pch->RxBuf[0];
    pch->RxBufNum = 0;
  }
}
