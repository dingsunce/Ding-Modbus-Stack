/*!*****************************************************************************
 * file		mb_slave_app.c
 * $Author: sunce.ding
 *******************************************************************************/

#include "mb_core.h"

bool MBS_APP_IsSupportFC(u16 FC)
{
  bool result = false;
  switch (FC)
  {
  case MODBUS_FC01_RD_COIL:
  case MODBUS_FC02_RD_DI:
  case MODBUS_FC03_RD_HOLDING_REG:
  case MODBUS_FC04_RD_INPUT_REG:
  case MODBUS_FC05_WR_SINGLE_COIL:
  case MODBUS_FC06_WR_SINGLE_REG:
  case MODBUS_FC08_LOOPBACK:
  case MODBUS_FC15_WR_MULTIPLE_COIL:
  case MODBUS_FC16_WR_MULTIPLE_REG:
  case MODBUS_FC20_RD_FILE:
  case MODBUS_FC21_WR_FILE:
    result = true;
    break;
  }

  return result;
}

bool MBS_APP_ReadCoilVerify(u16 start_coil_no, u16 nbr_coils)
{
  return false;
}

bool MBS_APP_ReadCoil(u16 coil_no)
{
  return false;
}

bool MBS_APP_WriteCoilVerify(u16 start_coil_no, u16 nbr_coils)
{
  return true;
}

void MBS_APP_WriteCoil(u16 coil_no, bool coil_val)
{
}

bool MBS_APP_ReadDiscreteInputVerify(u16 start_reg_no, u16 nbr_regs)
{
  return false;
}

bool MBS_APP_ReadDiscreteInput(u16 di_no)
{
  return false;
}

bool MBS_APP_ReadHoldingRegVerify(u16 start_reg_no, u16 nbr_regs)
{
  return false;
}

u16 MBS_APP_ReadHoldingReg(u16 reg_no)
{
  return (1);
}

bool MBS_APP_ReadInputRegVerify(u16 start_reg_no, u16 nbr_regs)
{
  return false;
}

u16 MBS_APP_ReadInputReg(u16 reg_no)
{
  return (1);
}

void MBS_APP_WriteHoldingReg(u16 reg_no, u16 reg_val_16)
{
}

bool MBS_APP_WriteHoldingRegVerify(u16 start_reg_no, u16 nbr_regs)
{
  return true;
}

u16 MBS_APP_ReadFileRecord(u16 file_nbr, u16 record_nbr, u16 ix, u8 *perr)
{
  *perr = MODBUS_ERR_NONE;
  return (0);
}

void MBS_APP_WriteFileRecord(u16 file_nbr, u16 record_nbr, u16 ix, u16 val, u8 *perr)
{
  *perr = MODBUS_ERR_NONE;
}
