/*!*****************************************************************************
 * file		mb_slave.h
 * $Author: sunce.ding
 *******************************************************************************/

#ifndef MB_SLAVE_H
#define MB_SLAVE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include  "mb_core.h"


bool  MBS_FCxx_Handler           (MODBUS_CH   *pch);

void  MBS_RxTask                 (MODBUS_CH   *pch);

void  MBS_StatInit               (MODBUS_CH   *pch);


#ifdef __cplusplus
}
#endif

#endif
