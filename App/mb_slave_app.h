/*!*****************************************************************************
 * file		mb_slave_app.h
 * $Author: sunce.ding
 *******************************************************************************/

#ifndef MB_SLAVE_APP_H
#define MB_SLAVE_APP_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "mb_core.h"

  bool MBS_APP_IsSupportFC(u16 FC);

  bool MBS_APP_ReadCoil(u16 coil_no);
  ;

  bool MBS_APP_ReadCoilVerify(u16 start_coil_no, u16 nbr_coils);

  void MBS_APP_WriteCoil(u16 coil_no, bool coil_val);

  bool MBS_APP_WriteCoilVerify(u16 start_coil_no, u16 nbr_coils);

  bool MBS_APP_ReadDiscreteInput(u16 di_no);

  bool MBS_APP_ReadDiscreteInputVerify(u16 start_reg_no, u16 nbr_regs);

  u16 MBS_APP_ReadInputReg(u16 reg_no);

  bool MBS_APP_ReadInputRegVerify(u16 start_reg_no, u16 nbr_regs);

  u16 MBS_APP_ReadHoldingReg(u16 reg_no);

  bool MBS_APP_ReadHoldingRegVerify(u16 start_reg_no, u16 nbr_regs);

  void MBS_APP_WriteHoldingReg(u16 reg_no, u16 reg_val_16);

  bool MBS_APP_WriteHoldingRegVerify(u16 start_reg_no, u16 nbr_regs);

  u16 MBS_APP_ReadFileRecord(u16 file_nbr, u16 record_nbr, u16 ix, u8 *perr);

  void MBS_APP_WriteFileRecord(u16 file_nbr, u16 record_nbr, u16 ix, u16 value, u8 *perr);

#ifdef __cplusplus
}
#endif

#endif
