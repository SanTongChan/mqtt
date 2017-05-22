#if defined(COMPILE_AIDIK_FWSW_01)

#include "aidik_iot_type.h"
#include "aidik_iot_sdk.h"


#define AIDIK_IOT_FM_VERSION "1.5.5"

#define ENABLE_ENTER_SETTING_MODE_WHEN_REPOWER_ON_TIMES
#define GPIO_HIGH       (0x1)
#define GPIO_LOW        (0x0)


#define SYS_KEY_GPIO_TRIGGER_METHOD GPIO_PIN_INTR_NEGEDGE
#define SYS_KEY_GPIO_RELEASED_LEVEL GPIO_HIGH

#define SWITCH_INPUT_GPIO           (0)
#define SWITCH_INPUT_GPIO_NAME      PERIPHS_IO_MUX_GPIO0_U
#define SWITCH_INPUT_GPIO_FUNC      FUNC_GPIO0

#define SWITCH_ON       GPIO_HIGH     
#define SWITCH_OFF      GPIO_LOW         

#define DEV_SLED_ON     GPIO_LOW
#define DEV_SLED_OFF    GPIO_HIGH

#define SWITCH_OUTPUT_GPIO          (12)
#define SWITCH_OUTPUT_GPIO_NAME     PERIPHS_IO_MUX_MTDI_U
#define SWITCH_OUTPUT_GPIO_FUNC     FUNC_GPIO12

#define DEV_SLED_GPIO             (13)
#define DEV_SLED_GPIO_NAME        PERIPHS_IO_MUX_MTCK_U
#define DEV_SLED_GPIO_FUNC        FUNC_GPIO13

void  ICACHE_FLASH_ATTR aidikIotRegisterCallbackSet(AidikIotDevice *device);

#else
#error "You should select the right device to compile `_'!"
#endif /* #ifdef COMPILE_IOTGO_FWSW_01 */

