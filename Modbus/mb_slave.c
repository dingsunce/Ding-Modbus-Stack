/*!*****************************************************************************
 * file		mb_slave.c
 * $Author: sunce.ding
 *******************************************************************************/
#include "mb_slave.h"
#include "mb_core.h"
#include "mb_def.h"
#include "mb_os.h"
#include "mb_slave_app.h"
#include "mb_util.h"

#define MBS_RX_DATA_START   (((u16)pch->RxData[2] << 8) + (u16)pch->RxData[3])
#define MBS_RX_DATA_START_H (pch->RxData[2])
#define MBS_RX_DATA_START_L (pch->RxData[3])

#define MBS_RX_DATA_POINTS   (((u16)pch->RxData[4] << 8) + (u16)pch->RxData[5])
#define MBS_RX_DATA_POINTS_H (pch->RxData[4])
#define MBS_RX_DATA_POINTS_L (pch->RxData[5])

#define MBS_RX_DATA_BYTES (pch->RxData[6])

#define MBS_RX_DATA_COIL   (((u16)pch->RxData[4] << 8) + (u16)pch->RxData[5])
#define MBS_RX_DATA_COIL_H (pch->RxData[4])
#define MBS_RX_DATA_COIL_L (pch->RxData[5])

#define MBS_RX_DATA_REG   (((u16)pch->RxData[4] << 8) + (u16)pch->RxData[5])
#define MBS_RX_DATA_REG_H (pch->RxData[4])
#define MBS_RX_DATA_REG_L (pch->RxData[5])

#define MBS_RX_DIAG_CODE   (((u16)pch->RxData[2] << 8) + (u16)pch->RxData[3])
#define MBS_RX_DIAG_CODE_H (pch->RxData[2])
#define MBS_RX_DIAG_CODE_L (pch->RxData[3])
#define MBS_RX_DIAG_DATA   (((u16)pch->RxData[4] << 8) + (u16)pch->RxData[5])
#define MBS_RX_DIAG_DATA_H (pch->RxData[4])
#define MBS_RX_DIAG_DATA_L (pch->RxData[5])

#define MBS_RX_FRAME        (&pch->RxFrame)
#define MBS_RX_FRAME_ADDR   (pch->RxData[0])
#define MBS_RX_FRAME_FC     (pch->RxData[1])
#define MBS_RX_FRAME_DATA   (pch->RxData[2])
#define MBS_RX_FRAME_NBYTES (pch->RxDataNum)

#define MBS_TX_DATA_START_H (pch->TxData[2])
#define MBS_TX_DATA_START_L (pch->TxData[3])

#define MBS_TX_DATA_POINTS_H (pch->TxData[4])
#define MBS_TX_DATA_POINTS_L (pch->TxData[5])

#define MBS_TX_DATA_COIL_H (pch->TxData[4])
#define MBS_TX_DATA_COIL_L (pch->TxData[5])

#define MBS_TX_DATA_REG_H (pch->TxData[4])
#define MBS_TX_DATA_REG_L (pch->TxData[5])

#define MBS_TX_DIAG_CODE_H (pch->TxData[2])
#define MBS_TX_DIAG_CODE_L (pch->TxData[3])
#define MBS_TX_DIAG_DATA_H (pch->TxData[4])
#define MBS_TX_DIAG_DATA_L (pch->TxData[5])

#define MBS_TX_FRAME_ADDR   (pch->TxData[0])
#define MBS_TX_FRAME_FC     (pch->TxData[1])
#define MBS_TX_FRAME_DATA   (pch->TxData[2])
#define MBS_TX_FRAME_NBYTES (pch->TxDataNum)

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static void MBS_ErrRespSet(MODBUS_CH *pch, u8 errcode);

static bool MBS_ReadCoils(MODBUS_CH *pch);

static bool MBS_ReadDiscreteInputs(MODBUS_CH *pch);

static bool MBS_ReadHoldingRegs(MODBUS_CH *pch);

static bool MBS_ReadInputRegs(MODBUS_CH *pch);

static bool MBS_WriteSingleCoil(MODBUS_CH *pch);

static bool MBS_WriteSingleHoldingReg(MODBUS_CH *pch);

static bool MBS_Loopback(MODBUS_CH *pch);

static bool MBS_WriteMultipleCoils(MODBUS_CH *pch);

static bool MBS_WriteMultipleHoldingRegs(MODBUS_CH *pch);

static bool MBS_ReadFileRecord(MODBUS_CH *pch);

static bool MBS_WriteFileRecord(MODBUS_CH *pch);
//-----------------------------------------------------------------------------------------------------------
static void MBS_ErrRespSet(MODBUS_CH *pch, u8 err_code)
{
  pch->StatExceptCtr++;
  MBS_TX_FRAME_ADDR = MBS_RX_FRAME_ADDR;
  MBS_TX_FRAME_FC = MBS_RX_FRAME_FC | 0x80;
  MBS_TX_FRAME_DATA = err_code;
  MBS_TX_FRAME_NBYTES = 3;
}
//-----------------------------------------------------------------------------------------------------------
bool MBS_FCxx_Handler(MODBUS_CH *pch)
{
  bool send_reply;

  memset(pch->TxData, 0, MB_TX_BUF_SIZE);

  send_reply = false;
  if ((MBS_RX_FRAME_ADDR == pch->NodeAddr) || /* Proper node address? (i.e. Is this message for us?)      */
      (MBS_RX_FRAME_ADDR == 0))
  { /* ... or a 'broadcast' address?                            */
    pch->StatSlaveMsgCtr++;
    if (MBS_APP_IsSupportFC(MBS_RX_FRAME_FC) == false)
    {
      MBS_ErrRespSet(pch, MODBUS_ERR_ILLEGAL_FC);
      return true;
    }

    switch (MBS_RX_FRAME_FC)
    {
    case MODBUS_FC01_RD_COIL:
      send_reply = MBS_ReadCoils(pch);
      break;

    case MODBUS_FC02_RD_DI:
      send_reply = MBS_ReadDiscreteInputs(pch);
      break;

    case MODBUS_FC03_RD_HOLDING_REG:
      send_reply = MBS_ReadHoldingRegs(pch);
      break;

    case MODBUS_FC04_RD_INPUT_REG:
      send_reply = MBS_ReadInputRegs(pch);
      break;

    case MODBUS_FC05_WR_SINGLE_COIL:
      send_reply = MBS_WriteSingleCoil(pch);
      break;

    case MODBUS_FC06_WR_SINGLE_REG:
      send_reply = MBS_WriteSingleHoldingReg(pch);
      break;

    case MODBUS_FC08_LOOPBACK:
      send_reply = MBS_Loopback(pch);
      break;

    case MODBUS_FC15_WR_MULTIPLE_COIL:
      send_reply = MBS_WriteMultipleCoils(pch);
      break;

    case MODBUS_FC16_WR_MULTIPLE_REG:
      send_reply = MBS_WriteMultipleHoldingRegs(pch);
      break;

    case MODBUS_FC20_RD_FILE:
      send_reply = MBS_ReadFileRecord(pch);
      break;

    case MODBUS_FC21_WR_FILE:
      send_reply = MBS_WriteFileRecord(pch);
      break;

    default:
      break;
    }
  }

  if (MBS_RX_FRAME_ADDR == 0)
  { /* Was the command received a 'broadcast'?                  */
    return false;
  }
  else
  {
    return (send_reply);
  }
}

/*
*********************************************************************************************************
*                                           MBS_ReadCoils()
* Description : Responds to a request to read the status of any number of coils.
*
* Note(s)     : 1) RX command format:             Example:
*                  <slave address>                0x11
*                  <function code>                0x01
*                  <start address HI>             0x00
*                  <start address LO>             0x13
*                  <# coils HI>                   0x00
*                  <# coils LO>                   0x25
*
*               2) TX reply format:               Example:
*                  <slave address>                0x11
*                  <function code>                0x01
*                  <byte count>                   0x05
*                  <Data Coils>                   0xCD  (Bit set to 1 means ON, Bit cleared means == OFF)
*                  <Data Coils>                   0x6B  (Bit set to 1 means ON, Bit cleared means == OFF)
*                  <Data Coils>                   0xB2  (Bit set to 1 means ON, Bit cleared means == OFF)
*                  <Data Coils>                   0x0E  (Bit set to 1 means ON, Bit cleared means == OFF)
*                  <Data Coils>                   0x1B  (Bit set to 1 means ON, Bit cleared means == OFF)
*********************************************************************************************************
*/
static bool MBS_ReadCoils(MODBUS_CH *pch)
{
  u8  *presp;
  u16  coil;
  bool coil_val;
  u16  nbr_coils;
  u16  nbr_bytes;
  u8   bit_mask;
  u16  ix;

  if (pch->RxDataNum != 6)
    return false;

  coil = MBS_RX_DATA_START;
  nbr_coils = MBS_RX_DATA_POINTS;
  if (nbr_coils == 0 || nbr_coils > 2000)
  {
    MBS_ErrRespSet(pch, MODBUS_ERR_ILLEGAL_DATA_QTY);
    return true;
  }

  if (MBS_APP_ReadCoilVerify(coil, nbr_coils) == false)
  {
    MBS_ErrRespSet(pch, MODBUS_ERR_ILLEGAL_DATA_ADDR);
    return true;
  }

  nbr_bytes = ((nbr_coils - 1) / 8) + 1;

  bit_mask = 0x01;
  ix = 0;
  presp = &pch->TxData[0];
  *presp++ = MBS_RX_FRAME_ADDR;
  *presp++ = MBS_RX_FRAME_FC;
  *presp++ = (u8)nbr_bytes;
  pch->TxDataNum = nbr_bytes + 3;

  while (ix < nbr_coils)
  {
    coil_val = MBS_APP_ReadCoil(coil);
    if (coil_val == MODBUS_COIL_ON)
      *presp |= bit_mask;

    coil++;
    ix++;
    if ((ix % 8) == 0)
    {
      bit_mask = 0x01;
      presp++;
    }
    else
    {
      bit_mask <<= 1;
    }
  }

  return true;
}

/*
*********************************************************************************************************
*                                            MBS_ReadDiscreteInputs()
*
* Description : Responds to a request to read the status of any number of Discrete Inputs.
*
* Note(s)     : 1) RX command format:             Example:
*                  <slave address>                0x11
*                  <function code>                0x02
*                  <start address HI>             0x00
*                  <start address LO>             0xC4
*                  <# input statuses HI>          0x00
*                  <# input statuses LO>          0x16
*
*               2) TX reply format:               Example:
*                  <slave address>                0x11
*                  <function code>                0x02
*                  <byte count>                   0x03
*                  <Data Inputs>                  0xAC  (Bit set to 1 means ON, Bit cleared means == OFF)
*                  <Data Inputs>                  0xDB  (Bit set to 1 means ON, Bit cleared means == OFF)
*                  <Data Inputs>                  0x35  (Bit set to 1 means ON, Bit cleared means == OFF)
*********************************************************************************************************
*/
static bool MBS_ReadDiscreteInputs(MODBUS_CH *pch)
{
  u8  *presp;
  u16  di;
  bool di_val;
  u16  nbr_di;
  u16  nbr_bytes;
  u8   bit_mask;
  u16  ix;

  if (pch->RxDataNum != 6)
    return false;

  di = MBS_RX_DATA_START;      /* Get the starting address of the desired DIs              */
  nbr_di = MBS_RX_DATA_POINTS; /* Find out how many DIs                                    */
  if (nbr_di == 0 || nbr_di > 2000)
  {
    MBS_ErrRespSet(pch, MODBUS_ERR_ILLEGAL_DATA_QTY);
    return true;
  }

  if (MBS_APP_ReadDiscreteInputVerify(di, nbr_di) == false)
  {
    MBS_ErrRespSet(pch, MODBUS_ERR_ILLEGAL_DATA_ADDR);
    return true;
  }

  nbr_bytes = ((nbr_di - 1) / 8) + 1;
  presp = &pch->TxData[0];

  bit_mask = 0x01;
  ix = 0;
  presp = &pch->TxData[0];
  *presp++ = MBS_RX_FRAME_ADDR;
  *presp++ = MBS_RX_FRAME_FC;
  *presp++ = (u8)nbr_bytes;
  pch->TxDataNum = nbr_bytes + 3;

  while (ix < nbr_di)
  {
    di_val = MBS_APP_ReadDiscreteInput(di);
    if (di_val == MODBUS_COIL_ON)
      *presp |= bit_mask;

    di++;
    ix++;
    if ((ix % 8) == 0)
    {
      bit_mask = 0x01;
      presp++;
    }
    else
    {
      bit_mask <<= 1;
    }
  }

  return true;
}

/*
*********************************************************************************************************
*                                        MBS_ReadHoldingRegs()
*
* Description : Obtains the contents of the specified holding registers.
*
* Note(s)     : 1) RX command format:             Example:
*                  <slave address>                0x11
*                  <function code>                0x03
*                  <start address HI>             0x00
*                  <start address LO>             0x6B
*                  <# registers HI>               0x00
*                  <# registers LO>               0x03
*
*               2) TX reply format:               Example:
*                  <slave address>                0x11
*                  <function code>                0x03
*                  <byte count>                   0x06
*                  <Data HI register>             0x02
*                  <Data LO register>             0x2B
*                  <Data HI register>             0x00
*                  <Data LO register>             0x00
*                  <Data HI register>             0x00
*                  <Data LO register>             0x64
*********************************************************************************************************
*/
static bool MBS_ReadHoldingRegs(MODBUS_CH *pch)
{
  u8 *presp;
  u16 reg;
  u16 nbr_regs;
  u16 nbr_bytes;
  u16 reg_val_16;

  if (pch->RxDataNum != 6)
    return false;

  reg = MBS_RX_DATA_START;
  nbr_regs = MBS_RX_DATA_POINTS;

  if (nbr_regs == 0 || nbr_regs > 125)
  {
    MBS_ErrRespSet(pch, MODBUS_ERR_ILLEGAL_DATA_QTY);
    return true;
  }

  if (MBS_APP_ReadHoldingRegVerify(reg, nbr_regs) == false)
  {
    MBS_ErrRespSet(pch, MODBUS_ERR_ILLEGAL_DATA_ADDR);
    return true;
  }

  nbr_bytes = (u8)(nbr_regs * sizeof(u16));
  presp = &pch->TxData[0];
  *presp++ = MBS_RX_FRAME_ADDR;
  *presp++ = MBS_RX_FRAME_FC;
  *presp++ = (u8)nbr_bytes;
  pch->TxDataNum = nbr_bytes + 3;

  while (nbr_regs > 0)
  {
    reg_val_16 = MBS_APP_ReadHoldingReg(reg);
    *presp++ = (u8)((reg_val_16 >> 8) & 0x00FF);
    *presp++ = (u8)(reg_val_16 & 0x00FF);
    reg++;
    nbr_regs--;
  }

  return true;
}

/*
*********************************************************************************************************
*                                          MBS_ReadInputRegs()
*
* Description : Obtains the contents of the specified input registers.
*
*
* Note(s)     : 1) RX command format:             Example:
*                  <slave address>                0x11
*                  <function code>                0x04
*                  <start address HI>             0x00
*                  <start address LO>             0x08
*                  <# registers HI>               0x00
*                  <# registers LO>               0x01
*
*               2) TX reply format:               Example:
*                  <slave address>                0x11
*                  <function code>                0x04
*                  <byte count>                   0x02
*                  <Data HI register value>       0x00
*                  <Data LO register value>       0x0A
*********************************************************************************************************
*/
static bool MBS_ReadInputRegs(MODBUS_CH *pch)
{
  u8 *presp;
  u16 reg;
  u16 nbr_regs;
  u16 nbr_bytes;
  u16 reg_val_16;

  if (pch->RxDataNum != 6)
    return false;

  reg = MBS_RX_DATA_START;
  nbr_regs = MBS_RX_DATA_POINTS;

  if (nbr_regs == 0 || nbr_regs > 125)
  {
    MBS_ErrRespSet(pch, MODBUS_ERR_ILLEGAL_DATA_QTY);
    return true;
  }

  if (MBS_APP_ReadInputRegVerify(reg, nbr_regs) == false)
  {
    MBS_ErrRespSet(pch, MODBUS_ERR_ILLEGAL_DATA_ADDR);
    return true;
  }

  nbr_bytes = (u8)(nbr_regs * sizeof(u16));
  presp = &pch->TxData[0];
  *presp++ = MBS_RX_FRAME_ADDR;
  *presp++ = MBS_RX_FRAME_FC;
  *presp++ = (u8)nbr_bytes;
  pch->TxDataNum = nbr_bytes + 3;

  while (nbr_regs > 0)
  {
    reg_val_16 = MBS_APP_ReadInputReg(reg);
    *presp++ = (u8)((reg_val_16 >> 8) & 0x00FF);
    *presp++ = (u8)(reg_val_16 & 0x00FF);
    reg++;
    nbr_regs--;
  }

  return true;
}

/*
*********************************************************************************************************
*                                           MBS_WriteSingleCoil()
*
* Description : Responds to a request to force a coil to a specified state.

*
* Note(s)     : 1) A value of 0xFF00 forces a coil ON and 0x0000 to OFF
*
*               2) RX command format:             Example:
*                  <slave address>                0x11
*                  <function code>                0x05
*                  <Coil address HI>              0x00
*                  <Coil address LO>              0xAC
*                  <Force coil value HI>          0xFF
*                  <Force coil value LO>          0x00
*
*               3) TX reply format:               Example:
*                  <slave address>                0x11
*                  <function code>                0x05
*                  <Coil address HI>              0x00
*                  <Coil address LO>              0xAC
*                  <Force coil value HI>          0xFF
*                  <Force coil value LO>          0x00
*********************************************************************************************************
*/
static bool MBS_WriteSingleCoil(MODBUS_CH *pch)
{
  u8  *prx_data;
  u8  *ptx_data;
  u8   i;
  u16  coil;
  bool coil_val;
  u16  temp;

  if (pch->RxDataNum != 6)
    return false;

  coil = MBS_RX_DATA_START;
  temp = MBS_RX_DATA_COIL;

  if (temp == MODBUS_COIL_OFF_CODE)
    coil_val = 0;
  else
    coil_val = 1;

  if (MBS_APP_WriteCoilVerify(coil, 1) == false)
  {
    MBS_ErrRespSet(pch, MODBUS_ERR_ILLEGAL_DATA_ADDR);
    return true;
  }

  MBS_APP_WriteCoil(coil, coil_val);

  pch->TxDataNum = 6;
  MBS_TX_FRAME_ADDR = MBS_RX_FRAME_ADDR;
  MBS_TX_FRAME_FC = MBS_RX_FRAME_FC;
  prx_data = &pch->RxData[2]; /* Copy received register address and data to response      */
  ptx_data = &pch->TxData[2];
  for (i = 0; i < 4; i++)
  {
    *ptx_data++ = *prx_data++;
  }

  return true;
}

/*
*********************************************************************************************************
*                                        MBS_WriteSingleHoldingReg()
*
* Description : Change the value of a single register.
*
*
* Note(s)     : 1) RX command format:             Example:
*                  <slave address>                0x11
*                  <function code>                0x06
*                  <start address HI>             0x00
*                  <start address LO>             0x01
*                  <Register value HI>            0x00
*                  <Register value LO>            0x03
*
*               2) TX reply format:               Example:
*                  <slave address>                0x11
*                  <function code>                0x06
*                  <start address HI>             0x00
*                  <start address LO>             0x01
*                  <Register value HI>            0x00
*                  <Register value LO>            0x03
*********************************************************************************************************
*/
static bool MBS_WriteSingleHoldingReg(MODBUS_CH *pch)
{
  u8 *prx_data;
  u8 *ptx_data;
  u8  i;
  u16 reg;
  u16 reg_val_16;

  if (pch->RxDataNum != 6)
    return false;

  reg = MBS_RX_DATA_START;

  reg_val_16 = MBS_RX_DATA_REG;

  if (MBS_APP_WriteHoldingRegVerify(reg, 1) == false)
  {
    MBS_ErrRespSet(pch, MODBUS_ERR_ILLEGAL_DATA_ADDR);
    return true;
  }

  MBS_APP_WriteHoldingReg(reg, reg_val_16);

  pch->TxDataNum = 6;
  MBS_TX_FRAME_ADDR = MBS_RX_FRAME_ADDR;
  MBS_TX_FRAME_FC = MBS_RX_FRAME_FC;
  prx_data = &pch->RxData[2];
  ptx_data = &pch->TxData[2]; /* Copy received register address and data to response      */

  for (i = 0; i < 4; i++)
  {
    *ptx_data++ = *prx_data++;
  }

  return true;
}

/*
*********************************************************************************************************
*                                           MBS_FC08_Loopback()
*
* Description : The LOOPBACK function contains various diagnostic codes that perform specific actions.
*               This function processes individual diagnostic requests and formats the response message
*               frame accordingly.  Unimplemented diagnostic codes will return an Illegal Data Value
*               Exception Response Code (03).
*
*********************************************************************************************************
*/
static bool MBS_Loopback(MODBUS_CH *pch)
{
  u16 diagcode;

  if (pch->RxDataNum != 6)
    return false;

  pch->TxDataNum = 6;

  /* Prepare response packet                                  */
  MBS_TX_FRAME_ADDR = MBS_RX_FRAME_ADDR;
  MBS_TX_FRAME_FC = MBS_RX_FRAME_FC;
  MBS_TX_DIAG_CODE_H = MBS_RX_DIAG_CODE_H;
  MBS_TX_DIAG_CODE_L = MBS_RX_DIAG_CODE_L;

  diagcode = MBS_RX_DIAG_CODE;

  switch (diagcode)
  {
    /* Return Query function code - no need to do anything.     */
  case MODBUS_FC08_LOOPBACK_QUERY:
    MBS_TX_DIAG_DATA_H = MBS_RX_DIAG_DATA_H;
    MBS_TX_DIAG_DATA_L = MBS_RX_DIAG_DATA_L;
    break;

  /* Initialize the system counters, echo response back.      */
  case MODBUS_FC08_LOOPBACK_CLR_CTR:
    MBS_TX_DIAG_DATA_H = MBS_RX_DIAG_DATA_H;
    MBS_TX_DIAG_DATA_L = MBS_RX_DIAG_DATA_L;
    MBS_StatInit(pch);
    break;

    /* Return the message count in diag information field.      */
  case MODBUS_FC08_LOOPBACK_BUS_MSG_CTR:
    MBS_TX_DIAG_DATA_H = (u8)((pch->StatMsgCtr & 0xFF00) >> 8);
    MBS_TX_DIAG_DATA_L = (u8)(pch->StatMsgCtr & 0x00FF);
    break;

    /* Return the CRC error count in diag information field.    */
  case MODBUS_FC08_LOOPBACK_BUS_CRC_CTR:
    MBS_TX_DIAG_DATA_H = (u8)((pch->StatCRCErrCtr & 0xFF00) >> 8);
    MBS_TX_DIAG_DATA_L = (u8)(pch->StatCRCErrCtr & 0x00FF);
    break;

    /* Return exception count in diag information field.        */
  case MODBUS_FC08_LOOPBACK_BUS_EXCEPT_CTR:
    MBS_TX_DIAG_DATA_H = (u8)((pch->StatExceptCtr & 0xFF00) >> 8);
    MBS_TX_DIAG_DATA_L = (u8)(pch->StatExceptCtr & 0x00FF);
    break;

    /* Return slave message count in diag information field.    */
  case MODBUS_FC08_LOOPBACK_SLAVE_MSG_CTR:
    MBS_TX_DIAG_DATA_H = (u8)((pch->StatSlaveMsgCtr & 0xFF00) >> 8);
    MBS_TX_DIAG_DATA_L = (u8)(pch->StatSlaveMsgCtr & 0x00FF);
    break;

  /* Return no response count in diag information field.      */
  case MODBUS_FC08_LOOPBACK_SLAVE_NO_RESP_CTR:
    MBS_TX_DIAG_DATA_H = (u8)((pch->StatNoRespCtr & 0xFF00) >> 8);
    MBS_TX_DIAG_DATA_L = (u8)(pch->StatNoRespCtr & 0x00FF);
    break;

  default:
    MBS_ErrRespSet(pch, MODBUS_ERR_ILLEGAL_DATA_VAL);
    break;
  }

  return true;
}

/*
*********************************************************************************************************
*                                       MBS_WriteMultipleCoils()
*
* Description : Processes the MODBUS "Force Multiple COILS" command and writes the COIL states.

* Note(s)     : 1) RX command format:             Example:
*                  <slave address>                0x11
*                  <function code>                0x0F
*                  <Coil address HI>              0x00
*                  <Coil address LO>              0x13
*                  <coils count HI>               0x00
*                  <coils count LO>               0x0A
*                  <byte count>                   0x02
*                  <Force Data HI>                0xCD
*                  <Force Data LO>                0x01
*
*               2) TX reply format:               Example:
*                  <slave address>                0x11
*                  <function code>                0x0F
*                  <Coil address HI>              0x00
*                  <Coil address LO>              0x13
*                  <coils count HI>               0x00
*                  <coils count LO>               0x0A
*********************************************************************************************************
*/
static bool MBS_WriteMultipleCoils(MODBUS_CH *pch)
{
  u16  coil;
  u16  nbr_coils;
  u16  nbr_bytes;
  u16  ix;
  u8   data_ix;
  bool coil_val;
  u8   temp;

  if (pch->RxDataNum < 8)
    return false;

  coil = MBS_RX_DATA_START;
  nbr_coils = MBS_RX_DATA_POINTS;
  nbr_bytes = MBS_RX_DATA_BYTES;

  if (nbr_coils == 0 || nbr_coils > 2000)
  {
    MBS_ErrRespSet(pch, MODBUS_ERR_ILLEGAL_DATA_QTY);
    return true;
  }

  if (MBS_APP_WriteCoilVerify(coil, nbr_coils) == false)
  {
    MBS_ErrRespSet(pch, MODBUS_ERR_ILLEGAL_DATA_QTY);
    return true;
  }

  if (((((nbr_coils - 1) / 8) + 1) == nbr_bytes) && (pch->RxDataNum == (nbr_bytes + 7)))
  {
    ix = 0;
    data_ix = 7;
    while (ix < nbr_coils)
    {
      if ((ix % 8) == 0)
        temp = pch->RxData[data_ix++];

      if (temp & 0x01)
        coil_val = MODBUS_COIL_ON;
      else
        coil_val = MODBUS_COIL_OFF;

      MBS_APP_WriteCoil(coil + ix, coil_val);

      temp >>= 1;
      ix++;
    }

    pch->TxDataNum = 6;
    MBS_TX_FRAME_ADDR = MBS_RX_FRAME_ADDR;
    MBS_TX_FRAME_FC = MBS_RX_FRAME_FC;
    MBS_TX_DATA_START_H = MBS_RX_DATA_START_H;
    MBS_TX_DATA_START_L = MBS_RX_DATA_START_L;
    MBS_TX_DATA_POINTS_H = MBS_RX_DATA_POINTS_H;
    MBS_TX_DATA_POINTS_L = MBS_RX_DATA_POINTS_L;
  }
  else
  {
    MBS_ErrRespSet(pch, MODBUS_ERR_ILLEGAL_DATA_VAL);
  }

  return true;
}

/*
*********************************************************************************************************
*                                    MBS_FC16_HoldingRegWrMultiple()
*
* Description : This function is called to write to multiple holding registers.  If the address of the
*               rquest exceeds or is equal to MODBUS_CFG_FP_START_IX, then the command would write to
*               multiple 'floating-point' according to the 'Daniels Flow Meter' extensions.  This means
*               that each register requested is considered as a 32-bit IEEE-754 floating-point format.

*
* Note(s)     : 1) RX command format:             Example:
*                  <slave address>                0x11
*                  <function code>                0x10
*                  <start address HI>             0x00
*                  <start address LO>             0x01
*                  <registers count HI>           0x00
*                  <registers count LO>           0x02
*                  <byte count>                   0x04
*                  <Register value HI>            0x00
*                  <Register value LO>            0x0A
*                  <Register value HI>            0x01
*                  <Register value LO>            0x02
*
*               2) TX reply format:               Example:
*                  <slave address>                0x11
*                  <function code>                0x10
*                  <start address HI>             0x00
*                  <start address LO>             0x01
*                  <registers count HI>           0x00
*                  <registers count LO>           0x02
*********************************************************************************************************
*/
static bool MBS_WriteMultipleHoldingRegs(MODBUS_CH *pch)
{
  u8 *prx_data;
  u16 reg;
  u16 nbr_regs;
  u16 nbr_bytes;
  u8  data_size;
  u16 reg_val_16;

  if (pch->RxDataNum < 9)
    return false;

  reg = MBS_RX_DATA_START;
  nbr_regs = MBS_RX_DATA_POINTS;
  nbr_bytes = MBS_RX_DATA_BYTES;

  if (nbr_regs == 0 || nbr_regs > 125)
  {
    MBS_ErrRespSet(pch, MODBUS_ERR_ILLEGAL_DATA_QTY);
    return true;
  }

  data_size = sizeof(u16);

  if (pch->RxDataNum != (nbr_bytes + 7))
  { /* Compare actual number of bytes to what they say.         */
    MBS_ErrRespSet(pch, MODBUS_ERR_ILLEGAL_DATA_QTY);
    return true;
  }

  if ((nbr_bytes / nbr_regs) != (u16)data_size)
  {
    MBS_ErrRespSet(pch, MODBUS_ERR_ILLEGAL_DATA_VAL);
    return true;
  }

  if (MBS_APP_WriteHoldingRegVerify(reg, nbr_regs) == false)
  {
    MBS_ErrRespSet(pch, MODBUS_ERR_ILLEGAL_DATA_QTY);
    return true;
  }

  prx_data = &pch->RxData[7]; /* Point to data in request frame    */
  while (nbr_regs > 0)
  {
    reg_val_16 = ((u16)*prx_data++) << 8;
    reg_val_16 += (u16)*prx_data++;
    MBS_APP_WriteHoldingReg(reg, reg_val_16);
    nbr_regs--;
  }

  pch->TxDataNum = 6;
  MBS_TX_FRAME_ADDR = MBS_RX_FRAME_ADDR;
  MBS_TX_FRAME_FC = MBS_RX_FRAME_FC;
  MBS_TX_DATA_START_H = MBS_RX_DATA_START_H;
  MBS_TX_DATA_START_L = MBS_RX_DATA_START_L;
  MBS_TX_DATA_POINTS_H = MBS_RX_DATA_POINTS_H;
  MBS_TX_DATA_POINTS_L = MBS_RX_DATA_POINTS_L;
  return true;
}

/*
*********************************************************************************************************
*                                          MBS_ReadFileRecord()
*
* Description : Read a record from a file.

*
* Note(s)     : 1) The current version of this software only supports ONE Sub-request at a time.
*
*               2) RX command format:             Example:
*                  <slave address>                0x11
*                  <function code>                0x14
*                  <byte count>                   0x0E
*                  <Reference Type>               0x06
*                  <File number HI>               0x00
*                  <File number LO>               0x04
*                  <Record number HI>             0x00
*                  <Record number LO>             0x01
*                  <Record count HI>              0x00
*                  <Record count LO>              0x02
*
*               3) TX reply format:               Example:
*                  <slave address>                0x11
*                  <function code>                0x14
*                  <byte count>                   0x06
*                  <Sub request byte count>       0x05
*                  <Reference Type>               0x06
*                  <Record data HI>               0x0D
*                  <Record data LO>               0xFE
*                  <Record data HI>               0x00
*                  <Record data LO>               0x20
*********************************************************************************************************
*/
static bool MBS_ReadFileRecord(MODBUS_CH *pch)
{
  u8 *presp;
  u16 file_nbr;
  u16 record_nbr;
  u16 record_count;
  u16 byte_count;
  u8  reference_type;
  u8  err;
  u16 reg_val;
  u16 ix;

  byte_count = pch->RxData[2]; /* Get the number of bytes in the command received          */
  if (byte_count < 7 || byte_count > 245)
  {
    MBS_ErrRespSet(pch, MODBUS_ERR_ILLEGAL_DATA_QTY);
    return true;
  }

  reference_type = pch->RxData[3];

  file_nbr = ((u16)pch->RxData[4] << 8) + (u16)pch->RxData[5];

  record_nbr = ((u16)pch->RxData[6] << 8) + (u16)pch->RxData[7];

  record_count = ((u16)pch->RxData[8] << 8) + (u16)pch->RxData[9];

  presp = &pch->TxData[0];

  /* Determine the total number of data bytes in the Tx frame */
  pch->TxDataNum = record_count * sizeof(u16) + 5;
  *presp++ = MBS_RX_FRAME_ADDR;
  *presp++ = MBS_RX_FRAME_FC;

  // File type should ALWAYS be 6.
  if (reference_type == 6)
  {
    // Total byte count (excluding  addr, FC, total byte count)
    *presp++ = (u8)(pch->TxDataNum - 3);

    // Sub request byte count (excluding addr, FC, total byte count, sub request byte count)
    *presp++ = (u8)(pch->TxDataNum - 4);

    // Reference type is ALWAYS 6.
    *presp++ = 6;

    ix = 0;
    while (record_count > 0)
    {
      reg_val = MBS_APP_ReadFileRecord(file_nbr, record_nbr, ix, &err);

      switch (err)
      {
      case MODBUS_ERR_NONE:
        *presp++ = (u8)(reg_val >> 8);
        *presp++ = (u8)(reg_val & 0x00FF);
        break;

      default:
        MBS_ErrRespSet(pch, err);
        return true;
      }

      ix++;
      record_count--;
    }
  }
  else
  {
    MBS_ErrRespSet(pch, MODBUS_ERR_ILLEGAL_DATA_ADDR);
    return true;
  }

  return true;
}

/*
*********************************************************************************************************
*                                       MBS_WriteFileRecord()
*
* Description : Write a record to a file.
*

* Note(s)     : (1) The current version of this software only supports ONE Sub-request at a time.
*
*               2) RX command format:             Example:
*                  <slave address>                0x11
*                  <function code>                0x15
*                  <byte count>                   0x0D
*                  <Reference Type>               0x06
*                  <File number HI>               0x00
*                  <File number LO>               0x04
*                  <Record number HI>             0x00
*                  <Record number LO>             0x07
*                  <Record count HI>              0x00
*                  <Record count LO>              0x03
*                  <Record data HI>               0x06
*                  <Record data LO>               0xAF
*                  <Record data HI>               0x04
*                  <Record data LO>               0xBE
*                  <Record data HI>               0x10
*                  <Record data LO>               0x0D
*
*               3) TX reply format:               Example:
*                  <slave address>                0x11
*                  <function code>                0x15
*                  <byte count>                   0x0D
*                  <Reference Type>               0x06
*                  <File number HI>               0x00
*                  <File number LO>               0x04
*                  <Record number HI>             0x00
*                  <Record number LO>             0x07
*                  <Record count HI>              0x00
*                  <Record count LO>              0x03
*                  <Record data HI>               0x06
*                  <Record data LO>               0xAF
*                  <Record data HI>               0x04
*                  <Record data LO>               0xBE
*                  <Record data HI>               0x10
*                  <Record data LO>               0x0D
*********************************************************************************************************
*/
static bool MBS_WriteFileRecord(MODBUS_CH *pch)
{
  u8 *prx_data;
  u8 *pcmd;
  u8 *presp;
  u16 byte_count;
  u8  reference_type;
  u16 file_nbr;
  u16 record_nbr;
  u16 record_count;
  u8  err;
  u16 reg_val;
  u16 ix;

  byte_count = pch->RxData[2];
  if (byte_count < 7 || byte_count > 245)
  {
    MBS_ErrRespSet(pch, MODBUS_ERR_ILLEGAL_DATA_QTY);
    return true;
  }

  reference_type = pch->RxData[3];
  file_nbr = ((u16)pch->RxData[4] << 8) + (u16)pch->RxData[5];

  record_nbr = ((u16)pch->RxData[6] << 8) + (u16)pch->RxData[7];

  record_count = ((u16)pch->RxData[8] << 8) + (u16)pch->RxData[9];

  prx_data = &pch->RxData[10];

  /* File type should ALWAYS be 6.                            */
  if (reference_type == 6)
  {
    ix = 0;
    while (record_count > 0)
    {
      reg_val = ((u16)*prx_data++ << 8) & 0xFF00;
      reg_val |= (u16)*prx_data++ & 0x00FF;
      MBS_APP_WriteFileRecord(file_nbr, record_nbr, ix, reg_val, &err);

      switch (err)
      {
      case MODBUS_ERR_NONE:
        break;

      default:
        MBS_ErrRespSet(pch, err);
        return true;
      }

      ix++;
      record_count--;
    }
  }
  else
  {
    MBS_ErrRespSet(pch, MODBUS_ERR_ILLEGAL_DATA_ADDR);
    return true;
  }

  record_count = ((u16)pch->RxData[8] << 8) + (u16)pch->RxData[9];
  pcmd = &pch->RxData[0];
  presp = &pch->TxData[0];
  pch->TxDataNum = (record_count * 2) + 10;

  for (ix = 0; ix < pch->TxDataNum; ix++)
  {
    *presp++ = *pcmd++;
  }

  return true;
}
//-----------------------------------------------------------------------------------------------------------
void MBS_StatInit(MODBUS_CH *pch)
{
  pch->StatMsgCtr = 0;
  pch->StatCRCErrCtr = 0;
  pch->StatExceptCtr = 0;
  pch->StatSlaveMsgCtr = 0;
  pch->StatNoRespCtr = 0;
}
//-----------------------------------------------------------------------------------------------------------
static void MBS_ASCII_Task(MODBUS_CH *pch)
{
  u16  calc_lrc;
  bool send_reply;

  pch->StatMsgCtr++;

  calc_lrc = (u16)MB_ASCII_RxCalcLRC(pch);
  if (calc_lrc != pch->RxCRC)
  {
    pch->StatCRCErrCtr++;
    pch->StatNoRespCtr++;
  }
  else
  {
    send_reply = MBS_FCxx_Handler(pch);
    if (send_reply == true)
      MB_ASCII_Tx(pch);
    else
      pch->StatNoRespCtr++;
  }
}
//-----------------------------------------------------------------------------------------------------------
static void MBS_RTU_Task(MODBUS_CH *pch)
{
  u16  calc_crc;
  bool send_reply;

  pch->StatMsgCtr++;

  calc_crc = (u16)MB_RTU_RxCalcCRC(pch);
  if (calc_crc != pch->RxCRC)
  {
    pch->StatCRCErrCtr++;
    pch->StatNoRespCtr++;
  }
  else
  {
    send_reply = MBS_FCxx_Handler(pch);
    if (send_reply == true)
      MB_RTU_Tx(pch);
    else
      pch->StatNoRespCtr++;
  }
}
//-----------------------------------------------------------------------------------------------------------
static void MBS_TCP_Task(MODBUS_CH *pch)
{
  bool send_reply;

  pch->StatMsgCtr++;

  send_reply = MBS_FCxx_Handler(pch);
  if (send_reply == true)
    MB_TCP_Tx(pch);
  else
    pch->StatNoRespCtr++;
}
//-----------------------------------------------------------------------------------------------------------
void MBS_RxTask(MODBUS_CH *pch)
{
  if (pch != (MODBUS_CH *)0)
  {
    if (pch->Mode == MODBUS_MODE_ASCII)
      MBS_ASCII_Task(pch);

    if (pch->Mode == MODBUS_MODE_RTU)
      MBS_RTU_Task(pch);

    if (pch->Mode == MODBUS_MODE_TCP)
      MBS_TCP_Task(pch);
  }
}
