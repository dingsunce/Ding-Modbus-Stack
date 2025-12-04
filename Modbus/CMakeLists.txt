
file(GLOB_RECURSE modbus_src         *.c)

file(GLOB_RECURSE exclude_port_src   Platform/*.c)
list(REMOVE_ITEM  modbus_src         ${exclude_port_src})

set(modbus_port_src                  Platform/UnitTest/mb_bsp.c)

add_library(MODBUS STATIC ${modbus_src} ${modbus_port_src})
