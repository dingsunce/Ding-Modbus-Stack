/*!*****************************************************************************
 * file		mb_bsp.c
 * $Author: sunce.ding
 *******************************************************************************/

#include "mb_bsp.h"
#include "define.h"
#include "mb_core.h"
#include "usart.h"
#include "mb_app.h"
//-----------------------------------------------------------------------------------------------------------
static void EnableTimer(void)
{
  // clear pending interrupt first
  LL_TIM_ClearFlag_CC1(TIM17);
  LL_TIM_EnableIT_CC1(TIM17);
  LL_TIM_CC_EnableChannel(TIM17, LL_TIM_CHANNEL_CH1);
}
//-----------------------------------------------------------------------------------------------------------
static void DisableTimer(void)
{
  // clear pending interrupt first
  LL_TIM_ClearFlag_CC1(TIM17);
  LL_TIM_DisableIT_CC1(TIM17);
  LL_TIM_CC_DisableChannel(TIM17, LL_TIM_CHANNEL_CH1);
}
//-----------------------------------------------------------------------------------------------------------
static void StartTimer(u16 nTimeOut)
{
  // the type of timer is 32bits, but we shrink the type to 16bits
  // so calculate the u16 type value first, and then assign to 32bits timer
  // In this approach, we will generate the overflow value(e.g. 65537 is 2 now)
  u32 time17 = LL_TIM_GetCounter(TIM17);
  u16 time = (u16)(time17 + nTimeOut);
  LL_TIM_OC_SetCompareCH1(TIM17, time);
  EnableTimer();
}
//-----------------------------------------------------------------------------------------------------------
void TIM17_IRQHandler(void)
{
  MODBUS_CH *pch = &MB_ChTbl[0];
  if (LL_TIM_IsActiveFlag_CC1(TIM17) == SET)
  {
    LL_TIM_ClearFlag_CC1(TIM17);
    DisableTimer();
    MB_UART_RxTimeout(pch);
  }
}
//-----------------------------------------------------------------------------------------------------------
void USART1_IRQHandler(void)
{
  u8         byte;
  MODBUS_CH *pch = &MB_ChTbl[0];
  if (pch->Mode == MODBUS_MODE_TCP)
    return;

  if (LL_USART_IsActiveFlag_ORE(USART1))
    LL_USART_ClearFlag_ORE(USART1);

  if (LL_USART_IsActiveFlag_NE(USART1))
    LL_USART_ClearFlag_NE(USART1);

  if (LL_USART_IsActiveFlag_FE(USART1))
    LL_USART_ClearFlag_FE(USART1);

  if (LL_USART_IsActiveFlag_RXNE(USART1))
  {
    // RXNE flag is cleared by reading of RDR register */
    byte = (u8)LL_USART_ReceiveData8(USART1);
    // In case of idle, byte 0x00 will be ignored by LL

    MB_UART_RxByte(pch, byte);

    if (pch->Mode == MODBUS_MODE_RTU)
      StartTimer(pch->RTU_Timeout);
  }

  if (LL_USART_IsActiveFlag_TC(USART1))
  {
    LL_USART_ClearFlag_TC(USART1);
    MB_UART_TxByte(pch);
  }
}
//-----------------------------------------------------------------------------------------------------------
void MB_BSP_UartExit(void)
{
  u8         ch;
  MODBUS_CH *pch = &MB_ChTbl[0];

  for (ch = 0; ch < MODBUS_CFG_MAX_CH; ch++)
  {
    if (pch->Mode != MODBUS_MODE_TCP)
    {
      MB_BSP_UartTxIntDisable(pch);
      MB_BSP_UartRxIntDisable(pch);
    }

    pch++;
  }
}
//-----------------------------------------------------------------------------------------------------------
void MB_BSP_UartPortCfg(MODBUS_CH *pch, u32 baud, u8 parity)
{
  if (pch->Mode == MODBUS_MODE_TCP)
    return;

  LL_USART_DeInit(USART1);
  
  LL_USART_InitTypeDef USART_InitStruct = {0};

  USART_InitStruct.PrescalerValue = LL_USART_PRESCALER_DIV1;
  USART_InitStruct.BaudRate = baud;
  
  if (parity == 0)
  {
    USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_9B;
    USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
    USART_InitStruct.Parity = LL_USART_PARITY_EVEN;
  }
  else if (parity == 1)
  {
    USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_9B;
    USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
    USART_InitStruct.Parity = LL_USART_PARITY_ODD;
  }
  else if (parity == 2)
  {
    USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_9B;
    USART_InitStruct.StopBits = LL_USART_STOPBITS_2;
    USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  }
  else if (parity == 3)
  {
    USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
    USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
    USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  }
  
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART1, &USART_InitStruct);
  LL_USART_SetTXFIFOThreshold(USART1, LL_USART_FIFOTHRESHOLD_1_8);
  LL_USART_SetRXFIFOThreshold(USART1, LL_USART_FIFOTHRESHOLD_1_8);
  LL_USART_DisableFIFO(USART1);
  LL_USART_ConfigAsyncMode(USART1);

  LL_USART_Enable(USART1);

  /* Polling USART1 initialisation */
  while ((!(LL_USART_IsActiveFlag_TEACK(USART1))) || (!(LL_USART_IsActiveFlag_REACK(USART1))))
  {
  }

  MB_BSP_UartRxIntEnable(pch);
  LL_TIM_EnableCounter(TIM17);
}
//-----------------------------------------------------------------------------------------------------------
void MB_BSP_UartRxIntDisable(MODBUS_CH *pch)
{
  LL_USART_DisableIT_RXNE_RXFNE(USART1);
}
//-----------------------------------------------------------------------------------------------------------
void MB_BSP_UartRxIntEnable(MODBUS_CH *pch)
{
  LL_USART_EnableIT_RXNE(USART1);
}
//-----------------------------------------------------------------------------------------------------------
void MB_BSP_UartTx(MODBUS_CH *pch, u8 c)
{
  LL_USART_TransmitData8(USART1, c);
}
//-----------------------------------------------------------------------------------------------------------
void MB_BSP_UartTxIntDisable(MODBUS_CH *pch)
{
  LL_USART_DisableIT_TC(USART1);
}
//-----------------------------------------------------------------------------------------------------------
void MB_BSP_UartTxIntEnable(MODBUS_CH *pch)
{
  LL_USART_EnableIT_TC(USART1);
}
//-----------------------------------------------------------------------------------------------------------
void MB_BSP_TcpTx(MODBUS_CH *pch)
{
  MB_APP_TcpTx(pch);
  
  //reset buffer point and number
  pch->TxBufNum = 0;
  pch->TxBufPtr = &pch->TxBuf[0];
}
