#ifndef _AIDIK_IOT_CONFIG_H_
#define _AIDIK_IOT_CONFIG_H_

#if defined(COMPILE_AIDIK_FWSW_01)
    #include "device/fw_switch.h"
#else
    #error Sorry! you must select a device!
#endif

#endif
