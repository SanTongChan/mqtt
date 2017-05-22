#include <ets_sys.h>
#include <osapi.h>
#include <gpio.h>
#include <os_type.h>
#include <user_interface.h>
#include <espconn.h>
#include <mem.h>
#include <spi_flash.h>
#include <upgrade.h>
#include <smartconfig.h>
#include <json/jsonparse.h>
#include <json/jsontree.h>
#include "user_config.h"
#include "aidik_iot_config.h"
#include "aidik_iot_global.h"
#include "aidik_iot_type.h"
#include "mqtt.h"
#include "wifi.h"
#include "config.h"
#include "debug.h"


#if defined(COMPILE_AIDIK_FWSW_01)

#define KEY_DEBONCE_INTERVAL    (25)
#define KEY_DEBONCE_FIRST_TIME  (KEY_DEBONCE_INTERVAL * 2)
#define KEY_DEBONCE_SECOND_TIME (KEY_DEBONCE_INTERVAL * 2)
#define KEY_LONG_PRESS_TIME     (5000)

MQTT_Client mqttClient;
static uint8 switch_state = SWITCH_OFF;
static uint32 key_pressed_level_counter = 0;
static uint32 key_released_level_counter = 0;
static os_timer_t key_isr_timer;

static inline void ICACHE_FLASH_ATTR switchOFF(void)
{
    GPIO_OUTPUT_SET(SWITCH_OUTPUT_GPIO, SWITCH_OFF);
    switch_state = SWITCH_OFF;
}
static inline void ICACHE_FLASH_ATTR switchON(void)
{
    GPIO_OUTPUT_SET(SWITCH_OUTPUT_GPIO, SWITCH_ON);
    switch_state = SWITCH_ON;
}

static void ICACHE_FLASH_ATTR keyISRStage2(void *arg)
{
    static bool long_press_processed = false;
    
    /* Read pin */
    if (SYS_KEY_GPIO_RELEASED_LEVEL == GPIO_INPUT_GET(SWITCH_INPUT_GPIO)) 
    {
        if (key_pressed_level_counter > 0)
        {
            key_released_level_counter++;
        }
    }
    else
    {
        if (0 == key_released_level_counter)
        {
            key_pressed_level_counter++;
            if (key_pressed_level_counter >= (KEY_LONG_PRESS_TIME / KEY_DEBONCE_INTERVAL))
            {
                key_pressed_level_counter = (KEY_LONG_PRESS_TIME / KEY_DEBONCE_INTERVAL);
#if 0
                IoTgoDeviceMode current_mode = iotgoDeviceMode();
                if (DEVICE_MODE_SETTING_SELFAP != current_mode)
                {
                    if (!long_press_processed)
                    {
                        long_press_processed = true;
                        system_os_post(IOTGO_NETWORK_CENTER, 
                            SIG_NETWORK_ENTER_SETTING_SELFAP_MODE, 0);
                    }
                }
                else
                {
                    long_press_processed = true;
                    //iotgoInfo("long key pressed do nothing!");
                }
#endif
                long_press_processed = true;
            }
        }
    }

    if (key_released_level_counter >= (KEY_DEBONCE_SECOND_TIME / KEY_DEBONCE_INTERVAL))
    {
        
#if 0
        if (key_pressed_level_counter >= (KEY_DEBONCE_FIRST_TIME / KEY_DEBONCE_INTERVAL)
            && !long_press_processed)
        {
            IoTgoDeviceMode current_mode = iotgoDeviceMode();
            if (DEVICE_MODE_SETTING_SELFAP == current_mode)
            {
                system_os_post(IOTGO_NETWORK_CENTER, SIG_NETWORK_EXIT_SETTING_SELFAP_MODE, 0);
            }
            else
            {
                if (SWITCH_ON == switch_state)
                {
                    switchOFF();
                }
                else
                {
                    switchON();
                }                
                if (DEVICE_MODE_WORK_AP_OK == current_mode 
                        || DEVICE_MODE_WORK_NORMAL == current_mode
                        || DEVICE_MODE_WORK_INIT== current_mode)
                {
                    system_os_post(IOTGO_DEVICE_CENTER, SIG_DEVICE_UPDATE_BY_LOCAL_UDP, 0); 
                }
                if (DEVICE_MODE_WORK_NORMAL == current_mode) 
                {
                    system_os_post(IOTGO_DEVICE_CENTER, SIG_DEVICE_UPDATE_BY_LOCAL, 0);   
                }
            }
        }
#endif       
        if(!long_press_processed)
        {
            if (SWITCH_ON == switch_state)
            {
                INFO("switch is off");
                switchOFF();
            }
            else
            {
                INFO("switch is on");
                switchON();
            }   
        }
        long_press_processed = false;
        os_timer_disarm(&key_isr_timer);
        key_released_level_counter = 0;
        key_pressed_level_counter = 0;
        gpio_pin_intr_state_set(GPIO_ID_PIN(SWITCH_INPUT_GPIO), SYS_KEY_GPIO_TRIGGER_METHOD);
    }
}

static void keyISR(void *pdata)
{
    uint32 gpio_status;
    ETS_GPIO_INTR_DISABLE();
    
    gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
    if (gpio_status & BIT((uint32)pdata)) 
    {
        gpio_pin_intr_state_set(GPIO_ID_PIN(SWITCH_INPUT_GPIO), GPIO_PIN_INTR_DISABLE);
        
        key_pressed_level_counter = 0;
        key_released_level_counter = 0;
        os_timer_disarm(&key_isr_timer);
        os_timer_setfn(&key_isr_timer, (os_timer_func_t *)keyISRStage2, NULL);
        os_timer_arm(&key_isr_timer, KEY_DEBONCE_INTERVAL, 1);
    }
    
    //clear interrupt status
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);
    ETS_GPIO_INTR_ENABLE();
}

static void ICACHE_FLASH_ATTR interruptInit(void)
{
    /* Init switch input gpio interrupt */
    ETS_GPIO_INTR_DISABLE();
    ETS_GPIO_INTR_ATTACH(keyISR, (void *)SWITCH_INPUT_GPIO);
    PIN_FUNC_SELECT(SWITCH_INPUT_GPIO_NAME, SWITCH_INPUT_GPIO_FUNC);
    GPIO_DIS_OUTPUT(SWITCH_INPUT_GPIO); /* Set as input pin */
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(SWITCH_INPUT_GPIO)); /* clear status */
    gpio_pin_intr_state_set(GPIO_ID_PIN(SWITCH_INPUT_GPIO), SYS_KEY_GPIO_TRIGGER_METHOD); /* enable interrupt */
    ETS_GPIO_INTR_ENABLE();
}

static void ICACHE_FLASH_ATTR switchInitAidikIotDevice(void)
{
    INFO("switch init..............");
    interruptInit(); 
    /* Init switch output relay gpio and set to low as default */
    PIN_FUNC_SELECT(SWITCH_OUTPUT_GPIO_NAME, SWITCH_OUTPUT_GPIO_FUNC);
    if (!iotgoIsXrstKeepGpio())
    {
        switchOFF();
    }
    else
    {
        if (GPIO_INPUT_GET(SWITCH_OUTPUT_GPIO) == SWITCH_ON)
        {
            switchON();
        }
        else
        {
            switchOFF();
        }
    }
}

static void ICACHE_FLASH_ATTR switchDevicePostCenter(os_event_t *events)
{

}

void wifiConnectCb(uint8_t status)
{
	if(status == STATION_GOT_IP){
		MQTT_Connect(&mqttClient);
	} else {
		MQTT_Disconnect(&mqttClient);
	}
}
void mqttConnectedCb(uint32_t *args)
{
	MQTT_Client* client = (MQTT_Client*)args;
	INFO("MQTT: Connected\r\n");
	MQTT_Subscribe(client, "/mqtt/topic/0", 0);
	MQTT_Subscribe(client, "/mqtt/topic/1", 1);
	MQTT_Subscribe(client, "/mqtt/topic/2", 2);

	MQTT_Publish(client, "/mqtt/topic/0", "hello0", 6, 0, 0);
	MQTT_Publish(client, "/mqtt/topic/1", "hello1", 6, 1, 0);
	MQTT_Publish(client, "/mqtt/topic/2", "hello2", 6, 2, 0);

}

void mqttDisconnectedCb(uint32_t *args)
{
	MQTT_Client* client = (MQTT_Client*)args;
	INFO("MQTT: Disconnected\r\n");
}

void mqttPublishedCb(uint32_t *args)
{
	MQTT_Client* client = (MQTT_Client*)args;
	INFO("MQTT: Published\r\n");
}

void mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len, const char *data, uint32_t data_len)
{
	char *topicBuf = (char*)os_zalloc(topic_len+1),
			*dataBuf = (char*)os_zalloc(data_len+1);

	MQTT_Client* client = (MQTT_Client*)args;

	os_memcpy(topicBuf, topic, topic_len);
	topicBuf[topic_len] = 0;

	os_memcpy(dataBuf, data, data_len);
	dataBuf[data_len] = 0;

	INFO("Receive topic: %s, data: %s \r\n", topicBuf, dataBuf);
	os_free(topicBuf);
	os_free(dataBuf);
}

static void switchMqttInit(void)
{
    MQTT_InitConnection(&mqttClient, sysCfg.mqtt_host, sysCfg.mqtt_port, sysCfg.security);
	//MQTT_InitConnection(&mqttClient, "192.168.11.122", 1880, 0);

	MQTT_InitClient(&mqttClient, sysCfg.device_id, sysCfg.mqtt_user, sysCfg.mqtt_pass, sysCfg.mqtt_keepalive, 1);
	//MQTT_InitClient(&mqttClient, "client_id", "user", "pass", 120, 1);

	MQTT_InitLWT(&mqttClient, "/lwt", "offline", 0, 0);
	MQTT_OnConnected(&mqttClient, mqttConnectedCb);
	MQTT_OnDisconnected(&mqttClient, mqttDisconnectedCb);
	MQTT_OnPublished(&mqttClient, mqttPublishedCb);
	MQTT_OnData(&mqttClient, mqttDataCb);

	WIFI_Connect(sysCfg.sta_ssid, sysCfg.sta_pwd, wifiConnectCb);
}

void  ICACHE_FLASH_ATTR aidikIotRegisterCallbackSet(AidikIotDevice *device)
{
    device->earlyInit = switchInitAidikIotDevice;
    device->postCenter = switchDevicePostCenter;
    device->mqttInit = switchMqttInit;
}


#endif

