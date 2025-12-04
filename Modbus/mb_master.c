/*!*****************************************************************************
 * file		mb_master.c
 * $Author: sunce.ding
 *******************************************************************************/
#include "mb_master.h"
#include "mb_core.h"
#include "mb_os.h"

static u16 MBM_ReadCoil_Resp(MODBUS_CH *pch);

static u16 MBM_ReadReg_Resp(MODBUS_CH *pch);

static u16 MBM_WriteSingleCoil_Resp(MODBUS_CH *pch);

static u16 MBM_WriteMultipleCoils_Resp(MODBUS_CH *pch);

static u16 MBM_WriteSingleReg_Resp(MODBUS_CH *pch);

static u16 MBM_WriteMultipleReg_Resp(MODBUS_CH *pch);

static u16 MBM_Diag_Resp(MODBUS_CH *pch);
//-----------------------------------------------------------------------------------------------------------
void MBM_ParseResp(MODBUS_CH *pch, u16 *perr)
{
  if (*perr != MODBUS_ERR_NONE)
  {
    pch->RespError = *perr;
    return;
  }

  switch (MBM_TX_FRAME_FC)
  {
  case MODBUS_FC01_RD_COIL:
  case MODBUS_FC02_RD_DI:
    *perr = MBM_ReadCoil_Resp(pch);
    break;

  case MODBUS_FC03_RD_HOLDING_REG:
  case MODBUS_FC04_RD_INPUT_REG:
    *perr = MBM_ReadReg_Resp(pch);
    break;

  case MODBUS_FC05_WR_SINGLE_COIL:
    *perr = MBM_WriteSingleCoil_Resp(pch);
    break;

  case MODBUS_FC06_WR_SINGLE_REG:
    *perr = MBM_WriteSingleReg_Resp(pch);
    break;

  case MODBUS_FC08_LOOPBACK:
    *perr = MBM_Diag_Resp(pch);
    break;

  case MODBUS_FC15_WR_MULTIPLE_COIL:
    *perr = MBM_WriteMultipleCoils_Resp(pch);
    break;

  case MODBUS_FC16_WR_MULTIPLE_REG:
    *perr = MBM_WriteMultipleReg_Resp(pch);
    break;

  default:
    *perr = MODBUS_ERR_RX;
    break;
  }

  pch->RespError = *perr;
}
/*
*********************************************************************************************************
*                                           MBM_ReadCoils()
*
* Note(s)     : 1) TX command format:             Example:
*                  <slave address>                0x11
*                  <function code>                0x01
*                  <start address HI>             0x00
*                  <start address LO>             0x13
*                  <# coils HI>                   0x00
*                  <# coils LO>                   0x25
*
*               2) RX reply format:               Example:
*                  <slave address>                0x11
*                  <function code>                0x01
*                  <byte count>                   0x05
*                  <Data Coils>                   0xCD  (Bit set to 1 means ON, Bit cleared means ==
* OFF) <Data Coils>                   0x6B  (Bit set to 1 means ON, Bit cleared means == OFF) <Data
* Coils>                   0xB2  (Bit set to 1 means ON, Bit cleared means == OFF) <Data Coils>
*              0x0E  (Bit set to 1 means ON, Bit cleared means == OFF) <Data Coils>   0x1B  (Bit set
* to 1 means ON, Bit cleared means == OFF)
*********************************************************************************************************
*/
u16 MBM_ReadCoils(MODBUS_CH *pch, u8 slave_addr, u16 start_addr, u16 nbr_coils)
{
  u16 err;

  MBM_TX_FRAME_NBYTES = 6;
  MBM_TX_FRAME_SLAVE_ADDR = slave_addr;
  MBM_TX_FRAME_FC = MODBUS_FC01_RD_COIL;
  MBM_TX_FRAME_FC01_ADDR_HI = (u8)((start_addr >> 8) & 0x00FF);
  MBM_TX_FRAME_FC01_ADDR_LO = (u8)(start_addr & 0x00FF);
  MBM_TX_FRAME_FC01_NBR_POINTS_HI = (u8)((nbr_coils >> 8) & 0x00FF);
  MBM_TX_FRAME_FC01_NBR_POINTS_LO = (u8)(nbr_coils & 0x00FF);

  MBM_OS_Tx(pch);

  MBM_OS_RxWait(pch, &err);

  MBM_ParseResp(pch, &err);

  return (err);
}
/*
*********************************************************************************************************
*                                            MBM_ReadDiscreteInputs()
*
*
* Note(s)     : 1) TX command format:             Example:
*                  <slave address>                0x11
*                  <function code>                0x02
*                  <start address HI>             0x00
*                  <start address LO>             0xC4
*                  <# input statuses HI>          0x00
*                  <# input statuses LO>          0x16
*
*               2) RX reply format:               Example:
*                  <slave address>                0x11
*                  <function code>                0x02
*                  <byte count>                   0x03
*                  <Data Inputs>                  0xAC  (Bit set to 1 means ON, Bit cleared means ==
* OFF) <Data Inputs>                  0xDB  (Bit set to 1 means ON, Bit cleared means == OFF) <Data
* Inputs>                  0x35  (Bit set to 1 means ON, Bit cleared means == OFF)
*********************************************************************************************************
*/
u16 MBM_ReadDiscreteInputs(MODBUS_CH *pch, u8 slave_addr, u16 start_addr, u16 nbr_di)
{
  u16 err;

  MBM_TX_FRAME_NBYTES = 6;
  MBM_TX_FRAME_SLAVE_ADDR = slave_addr;
  MBM_TX_FRAME_FC = MODBUS_FC02_RD_DI;
  MBM_TX_FRAME_FC02_ADDR_HI = (u8)((start_addr >> 8) & 0x00FF);
  MBM_TX_FRAME_FC02_ADDR_LO = (u8)(start_addr & 0x00FF);
  MBM_TX_FRAME_FC02_NBR_POINTS_HI = (u8)((nbr_di >> 8) & 0x00FF);
  MBM_TX_FRAME_FC02_NBR_POINTS_LO = (u8)(nbr_di & 0x00FF);

  MBM_OS_Tx(pch);

  MBM_OS_RxWait(pch, &err);

  MBM_ParseResp(pch, &err);

  return (err);
}
/*
*********************************************************************************************************
*                                        MBM_ReadHoldingRegs()
*
*
* Note(s)     : 1) TX command format:             Example:
*                  <slave address>                0x11
*                  <function code>                0x03
*                  <start address HI>             0x00
*                  <start address LO>             0x6B
*                  <# registers HI>               0x00
*                  <# registers LO>               0x03
*
*               2) RX reply format:               Example:
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
u16 MBM_ReadHoldingRegs(MODBUS_CH *pch, u8 slave_addr, u16 start_addr, u16 nbr_regs)
{
  u16 err;

  MBM_TX_FRAME_NBYTES = 6;
  MBM_TX_FRAME_SLAVE_ADDR = slave_addr;
  MBM_TX_FRAME_FC = MODBUS_FC03_RD_HOLDING_REG;
  MBM_TX_FRAME_FC03_ADDR_HI = (u8)((start_addr >> 8) & 0x00FF);
  MBM_TX_FRAME_FC03_ADDR_LO = (u8)(start_addr & 0x00FF);
  MBM_TX_FRAME_FC03_NBR_POINTS_HI = (u8)((nbr_regs >> 8) & 0x00FF);
  MBM_TX_FRAME_FC03_NBR_POINTS_LO = (u8)(nbr_regs & 0x00FF);

  MBM_OS_Tx(pch);

  MBM_OS_RxWait(pch, &err);

  MBM_ParseResp(pch, &err);

  return (err);
}
/*
*********************************************************************************************************
*                                          MBM_ReadInputRegs()
*
*
*
* Note(s)     : 1) TX command format:             Example:
*                  <slave address>                0x11
*                  <function code>                0x04
*                  <start address HI>             0x00
*                  <start address LO>             0x08
*                  <# registers HI>               0x00
*                  <# registers LO>               0x01
*
*               2) RX reply format:               Example:
*                  <slave address>                0x11
*                  <function code>                0x04
*                  <byte count>                   0x02
*                  <Data HI register value>       0x00
*                  <Data LO register value>       0x0A
*********************************************************************************************************
*/
u16 MBM_ReadInputRegs(MODBUS_CH *pch, u8 slave_addr, u16 start_addr, u16 nbr_regs)
{
  u16 err;

  MBM_TX_FRAME_NBYTES = 6;
  MBM_TX_FRAME_SLAVE_ADDR = slave_addr;
  MBM_TX_FRAME_FC = MODBUS_FC04_RD_INPUT_REG;
  MBM_TX_FRAME_FC04_ADDR_HI = (u8)((start_addr >> 8) & 0x00FF);
  MBM_TX_FRAME_FC04_ADDR_LO = (u8)(start_addr & 0x00FF);
  MBM_TX_FRAME_FC04_NBR_POINTS_HI = (u8)((nbr_regs >> 8) & 0x00FF);
  MBM_TX_FRAME_FC04_NBR_POINTS_LO = (u8)(nbr_regs & 0x00FF);

  MBM_OS_Tx(pch);

  MBM_OS_RxWait(pch, &err);

  MBM_ParseResp(pch, &err);

  return (err);
}
/*
*********************************************************************************************************
*                                           MBM_WriteSingleCoil()
*
*
* Note(s)     : 1) A value of 0xFF00 forces a coil ON and 0x0000 to OFF
*
*               2) TX command format:             Example:
*                  <slave address>                0x11
*                  <function code>                0x05
*                  <Coil address HI>              0x00
*                  <Coil address LO>              0xAC
*                  <Force coil value HI>          0xFF
*                  <Force coil value LO>          0x00
*
*               3) RX reply format:               Example:
*                  <slave address>                0x11
*                  <function code>                0x05
*                  <Coil address HI>              0x00
*                  <Coil address LO>              0xAC
*                  <Force coil value HI>          0xFF
*                  <Force coil value LO>          0x00
*********************************************************************************************************
*/
u16 MBM_WriteSingleCoil(MODBUS_CH *pch, u8 slave_addr, u16 addr, bool coil_val)
{
  u16 err;

  MBM_TX_FRAME_NBYTES = 6;
  MBM_TX_FRAME_SLAVE_ADDR = slave_addr;
  MBM_TX_FRAME_FC = MODBUS_FC05_WR_SINGLE_COIL;
  MBM_TX_FRAME_FC05_ADDR_HI = (u8)((addr >> 8) & 0x00FF);
  MBM_TX_FRAME_FC05_ADDR_LO = (u8)(addr & 0x00FF);

  if (coil_val == MODBUS_COIL_OFF)
  {
    MBM_TX_FRAME_FC05_FORCE_DATA_HI = (u8)0x00;
    MBM_TX_FRAME_FC05_FORCE_DATA_LO = (u8)0x00;
  }
  else
  {
    MBM_TX_FRAME_FC05_FORCE_DATA_HI = (u8)0xFF;
    MBM_TX_FRAME_FC05_FORCE_DATA_LO = (u8)0x00;
  }

  MBM_OS_Tx(pch);

  MBM_OS_RxWait(pch, &err);

  MBM_ParseResp(pch, &err);

  return (err);
}
/*
*********************************************************************************************************
*                                        MBM_WriteSingleHoldingReg()
*
*
* Note(s)     : 1) TX command format:             Example:
*                  <slave address>                0x11
*                  <function code>                0x06
*                  <start address HI>             0x00
*                  <start address LO>             0x01
*                  <Register value HI>            0x00
*                  <Register value LO>            0x03
*
*               2) RX reply format:               Example:
*                  <slave address>                0x11
*                  <function code>                0x06
*                  <start address HI>             0x00
*                  <start address LO>             0x01
*                  <Register value HI>            0x00
*                  <Register value LO>            0x03
*********************************************************************************************************
*/
u16 MBM_WriteSingleHoldingReg(MODBUS_CH *pch, u8 slave_addr, u16 addr, u16 reg_val)
{
  u16 err;

  MBM_TX_FRAME_NBYTES = 6;
  MBM_TX_FRAME_SLAVE_ADDR = slave_addr;
  MBM_TX_FRAME_FC = MODBUS_FC06_WR_SINGLE_REG;
  MBM_TX_FRAME_FC06_ADDR_HI = (u8)((addr >> 8) & 0x00FF);
  MBM_TX_FRAME_FC06_ADDR_LO = (u8)(addr & 0x00FF);
  MBM_TX_FRAME_FC06_DATA_HI = (u8)((reg_val >> 8) & 0x00FF);
  MBM_TX_FRAME_FC06_DATA_LO = (u8)(reg_val & 0x00FF);

  MBM_OS_Tx(pch);

  MBM_OS_RxWait(pch, &err);

  MBM_ParseResp(pch, &err);

  return (err);
}
/*
*********************************************************************************************************
*                                           MBS_FC08_Loopback()
*
* Description : The LOOPBACK function contains various diagnostic codes that perform specific
* actions. This function processes individual diagnostic requests and formats the response message
*               frame accordingly.  Unimplemented diagnostic codes will return an Illegal Data Value
*               Exception Response Code (03).
*
*********************************************************************************************************
*/
u16 MBM_Diagnostics(MODBUS_CH *pch, u8 slave_addr, u16 fnct, u16 fnct_data)
{
  u16 err;

  MBM_TX_FRAME_NBYTES = 6;
  MBM_TX_FRAME_SLAVE_ADDR = slave_addr;
  MBM_TX_FRAME_FC = MODBUS_FC08_LOOPBACK;
  MBM_TX_FRAME_FC08_FNCT_HI = (u8)((fnct >> 8) & 0x00FF);
  MBM_TX_FRAME_FC08_FNCT_LO = (u8)(fnct & 0x00FF);
  MBM_TX_FRAME_FC08_FNCT_DATA_HI = (u8)((fnct_data >> 8) & 0x00FF);
  MBM_TX_FRAME_FC08_FNCT_DATA_LO = (u8)(fnct_data & 0x00FF);

  MBM_OS_Tx(pch);

  MBM_OS_RxWait(pch, &err);

  MBM_ParseResp(pch, &err);

  return (err);
}
/*
*********************************************************************************************************
*                                       MBM_WriteMultipleCoils()
*

* Note(s)     : 1) TX command format:             Example:
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
*               2) RX reply format:               Example:
*                  <slave address>                0x11
*                  <function code>                0x0F
*                  <Coil address HI>              0x00
*                  <Coil address LO>              0x13
*                  <coils count HI>               0x00
*                  <coils count LO>               0x0A
*********************************************************************************************************
*/
u16 MBM_WriteMultipleCoils(MODBUS_CH *pch, u8 slave_addr, u16 start_addr, u8 *p_coil_tbl,
                           u16 nbr_coils)
{
  u16 err;
  u8  nbr_bytes;
  u8  i;
  u8 *p_data;

  nbr_bytes = (u8)(((nbr_coils - 1) / 8) + 1);
  MBM_TX_FRAME_NBYTES = 7 + nbr_bytes;
  MBM_TX_FRAME_SLAVE_ADDR = slave_addr;
  MBM_TX_FRAME_FC = MODBUS_FC15_WR_MULTIPLE_COIL;
  MBM_TX_FRAME_FC15_ADDR_HI = (u8)((start_addr >> 8) & 0x00FF);
  MBM_TX_FRAME_FC15_ADDR_LO = (u8)(start_addr & 0x00FF);
  MBM_TX_FRAME_FC15_NBR_POINTS_HI = (u8)((nbr_coils >> 8) & 0x00FF);
  MBM_TX_FRAME_FC15_NBR_POINTS_LO = (u8)(nbr_coils & 0x00FF);
  MBM_TX_FRAME_FC15_BYTE_CNT = nbr_bytes;
  p_data = MBM_TX_FRAME_FC15_DATA;

  for (i = 0; i < nbr_bytes; i++)
  {
    *p_data++ = *p_coil_tbl++;
  }

  MBM_OS_Tx(pch);

  MBM_OS_RxWait(pch, &err);

  MBM_ParseResp(pch, &err);

  return (err);
}
/*
*********************************************************************************************************
*                                    MBM_WriteMultipleHoldingRegs()
*
*
* Note(s)     : 1) TX command format:             Example:
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
*               2) RX reply format:               Example:
*                  <slave address>                0x11
*                  <function code>                0x10
*                  <start address HI>             0x00
*                  <start address LO>             0x01
*                  <registers count HI>           0x00
*                  <registers count LO>           0x02
*********************************************************************************************************
*/
u16 MBM_WriteMultipleHoldingRegs(MODBUS_CH *pch, u8 slave_addr, u16 start_addr, u16 *p_reg_tbl,
                                 u16 nbr_regs)
{
  u16 err;
  u8  nbr_bytes;
  u8  i;
  u8 *p_data;

  nbr_bytes = (u8)(nbr_regs * sizeof(u16));
  MBM_TX_FRAME_NBYTES = 7 + nbr_bytes;
  MBM_TX_FRAME_SLAVE_ADDR = slave_addr;
  MBM_TX_FRAME_FC = MODBUS_FC16_WR_MULTIPLE_REG;
  MBM_TX_FRAME_FC16_ADDR_HI = (u8)((start_addr >> 8) & 0x00FF);
  MBM_TX_FRAME_FC16_ADDR_LO = (u8)(start_addr & 0x00FF);
  MBM_TX_FRAME_FC16_NBR_REGS_HI = (u8)((nbr_regs >> 8) & 0x00FF);
  MBM_TX_FRAME_FC16_NBR_REGS_LO = (u8)(nbr_regs & 0x00FF);
  MBM_TX_FRAME_FC16_BYTE_CNT = nbr_bytes;
  p_data = MBM_TX_FRAME_FC16_DATA;

  for (i = 0; i < nbr_bytes; i++)
  {
    *p_data++ = (u8)((*p_reg_tbl >> 8) & 0x00FF);
    *p_data++ = (u8)(*p_reg_tbl & 0x00FF);
    p_reg_tbl++;
  }

  MBM_OS_Tx(pch);

  MBM_OS_RxWait(pch, &err);

  MBM_ParseResp(pch, &err);

  return (err);
}
/*
*********************************************************************************************************
*                    format:                      Example:
*                  <slave address>                0x11
*                  <function code>                0x01 or 0x02
*                  <byte count>                   0x05
*                  <Data Coils>                   0xCD  (Bit set to 1 means ON, Bit cleared means ==
* OFF) <Data Coils>                   0x6B  (Bit set to 1 means ON, Bit cleared means == OFF) <Data
* Coils>                   0xB2  (Bit set to 1 means ON, Bit cleared means == OFF) <Data Coils>
*              0x0E  (Bit set to 1 means ON, Bit cleared means == OFF) <Data Coils>   0x1B  (Bit set
* to 1 means ON, Bit cleared means == OFF)
*********************************************************************************************************
*/
static u16 MBM_ReadCoil_Resp(MODBUS_CH *pch)
{
  u8  slave_addr;
  u8  fnct_code;
  u8  byte_cnt;
  u8  nbr_points;
  u8  i;
  u8 *psrc;
  u8 *ptbl;

  slave_addr = MBM_TX_FRAME_SLAVE_ADDR;
  if (slave_addr != pch->RxData[0])
    return (MODBUS_ERR_SLAVE_ADDR);

  fnct_code = MBM_TX_FRAME_FC;
  if (fnct_code != pch->RxData[1])
    return (MODBUS_ERR_FC);

  nbr_points = (MBM_TX_FRAME_FC01_NBR_POINTS_HI << 8) + MBM_TX_FRAME_FC01_NBR_POINTS_LO;

  byte_cnt = (u8)((nbr_points - 1) / 8) + 1;
  if (byte_cnt != pch->RxData[2])
    return (MODBUS_ERR_BYTE_COUNT);

  pch->RespDataNum = byte_cnt;
  ptbl = pch->RespData;
  psrc = &pch->RxData[3];
  for (i = 0; i < byte_cnt; i++)
  {
    *ptbl++ = *psrc++;
  }

  return (MODBUS_ERR_NONE);
}
/*
*********************************************************************************************************
*
*                   format:                       Example:
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
static u16 MBM_ReadReg_Resp(MODBUS_CH *pch)
{
  u8  slave_addr;
  u8  fnct_code;
  u8  byte_cnt;
  u8  nbr_points;
  u8  i;
  u8 *psrc;
  u8 *ptbl;

  slave_addr = MBM_TX_FRAME_SLAVE_ADDR;
  if (slave_addr != pch->RxData[0])
    return (MODBUS_ERR_SLAVE_ADDR);

  fnct_code = MBM_TX_FRAME_FC;
  if (fnct_code != pch->RxData[1])
    return (MODBUS_ERR_FC);

  nbr_points = (MBM_TX_FRAME_FC01_NBR_POINTS_HI << 8) + MBM_TX_FRAME_FC01_NBR_POINTS_LO;

  byte_cnt = nbr_points * sizeof(u16);
  if (byte_cnt != pch->RxData[2])
    return (MODBUS_ERR_BYTE_COUNT);

  pch->RespDataNum = byte_cnt;
  ptbl = pch->RespData;
  psrc = &pch->RxData[3];
  for (i = 0; i < byte_cnt; i++)
  {
    *ptbl++ = *psrc++;
    *ptbl++ = *psrc++;
  }

  return (MODBUS_ERR_NONE);
}
/*
*********************************************************************************************************
*
*                       format:                  Example:
*                  <slave address>                0x11
*                  <function code>                0x05
*                  <Coil address HI>              0x00
*                  <Coil address LO>              0xAC
*                  <Force coil value HI>          0xFF
*                  <Force coil value LO>          0x00
*********************************************************************************************************
*/
static u16 MBM_WriteSingleCoil_Resp(MODBUS_CH *pch)
{
  u8 slave_addr;
  u8 fnct_code;
  u8 coil_addr_hi;
  u8 coil_addr_lo;

  slave_addr = MBM_TX_FRAME_SLAVE_ADDR;
  if (slave_addr != pch->RxData[0])
    return (MODBUS_ERR_SLAVE_ADDR);

  fnct_code = MBM_TX_FRAME_FC;
  if (fnct_code != pch->RxData[1])
    return (MODBUS_ERR_FC);

  coil_addr_hi = MBM_TX_FRAME_FC05_ADDR_HI;
  coil_addr_lo = MBM_TX_FRAME_FC05_ADDR_LO;
  if ((coil_addr_hi != pch->RxData[2]) || (coil_addr_lo != pch->RxData[3]))
  {
    return (MODBUS_ERR_COIL_ADDR);
  }

  return (MODBUS_ERR_NONE);
}
/*
*********************************************************************************************************
*
*                      format:                    Example:
*                  <slave address>                0x11
*                  <function code>                0x0F
*                  <Coil address HI>              0x00
*                  <Coil address LO>              0x13
*                  <coils count HI>               0x00
*                  <coils count LO>               0x0A
*********************************************************************************************************
*/
static u16 MBM_WriteMultipleCoils_Resp(MODBUS_CH *pch)
{
  u8 slave_addr;
  u8 fnct_code;
  u8 coil_addr_hi;
  u8 coil_addr_lo;
  u8 coil_qty_hi;
  u8 coil_qty_lo;

  slave_addr = MBM_TX_FRAME_SLAVE_ADDR;
  if (slave_addr != pch->RxData[0])
    return (MODBUS_ERR_SLAVE_ADDR);

  fnct_code = MBM_TX_FRAME_FC;
  if (fnct_code != pch->RxData[1])
    return (MODBUS_ERR_FC);

  coil_addr_hi = MBM_TX_FRAME_FC05_ADDR_HI;
  coil_addr_lo = MBM_TX_FRAME_FC05_ADDR_LO;
  if ((coil_addr_hi != pch->RxData[2]) || (coil_addr_lo != pch->RxData[3]))
    return (MODBUS_ERR_COIL_ADDR);

  coil_qty_hi = MBM_TX_FRAME_FC15_NBR_POINTS_HI;
  coil_qty_lo = MBM_TX_FRAME_FC15_NBR_POINTS_LO;
  if ((coil_qty_hi != pch->RxData[4]) || (coil_qty_lo != pch->RxData[5]))
    return (MODBUS_ERR_COIL_QTY);

  return (MODBUS_ERR_NONE);
}
/*
*********************************************************************************************************
*
*                     format:                     Example:
*                  <slave address>                0x11
*                  <function code>                0x06
*                  <start address HI>             0x00
*                  <start address LO>             0x01
*                  <Register value HI>            0x00
*                  <Register value LO>            0x03
*********************************************************************************************************
*/
static u16 MBM_WriteSingleReg_Resp(MODBUS_CH *pch)
{
  u8 slave_addr;
  u8 fnct_code;
  u8 reg_addr_hi;
  u8 reg_addr_lo;

  slave_addr = MBM_TX_FRAME_SLAVE_ADDR;
  if (slave_addr != pch->RxData[0])
    return (MODBUS_ERR_SLAVE_ADDR);

  fnct_code = MBM_TX_FRAME_FC;
  if (fnct_code != pch->RxData[1])
    return (MODBUS_ERR_FC);

  reg_addr_hi = MBM_TX_FRAME_FC06_ADDR_HI;
  reg_addr_lo = MBM_TX_FRAME_FC06_ADDR_LO;
  if ((reg_addr_hi != pch->RxData[2]) || (reg_addr_lo != pch->RxData[3]))
    return (MODBUS_ERR_REG_ADDR);

  return (MODBUS_ERR_NONE);
}
/*
*********************************************************************************************************
*                      format:                   Example:
*                  <slave address>                0x11
*                  <function code>                0x10
*                  <start address HI>             0x00
*                  <start address LO>             0x01
*                  <registers count HI>           0x00
*                  <registers count LO>           0x02
*********************************************************************************************************
*/
static u16 MBM_WriteMultipleReg_Resp(MODBUS_CH *pch)
{
  u8 slave_addr;
  u8 fnct_code;
  u8 reg_addr_hi;
  u8 reg_addr_lo;
  u8 nbr_reg_hi;
  u8 nbr_reg_lo;

  slave_addr = MBM_TX_FRAME_SLAVE_ADDR;
  if (slave_addr != pch->RxData[0])
    return (MODBUS_ERR_SLAVE_ADDR);

  fnct_code = MBM_TX_FRAME_FC;
  if (fnct_code != pch->RxData[1])
    return (MODBUS_ERR_FC);

  reg_addr_hi = MBM_TX_FRAME_FC16_ADDR_HI;
  reg_addr_lo = MBM_TX_FRAME_FC16_ADDR_LO;
  if ((reg_addr_hi != pch->RxData[2]) || (reg_addr_lo != pch->RxData[3]))
    return (MODBUS_ERR_REG_ADDR);

  nbr_reg_hi = MBM_TX_FRAME_FC16_NBR_REGS_HI;
  nbr_reg_lo = MBM_TX_FRAME_FC16_NBR_REGS_LO;
  if ((nbr_reg_hi != pch->RxData[4]) || (nbr_reg_lo != pch->RxData[5]))
    return (MODBUS_ERR_NBR_REG);

  return (MODBUS_ERR_NONE);
}
//-----------------------------------------------------------------------------------------------------------
static u16 MBM_Diag_Resp(MODBUS_CH *pch)
{
  u8  slave_addr;
  u8  fnct_code;
  u8  sub_code_hi;
  u8  sub_code_lo;
  u16 sub_code;
  u8  data_hi;
  u8  data_lo;
  u8 *ptbl;

  slave_addr = MBM_TX_FRAME_SLAVE_ADDR;
  if (slave_addr != pch->RxData[0])
    return (MODBUS_ERR_SLAVE_ADDR);

  fnct_code = MBM_TX_FRAME_FC;
  if (fnct_code != pch->RxData[1])
    return (MODBUS_ERR_FC);

  sub_code_hi = MBM_TX_FRAME_FC08_FNCT_HI;
  sub_code_lo = MBM_TX_FRAME_FC08_FNCT_LO;
  if ((sub_code_hi != pch->RxData[2]) || (sub_code_lo != pch->RxData[3]))
    return (MODBUS_ERR_SUB_FNCT);

  sub_code = ((u16)sub_code_hi << 8) + (u16)sub_code_lo;
  data_hi = MBM_TX_FRAME_FC08_FNCT_DATA_HI;
  data_lo = MBM_TX_FRAME_FC08_FNCT_DATA_LO;

  pch->RespDataNum = 2;
  ptbl = pch->RespData;

  switch (sub_code)
  {
  case MODBUS_FC08_LOOPBACK_QUERY: /* Return Query function code - no need to do anything.     */
  case MODBUS_FC08_LOOPBACK_CLR_CTR:
    if ((data_hi != pch->RxData[4]) || (data_lo != pch->RxData[5]))
    {
      *ptbl = 0;
      return (MODBUS_ERR_DIAG);
    }
    else
    {
      *ptbl++ = pch->RxData[4];
      *ptbl++ = pch->RxData[5];
      return (MODBUS_ERR_NONE);
    }

  case MODBUS_FC08_LOOPBACK_BUS_MSG_CTR: /* Return the message count in diag information field. */
  case MODBUS_FC08_LOOPBACK_BUS_CRC_CTR: /* Return the CRC error count in diag information field. */
  case MODBUS_FC08_LOOPBACK_BUS_EXCEPT_CTR: /* Return exception count in diag information field. */
  case MODBUS_FC08_LOOPBACK_SLAVE_MSG_CTR:  /* Return slave message count in diag information field.
                                             */
  case MODBUS_FC08_LOOPBACK_SLAVE_NO_RESP_CTR: /* Return no response count in diag information
                                                  field.      */
    *ptbl++ = pch->RxData[4];
    *ptbl++ = pch->RxData[5];
    return (MODBUS_ERR_NONE);

  default:
    return (MODBUS_ERR_DIAG);
  }
}
//-----------------------------------------------------------------------------------------------------------
void MBM_Tx(MODBUS_CH *pch)
{
  if (pch->Mode == MODBUS_MODE_ASCII)
    MB_ASCII_Tx(pch);

  if (pch->Mode == MODBUS_MODE_RTU)
    MB_RTU_Tx(pch);

  if (pch->Mode == MODBUS_MODE_TCP)
  {
    pch->T_Id++;
    MB_TCP_Tx(pch);
  }
}
//-----------------------------------------------------------------------------------------------------------
void MBM_TxFlush(MODBUS_CH *pch, u8 slave_addr)
{
  MBM_OS_TxFlush(pch, slave_addr);
}