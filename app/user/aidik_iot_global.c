#include "aidik_iot_global.h"
#include "aidik_iot_type.h"

AidikIotDevice iotgo_device;

uint32 ICACHE_FLASH_ATTR iotgoBootReason(void)
{
    static struct rst_info* rstp = NULL;
    if (!rstp) 
    {
        rstp = system_get_rst_info();
    }
    return rstp->reason;
}   

bool iotgoIsXrstKeepGpio(void)
{
    uint32 device_rst_reason = iotgoBootReason();
    if (REASON_SOFT_RESTART == device_rst_reason
        || REASON_SOFT_WDT_RST == device_rst_reason
        || REASON_EXCEPTION_RST == device_rst_reason)
    {
        return true;
    }
    return false;
}

