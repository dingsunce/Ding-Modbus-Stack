. The main code is from uc-modbus-2.14.00, please refer to https://github.com/weston-embedded/uC-Modbus

. Support RTU master/slave, TCP master/slave, ASCII master/slave

. It can run under DING OS or other preemptive os, like ucosIII, rt-thread, free rtos, linux, window

. For master:

  It provides MBM_ReadCoils, MBM_ReadDiscreteInputs... api to trgger the read/write operation

  In DING os environment MBM_APP_RespCallback() will receives the response.

  In other preemptive os environment, the api itself would sleep until it gets the response or timeout

. For slave:
  
  mb_slave_app set slave services option, that it: does it support coil read, which register does it have...
  


