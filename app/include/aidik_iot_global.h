#ifndef _AIDIK_IOT_GLOBAL_H_
#define _AIDIK_IOT_GLOBAL_H_

#include "aidik_iot_sdk.h"

extern AidikIotDevice iotgo_device;

uint32 ICACHE_FLASH_ATTR iotgoBootReason(void);
bool iotgoIsXrstKeepGpio(void);


#endif
