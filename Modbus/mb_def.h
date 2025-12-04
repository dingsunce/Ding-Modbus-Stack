/*!*****************************************************************************
 * file		mb_def.h
 * $Author: sunce.ding
 *******************************************************************************/

#ifndef MB_DEF_H
#define MB_DEF_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "define.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MODBUS_VERSION 10

#ifndef MODBUS_CFG_MAX_CH
#define MODBUS_CFG_MAX_CH 1
#endif

/*
 * TCP MODBUS ADU = 253 bytes + MBAP (7 bytes) = 260 bytes
 *
 * RS232 / RS485 ADU = 253 bytes + Server address (1 byte) + CRC (2 bytes) = 256 bytes
 *
 * MBAP = Transaction Identifier(2 Bytes) + Protocol Identifier(2 Bytes) + Length (2 Bytes)
 *        + Server address(1 Bytes)
 */
#ifndef MB_TX_BUF_SIZE
#define MB_TX_BUF_SIZE 260
#endif

#ifndef MB_RX_BUF_SIZE
#define MB_RX_BUF_SIZE 260
#endif

  /*
  *********************************************************************************************************
  *                                         MODBUS RTU CONSTANTS
  *********************************************************************************************************
  */

#define MODBUS_RTU_MIN_MSG_SIZE 4

#define MODBUS_CRC16_POLY 0xA001 /* CRC-16 Generation Polynomial value.     */

/*
*********************************************************************************************************
*                                         MODBUS TCP CONSTANTS
*********************************************************************************************************
*/
/*
 *
 * <------------------------ MODBUS TCP/IP ADU(1) ------------------------->
 *              <----------- MODBUS PDU (1') ---------------->
 *  +-----------+---------------+------------------------------------------+
 *  | TID | PID | Length |
 *  +-----------+---------------+------------------------------------------+
 *  |     |     |        |
 * (2)   (3)   (4)      (5)
 *
 * (2)  ... MB_TCP_TID          = 0 (Transaction Identifier - 2 Byte)
 * (3)  ... MB_TCP_PID          = 2 (Protocol Identifier    - 2 Byte)
 * (4)  ... MB_TCP_LEN          = 4 (Number of bytes        - 2 Byte)

 */
#define MB_TCP_TID 0
#define MB_TCP_PID 2
#define MB_TCP_LEN 4

#define MB_TCP_PROTOCOL_ID 0 /* 0 = Modbus Protocol */
#define MB_TCP_HEADER      6

#define MODBUS_TCP_MIN_MSG_SIZE (MB_TCP_HEADER + 4)

/*
*********************************************************************************************************
*                                              CONSTANTS
*********************************************************************************************************
*/
#define MODBUS_MODE_RTU   0
#define MODBUS_MODE_ASCII 1
#define MODBUS_MODE_TCP   2
#define MODBUS_MODE_NONE  3

#define MODBUS_PARITY_NONE 0
#define MODBUS_PARITY_ODD  1
#define MODBUS_PARITY_EVEN 2

#define MODBUS_COIL_OFF 0
#define MODBUS_COIL_ON  1

#define MODBUS_SLAVE  0
#define MODBUS_MASTER 1

  /*
  *********************************************************************************************************
  *                                              CONSTANTS
  *********************************************************************************************************
  */

#define MODBUS_FC01_RD_COIL          1 /* COIL Status.                            */
#define MODBUS_FC02_RD_DI            2 /* Read Discrete Input                     */
#define MODBUS_FC03_RD_HOLDING_REG   3 /* Holding registers.                      */
#define MODBUS_FC04_RD_INPUT_REG     4 /* Read Only registers.                    */
#define MODBUS_FC05_WR_SINGLE_COIL   5 /* Set a single COIL value.                */
#define MODBUS_FC06_WR_SINGLE_REG    6 /* Holding registers.                      */
#define MODBUS_FC08_LOOPBACK         8
#define MODBUS_FC15_WR_MULTIPLE_COIL 15 /* Set multiple COIL values.               */
#define MODBUS_FC16_WR_MULTIPLE_REG  16 /* Holding registers                       */
#define MODBUS_FC20_RD_FILE          20 /* Read contents of a File/Record          */
#define MODBUS_FC21_WR_FILE          21 /* Write data to a File/Record             */

#define MODBUS_FC08_LOOPBACK_QUERY             0 /* Loopback sub-function codes             */
#define MODBUS_FC08_LOOPBACK_CLR_CTR           10
#define MODBUS_FC08_LOOPBACK_BUS_MSG_CTR       11
#define MODBUS_FC08_LOOPBACK_BUS_CRC_CTR       12
#define MODBUS_FC08_LOOPBACK_BUS_EXCEPT_CTR    13
#define MODBUS_FC08_LOOPBACK_SLAVE_MSG_CTR     14
#define MODBUS_FC08_LOOPBACK_SLAVE_NO_RESP_CTR 15

#define MODBUS_COIL_OFF_CODE 0x0000
#define MODBUS_COIL_ON_CODE  0xFF00
  /*
  *********************************************************************************************************
  *                                              ERROR CODES
  *********************************************************************************************************
  */

#define MODBUS_ERR_NONE 0

#define MODBUS_ERR_ILLEGAL_FC        1
#define MODBUS_ERR_ILLEGAL_DATA_ADDR 2
#define MODBUS_ERR_ILLEGAL_DATA_QTY  3
#define MODBUS_ERR_ILLEGAL_DATA_VAL  4
#define MODBUS_ERR_NOT_PREEMPTIVE_OS 5

#define MODBUS_ERR_TIME_OUT   3000
#define MODBUS_ERR_NOT_MASTER 3001
#define MODBUS_ERR_INVALID    3002
#define MODBUS_ERR_NULLPTR    3003

#define MODBUS_ERR_COIL_ADDR  5000
#define MODBUS_ERR_COIL_WR    5001
#define MODBUS_ERR_SLAVE_ADDR 5002
#define MODBUS_ERR_FC         5003
#define MODBUS_ERR_BYTE_COUNT 5004
#define MODBUS_ERR_COIL_QTY   5005
#define MODBUS_ERR_REG_ADDR   5006
#define MODBUS_ERR_NBR_REG    5007
#define MODBUS_ERR_SUB_FNCT   5008
#define MODBUS_ERR_DIAG       5009
#define MODBUS_ERR_WR         5010

#define MODBUS_ERR_RX 6000

  /*
  *********************************************************************************************************
  *                                        MODBUS ASCII CONSTANTS
  *********************************************************************************************************
  */

#define MODBUS_ASCII_START_FRAME_CHAR ':'  /* Start of frame delimiter                */
#define MODBUS_ASCII_END_FRAME_CHAR1  0x0D /* ASCII character: Carriage return        */
#define MODBUS_ASCII_END_FRAME_CHAR2  0x0A /* ASCII character: Line Feed              */
#define MODBUS_ASCII_MIN_MSG_SIZE     11

#ifdef __cplusplus
}
#endif

#endif
