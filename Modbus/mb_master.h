/*!*****************************************************************************
 * file		mb_master.h
 * $Author: sunce.ding
 *******************************************************************************/

#ifndef MB_MASTER_H
#define MB_MASTER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "mb_core.h"

#define MBM_TX_FRAME_NBYTES (pch->TxDataNum)

#define MBM_TX_FRAME_SLAVE_ADDR (pch->TxData[0])
#define MBM_TX_FRAME_FC         (pch->TxData[1])
#define MBM_TX_FRAME_ADDR_HI    (pch->TxData[2])
#define MBM_TX_FRAME_ADDR_LO    (pch->TxData[3])

#define MBM_TX_FRAME_FC01_ADDR_HI       (pch->TxData[2])
#define MBM_TX_FRAME_FC01_ADDR_LO       (pch->TxData[3])
#define MBM_TX_FRAME_FC01_NBR_POINTS_HI (pch->TxData[4])
#define MBM_TX_FRAME_FC01_NBR_POINTS_LO (pch->TxData[5])

#define MBM_TX_FRAME_FC02_ADDR_HI       (pch->TxData[2])
#define MBM_TX_FRAME_FC02_ADDR_LO       (pch->TxData[3])
#define MBM_TX_FRAME_FC02_NBR_POINTS_HI (pch->TxData[4])
#define MBM_TX_FRAME_FC02_NBR_POINTS_LO (pch->TxData[5])

#define MBM_TX_FRAME_FC03_ADDR_HI       (pch->TxData[2])
#define MBM_TX_FRAME_FC03_ADDR_LO       (pch->TxData[3])
#define MBM_TX_FRAME_FC03_NBR_POINTS_HI (pch->TxData[4])
#define MBM_TX_FRAME_FC03_NBR_POINTS_LO (pch->TxData[5])

#define MBM_TX_FRAME_FC04_ADDR_HI       (pch->TxData[2])
#define MBM_TX_FRAME_FC04_ADDR_LO       (pch->TxData[3])
#define MBM_TX_FRAME_FC04_NBR_POINTS_HI (pch->TxData[4])
#define MBM_TX_FRAME_FC04_NBR_POINTS_LO (pch->TxData[5])

#define MBM_TX_FRAME_FC05_ADDR_HI       (pch->TxData[2])
#define MBM_TX_FRAME_FC05_ADDR_LO       (pch->TxData[3])
#define MBM_TX_FRAME_FC05_FORCE_DATA_HI (pch->TxData[4])
#define MBM_TX_FRAME_FC05_FORCE_DATA_LO (pch->TxData[5])

#define MBM_TX_FRAME_FC06_ADDR_HI   (pch->TxData[2])
#define MBM_TX_FRAME_FC06_ADDR_LO   (pch->TxData[3])
#define MBM_TX_FRAME_FC06_DATA_ADDR (&pch->TxData[4])
#define MBM_TX_FRAME_FC06_DATA_HI   (pch->TxData[4])
#define MBM_TX_FRAME_FC06_DATA_LO   (pch->TxData[5])

#define MBM_TX_FRAME_FC08_FNCT_HI      (pch->TxData[2])
#define MBM_TX_FRAME_FC08_FNCT_LO      (pch->TxData[3])
#define MBM_TX_FRAME_FC08_FNCT_DATA_HI (pch->TxData[4])
#define MBM_TX_FRAME_FC08_FNCT_DATA_LO (pch->TxData[5])

#define MBM_TX_FRAME_FC15_ADDR_HI       (pch->TxData[2])
#define MBM_TX_FRAME_FC15_ADDR_LO       (pch->TxData[3])
#define MBM_TX_FRAME_FC15_NBR_POINTS_HI (pch->TxData[4])
#define MBM_TX_FRAME_FC15_NBR_POINTS_LO (pch->TxData[5])
#define MBM_TX_FRAME_FC15_BYTE_CNT      (pch->TxData[6])
#define MBM_TX_FRAME_FC15_DATA          (&pch->TxData[7])

#define MBM_TX_FRAME_FC16_ADDR_HI     (pch->TxData[2])
#define MBM_TX_FRAME_FC16_ADDR_LO     (pch->TxData[3])
#define MBM_TX_FRAME_FC16_NBR_REGS_HI (pch->TxData[4])
#define MBM_TX_FRAME_FC16_NBR_REGS_LO (pch->TxData[5])
#define MBM_TX_FRAME_FC16_BYTE_CNT    (pch->TxData[6])
#define MBM_TX_FRAME_FC16_DATA        (&pch->TxData[7])

#define MBM_TX_FRAME_DIAG_FNCT_HI      (pch->TxData[2])
#define MBM_TX_FRAME_DIAG_FNCT_LO      (pch->TxData[3])
#define MBM_TX_FRAME_DIAG_FNCT_DATA_HI (pch->TxData[4])
#define MBM_TX_FRAME_DIAG_FNCT_DATA_LO (pch->TxData[5])

  u16 MBM_ReadCoils(MODBUS_CH *pch, u8 slave_addr, u16 start_addr, u16 nbr_coils);

  u16 MBM_ReadDiscreteInputs(MODBUS_CH *pch, u8 slave_addr, u16 start_addr, u16 nbr_di);

  u16 MBM_ReadHoldingRegs(MODBUS_CH *pch, u8 slave_addr, u16 start_addr, u16 nbr_regs);

  u16 MBM_ReadInputRegs(MODBUS_CH *pch, u8 slave_addr, u16 start_addr, u16 nbr_regs);

  u16 MBM_WriteSingleCoil(MODBUS_CH *pch, u8 slave_addr, u16 addr, bool coil_val);

  u16 MBM_WriteSingleHoldingReg(MODBUS_CH *pch, u8 slave_addr, u16 addr, u16 reg_val);

  u16 MBM_Diagnostics(MODBUS_CH *pch, u8 slave_addr, u16 fnct, u16 fnct_data);

  u16 MBM_WriteMultipleCoils(MODBUS_CH *pch, u8 slave_addr, u16 start_addr, u8 *p_coil_tbl, u16 nbr_coils);

  u16 MBM_WriteMultipleHoldingRegs(MODBUS_CH *pch, u8 slave_addr, u16 start_addr, u16 *p_reg_tbl, u16 nbr_regs);

  void MBM_ParseResp(MODBUS_CH *pch, u16 *perr);

  void MBM_TxFlush(MODBUS_CH *pch, u8 slave_addr);

  void MBM_Tx(MODBUS_CH *pch);

#ifdef __cplusplus
}
#endif

#endif
