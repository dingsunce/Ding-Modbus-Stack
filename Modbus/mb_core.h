/*!*****************************************************************************
 * file		mb_core.h
 * $Author: sunce.ding
 *******************************************************************************/

#ifndef MB_CORE_H
#define MB_CORE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "mb_def.h"

  /*
  *********************************************************************************************************
  *                                               DATA TYPES
  *********************************************************************************************************
  */

  typedef enum
  {
    MB_TX_IDLE,
    MB_TX_WAITING_FOR_RESP,
  } MB_TX_STATE;

  typedef struct modbus_ch
  {
    u8  Ch;       /* Channel number                                                   */
    u8  NodeAddr; /* Modbus node address of the channel                               */
    u16 T_Id;     /* Tcp Transaction Identifier                                       */

    u8  PortNbr;  /* UART port number                                                 */
    u32 BaudRate; /* Baud Rate                                                        */
    u8  Parity;   /* UART's parity settings (MODBUS_PARITY_NONE, _ODD or _EVEN)       */

    u8 Mode; /* Modbus mode: MODBUS_MODE_ASCII MODBUS_MODE_RTU MODBUS_MODE_TCP   */

    u8 MasterSlave; /* Slave when set to MODBUS_SLAVE, Master when set to MODBUS_MASTER */

    u32 RTU_Timeout; /* Timeout when RTU byte received                                   */

    u16 StatMsgCtr; /* Statistics                                                       */
    u16 StatCRCErrCtr;
    u16 StatExceptCtr;
    u16 StatSlaveMsgCtr;
    u16 StatNoRespCtr;

    u16 RxBufNum;              /* Number of bytes received                                         */
    u8 *RxBufPtr;              /* Pointer to current position in buffer                            */
    u8  RxBuf[MB_RX_BUF_SIZE]; /* Storage of received characters or characters received            */

    u8  RxData[MB_RX_BUF_SIZE]; /* Additional data for function requested.                          */
    u16 RxDataNum;              /* Number of bytes in the data field.                               */
    u16 RxCRC;                  /* Error check value (LRC or CRC-16).                               */

    MB_TX_STATE txState;

    u16 TxBufNum;              /* Number of bytes received or to send                              */
    u8 *TxBufPtr;              /* Pointer to current position in buffer                            */
    u8  TxBuf[MB_TX_BUF_SIZE]; /* Storage of received characters or characters to send             */

    u8  TxData[MB_TX_BUF_SIZE]; /* Additional data for function requested.                          */
    u16 TxDataNum;              /* Number of bytes in the data field.                               */

    u8  RespData[MB_RX_BUF_SIZE]; /* data in response.                                                */
    u16 RespDataNum;              /* Number of bytes in response.                                     */
    u16 RespError;                /* response error type                                              */
    u32 RespTimeout;              /* Amount of time Master is willing to wait for response from slave */
  } MODBUS_CH;

  /*
  *********************************************************************************************************
  *                                           GLOBAL VARIABLES
  *********************************************************************************************************
  */

  extern u16 MB_RTU_Freq;   /* Frequency at which RTU timer is running                          */
  extern u32 MB_RTU_TmrCtr; /* Incremented every Modbus RTU timer interrupt                     */

  extern MODBUS_CH MB_ChTbl[MODBUS_CFG_MAX_CH]; /* Modbus channels */

  /*
  *********************************************************************************************************
  *                                  MODBUS INTERFACE FUNCTION PROTOTYPES
  *                                               (MB.C)
  *********************************************************************************************************
  */

  void MB_Init(void);

  void MB_Exit(void);

  MODBUS_CH *MB_UART_Cfg(u8 ch_no, u8 node_addr, u8 master_slave, u8 Mode, u32 resp_Timeout, u32 baud,
                         u8 parity);

  MODBUS_CH *MB_TCP_Cfg(u8 ch_no, u8 node_addr, u8 master_slave, u32 resp_Timeout);
  
  void MB_UART_RxTimeout(MODBUS_CH *pch);

  void MB_UART_RxByte(MODBUS_CH *pch, u8 rx_byte);

  void MB_UART_TxByte(MODBUS_CH *pch);

  void MB_ASCII_Tx(MODBUS_CH *pch);

  void MB_RTU_Tx(MODBUS_CH *pch);

  void MB_TCP_Tx(MODBUS_CH *pch);

  void MB_TCP_Rx(MODBUS_CH *pch);

#ifdef __cplusplus
}
#endif

#endif
