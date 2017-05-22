#ifndef _AIDIK_IOT_TYPE_H__
#define _AIDIK_IOT_TYPE_H__

#include "aidik_iot_sdk.h"

typedef enum {
    AIDIK_IOT_DEVICE_MODEL_SIZE      = 20 + 1,
    AIDIK_IOT_DEVICEID_SIZE          = 10 + 1,
    AIDIK_IOT_OWNER_UUID_SIZE        = 36 + 1,
} AidikIotDeviceConstant;


typedef struct
{
    char device_model[AIDIK_IOT_DEVICE_MODEL_SIZE];
    char deviceid[AIDIK_IOT_DEVICEID_SIZE];
    char factory_apikey[AIDIK_IOT_OWNER_UUID_SIZE];
}FactoryData;

typedef struct
{
    char deviceid[AIDIK_IOT_DEVICEID_SIZE];
    char factory_apikey[AIDIK_IOT_OWNER_UUID_SIZE];
    char owner_uuid[AIDIK_IOT_OWNER_UUID_SIZE];

    void (*earlyInit)(void);
    void (*postCenter)(os_event_t *events);
    void (*mqttInit)(void);
}AidikIotDevice;
#endif
