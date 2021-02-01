#include <string.h>
#include <time.h>
#include <stdio.h>
#include "utils/atomic.h"
#include <avr/wdt.h>
#include "include/pin_manager.h"
#include "application_manager.h"
#include "mcc.h"
#include "config/IoT_Sensor_Node_config.h"
#include "config/conf_winc.h"
#include "config/mqtt_config.h"
#include "config/cloud_config.h"
#include "cloud/cloud_service.h"
#include "cloud/mqtt_service.h"
#include "cloud/crypto_client/crypto_client.h"
#include "cloud/wifi_service.h"
#include "CryptoAuth_init.h"
#include "../mcc_generated_files/sensors_handling.h"
#include "credentials_storage/credentials_storage.h"
#include "led.h"
#include "debug_print.h"
#include "time_service.h"
#if CFG_ENABLE_CLI
#include "cli/cli.h"
#endif


#include "../millis.h"
#include "../MAX30105.h"
#include "../HeartRate.h"
#include "../MMA7660.h"
#include "../feature_utils.h"
#include "../SEFR.h"

#define MAIN_DATA_TASK_INTERVAL 500L
#define HEART_RATE_TASK_INTERVAL 1L
#define ACC_DATA_TASK_INTERVAL 1L
// The debounce time is currently close to 2 Seconds.
#define SW_DEBOUNCE_INTERVAL   1460000L
#define SW0_TOGGLE_STATE	   SW0_GetValue()
#define SW1_TOGGLE_STATE	   SW1_GetValue()
#define TOGGLE_ON  1
#define TOGGLE_OFF 0
#define DEVICE_SHADOW_INIT_INTERVAL 1000L
#define UPDATE_DEVICE_SHADOW_BUFFER_TIME (2)
#define AWS_MCHP_SANDBOX_URL "a1gqt8sttiign3.iot.us-east-2.amazonaws.com"
static uint8_t toggleState = 0;

// This will contain the device ID, before we have it this dummy value is the init value which is non-0
char attDeviceID[20] = "BAAAAADD1DBAAADD1D";
char mqttSubscribeTopic[SUBSCRIBE_TOPIC_SIZE];
ATCA_STATUS retValCryptoClientSerialNumber;
static uint8_t holdCount = 0;

uint32_t MAIN_dataTask(void *payload);
uint32_t HEART_RATE_Task();
uint32_t ACC_DATA_Task();
timerStruct_t MAIN_dataTasksTimer = {MAIN_dataTask};
timerStruct_t HEART_RATE_TasksTimer = {HEART_RATE_Task};
timerStruct_t ACC_DATA_TasksTimer = {ACC_DATA_Task};

static void wifiConnectionStateChanged(uint8_t status);
static void sendToCloud(void);
static void updateDeviceShadow(void);
static void subscribeToCloud(void);
static void receivedFromCloud(uint8_t *topic, uint8_t *payload);
static void setToggleState(uint8_t passedToggleState);
static uint8_t getToggleState(void);
uint32_t initDeviceShadow(void *payload);
timerStruct_t initDeviceShadowTimer = {initDeviceShadow};
#if USE_CUSTOM_ENDPOINT_URL
void loadCustomAWSEndpoint(void);
#else
void loadDefaultAWSEndpoint(void);
#endif

//////////
#define ACC_BUF_LEN 384 // 2000ms @64Hz
#define FEATURE_SIZE 12
#define RATE_SIZE 4 //Increase this for more averaging. 4 is good.

int16_t acc_buf[ACC_BUF_LEN] = {0};
uint8_t rates[RATE_SIZE] = {0, 0, 0, 0}; //Array of heart rates
uint8_t rateSpot = 0;
uint32_t lastBeat = 0; //Time at which the last beat occurred
float beatsPerMinute = 0.0f;
int beatAvg = 0;

uint32_t HEART_RATE_Task() {
    uint32_t irValue = MAX30105_getIR();
    
    if (checkForBeat(irValue) == true) {
        uint32_t delta = millis() - lastBeat;
        lastBeat = millis();
        beatsPerMinute = 60 / (delta / 1000.0);
        if (beatsPerMinute < 255 && beatsPerMinute > 20) {
            rates[rateSpot++] = (uint8_t) beatsPerMinute; //Store this reading in the array
            rateSpot %= RATE_SIZE; //Wrap variable

            //Take average of readings
            beatAvg = 0;
            for (uint8_t x = 0; x < RATE_SIZE; x++) {
                beatAvg += rates[x];
                printf(" [%d] ", rates[x]);
            }
            beatAvg /= RATE_SIZE;
        }

    }

//    printf("IR = %lu, ", irValue);
//    printf("BPM = %d, ", (int) beatsPerMinute * 100);
//    printf("Avg BPM = %d\n", beatAvg);

    if (irValue < 50000) {
        //printf("No finger?\n");
        beatAvg = 0;
    }

    return HEART_RATE_TASK_INTERVAL;
}

uint32_t ACC_DATA_Task() {
    memmove(acc_buf, acc_buf + 3, (ACC_BUF_LEN - 3) * sizeof(int8_t));
    
    double xyz[3];
    ADXL345_getAcceleration(xyz);
    acc_buf[ACC_BUF_LEN - 3] = (uint16_t) xyz[0];
    acc_buf[ACC_BUF_LEN - 2] = (uint16_t) xyz[1];
    acc_buf[ACC_BUF_LEN - 1] = (uint16_t) xyz[2];
    
    //MMA7660_getAccXYZ(&acc_buf[ACC_BUF_LEN - 3], &acc_buf[ACC_BUF_LEN - 2], &acc_buf[ACC_BUF_LEN - 1]);
    printf("%d, %d,  %d\n", acc_buf[ACC_BUF_LEN - 3], acc_buf[ACC_BUF_LEN - 2], acc_buf[ACC_BUF_LEN - 1]);
    return ACC_DATA_TASK_INTERVAL;
}
// This will get called every 1 second only while we have a valid Cloud connection

static void sendToCloud(void) {
    int16_t ax[ACC_BUF_LEN/3], ay[ACC_BUF_LEN/3], az[ACC_BUF_LEN/3];
    uint8_t j = 0;
    for (uint16_t i = 0; i < ACC_BUF_LEN; i=i+3) {
        ax[j] = acc_buf[i];
        ay[j] = acc_buf[i+1];
        az[j] = acc_buf[i+2];
        j++;
    }
    
    float features[FEATURE_SIZE]= { 
        rms(ax, ACC_BUF_LEN/3), std_dev(ax, ACC_BUF_LEN/3), skewness(ax, ACC_BUF_LEN/3), kurtosis(ax, ACC_BUF_LEN/3),
        rms(ay, ACC_BUF_LEN/3), std_dev(ay, ACC_BUF_LEN/3), skewness(ay, ACC_BUF_LEN/3), kurtosis(ay, ACC_BUF_LEN/3),
        rms(az, ACC_BUF_LEN/3), std_dev(az, ACC_BUF_LEN/3), skewness(az, ACC_BUF_LEN/3), kurtosis(az, ACC_BUF_LEN/3),
    };
    
//    /* Normalize Features */
    float max[FEATURE_SIZE] = { 
        20.563904,  20.22484,  10.65187, 115.28641,  
        25.33499 ,  23.21588,  8.645585,  92.44442,  
        19.243101,  17.982597,  7.243396,  83.07158 
    };

    float min[FEATURE_SIZE] = {
        0.06523956,   0.04126525, -10.173604,  -1.7842116,
        0.06462117,   0.05163805,  -8.996278,  -1.838343,
        0.07216922,   0.06428464,  -8.466334,  -1.8937062
    };

    for (uint8_t i = 0; i < FEATURE_SIZE; i++) {
        printf("%d, ", features[i]*100);
        features[i] = (features[i]*9.81f/10.0 - min[i]) / (max[i] - min[i]);  
    }
    printf("\n");

    const int32_t predicted_class = predict(features);
    printf("predicted_class=%d\n", predicted_class);
    
    static char json[PAYLOAD_SIZE];
    static char publishMqttTopic[PUBLISH_TOPIC_SIZE];
    //int rawTemperature = 0;
    int light = 0;
    
    int len = 0;
    memset((void*) publishMqttTopic, 0, sizeof (publishMqttTopic));
    sprintf(publishMqttTopic, "%s/sensors", cid);
    // This part runs every CFG_SEND_INTERVAL seconds
    if (shared_networking_params.haveAPConnection) {
        //rawTemperature = SENSORS_getTempValue();
        light = SENSORS_getLightValue();

        len = sprintf(
            json, 
            "{\"Predicted Class\":%c,\"Average BPM\": %d}", 
            1, //predicted_class,
            beatAvg
        );


        //len = sprintf(json,"{\"Light\":%d,\"Temp\":%d.%02d}", light,rawTemperature/100,abs(rawTemperature)%100);
    }
    if (len > 0) {
        CLOUD_publishData((uint8_t*) publishMqttTopic, (uint8_t*) json, len);
        if (holdCount) {
            holdCount--;
        } else {
            ledParameterYellow.onTime = LED_BLIP;
            ledParameterYellow.offTime = LED_BLIP;
            LED_control(&ledParameterYellow);
        }
    }
}

//This handles messages published from the MQTT server when subscribed

static void receivedFromCloud(uint8_t *topic, uint8_t *payload) {
    char *toggleToken = "\"toggle\":";
    char *subString;
    sprintf(mqttSubscribeTopic, "$aws/things/%s/shadow/update/delta", cid);
    if (strncmp((void*) mqttSubscribeTopic, (void*) topic, strlen(mqttSubscribeTopic)) == 0) {
        if ((subString = strstr((char*) payload, toggleToken))) {
            if (subString[strlen(toggleToken)] == '1') {
                setToggleState(TOGGLE_ON);
                ledParameterYellow.onTime = SOLID_ON;
                ledParameterYellow.offTime = SOLID_OFF;
                LED_control(&ledParameterYellow);
            } else {
                setToggleState(TOGGLE_OFF);
                ledParameterYellow.onTime = SOLID_OFF;
                ledParameterYellow.offTime = SOLID_ON;
                LED_control(&ledParameterYellow);
            }
            holdCount = 2;
        }
    }
    debug_printIoTAppMsg("topic: %s", topic);
    debug_printIoTAppMsg("payload: %s", payload);
    updateDeviceShadow();
}

void application_init(void) {
    uint8_t mode = WIFI_DEFAULT;
    uint32_t sw0CurrentVal = 0;
    uint32_t sw1CurrentVal = 0;
    uint32_t i = 0;

    // Initialization of modules before interrupts are enabled
    SYSTEM_Initialize();
    LED_test();
#if CFG_ENABLE_CLI     
    CLI_init();
    CLI_setdeviceId(attDeviceID);
#endif   
    debug_init(attDeviceID);

    // Initialization of modules where the init needs interrupts to be enabled
    if (!CryptoAuth_Initialize()) {
        debug_printError("APP: CryptoAuthInit failed");
        shared_networking_params.haveError = 1;
    }
    // Get serial number from the ECC608 chip 
    retValCryptoClientSerialNumber = CRYPTO_CLIENT_printSerialNumber(attDeviceID);
    if (retValCryptoClientSerialNumber != ATCA_SUCCESS) {
        shared_networking_params.haveError = 1;
        switch (retValCryptoClientSerialNumber) {
            case ATCA_GEN_FAIL:
                debug_printError("APP: DeviceID generation failed, unspecified error");
                break;
            case ATCA_BAD_PARAM:
                debug_printError("APP: DeviceID generation failed, bad argument");
            default:
                debug_printError("APP: DeviceID generation failed");
                break;
        }

    }
#if CFG_ENABLE_CLI   
    CLI_setdeviceId(attDeviceID);
#endif   
    debug_setPrefix(attDeviceID);
#if USE_CUSTOM_ENDPOINT_URL
    loadCustomAWSEndpoint();
#else
    loadDefaultAWSEndpoint();
#endif  
    wifi_readThingNameFromWinc();
    timeout_create(&initDeviceShadowTimer, DEVICE_SHADOW_INIT_INTERVAL);
    // Blocking debounce
    for (i = 0; i < SW_DEBOUNCE_INTERVAL; i++) {
        sw0CurrentVal += SW0_TOGGLE_STATE;
        sw1CurrentVal += SW1_TOGGLE_STATE;
    }
    if (sw0CurrentVal < (SW_DEBOUNCE_INTERVAL / 2)) {
        if (sw1CurrentVal < (SW_DEBOUNCE_INTERVAL / 2)) {
            // Default Credentials + Connect to AP
            strcpy(ssid, CFG_MAIN_WLAN_SSID);
            strcpy(pass, CFG_MAIN_WLAN_PSK);
            sprintf((char*) authType, "%d", CFG_MAIN_WLAN_AUTH);

            ledParameterBlue.onTime = LED_BLINK;
            ledParameterBlue.offTime = LED_BLINK;
            LED_control(&ledParameterBlue);
            ledParameterGreen.onTime = LED_BLINK;
            ledParameterGreen.offTime = LED_BLINK;
            LED_control(&ledParameterGreen);
            ledParameterYellow.onTime = SOLID_OFF;
            ledParameterYellow.offTime = SOLID_ON;
            LED_control(&ledParameterYellow);
            ledParameterRed.onTime = SOLID_OFF;
            ledParameterRed.offTime = SOLID_ON;
            LED_control(&ledParameterRed);
            shared_networking_params.amConnectingAP = 1;
            shared_networking_params.amSoftAP = 0;
            shared_networking_params.amDefaultCred = 1;
        } else {
            // Host as SOFT AP
            ledParameterBlue.onTime = LED_BLIP;
            ledParameterBlue.offTime = LED_BLIP;
            LED_control(&ledParameterBlue);
            ledParameterGreen.onTime = SOLID_OFF;
            ledParameterGreen.offTime = SOLID_ON;
            LED_control(&ledParameterGreen);
            ledParameterYellow.onTime = SOLID_OFF;
            ledParameterYellow.offTime = SOLID_ON;
            LED_control(&ledParameterYellow);
            ledParameterRed.onTime = SOLID_OFF;
            ledParameterRed.offTime = SOLID_ON;
            LED_control(&ledParameterRed);
            mode = WIFI_SOFT_AP;
            shared_networking_params.amConnectingAP = 0;
            shared_networking_params.amSoftAP = 1;
            shared_networking_params.amDefaultCred = 0;
        }
    } else {
        // Connect to AP
        ledParameterBlue.onTime = LED_BLINK;
        ledParameterBlue.offTime = LED_BLINK;
        LED_control(&ledParameterBlue);
        ledParameterGreen.onTime = SOLID_OFF;
        ledParameterGreen.offTime = SOLID_ON;
        LED_control(&ledParameterGreen);
        ledParameterYellow.onTime = SOLID_OFF;
        ledParameterYellow.offTime = SOLID_ON;
        LED_control(&ledParameterYellow);
        ledParameterRed.onTime = SOLID_OFF;
        ledParameterRed.offTime = SOLID_ON;
        LED_control(&ledParameterRed);
        shared_networking_params.amConnectingAP = 1;
        shared_networking_params.amSoftAP = 0;
        shared_networking_params.amDefaultCred = 0;
    }
    wifi_init(wifiConnectionStateChanged, mode);

    if (mode == WIFI_DEFAULT) {
        CLOUD_setupTask(attDeviceID);
        timeout_create(&MAIN_dataTasksTimer, MAIN_DATA_TASK_INTERVAL);
    }

    LED_test();
    subscribeToCloud();

    initTimer();
    sei();
    printf("Timer init [OK]\n");

//    MMA7660_init();
      ADXL345_init();
      printf("Accelerometer init [OK]\n");

//    if (!MAX30105_begin()) {
//        printf("MAX30105 was not found. Please check wiring/power\n.");
//        while (1);
//    }
//
//    printf("Heart Rate sensor init [OK]\n");
//
//    MAX30105_setup(
//        0x1F, // powerLevel
//        4, // sampleAverage
//        3, // ledMode
//        400, // sampleRate
//        411, // pulseWidth
//        4096 // adcRange
//    );
//    MAX30105_setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
//    MAX30105_setPulseAmplitudeGreen(0); //Turn off Green LED
//    printf("setup done MAX30105\n");
//    
//    timeout_create(&HEART_RATE_TasksTimer, HEART_RATE_TASK_INTERVAL);
      timeout_create(&ACC_DATA_TasksTimer, ACC_DATA_TASK_INTERVAL);
}

static void subscribeToCloud(void) {
    sprintf(mqttSubscribeTopic, "$aws/things/%s/shadow/update/delta", cid);
    CLOUD_registerSubscription((uint8_t*) mqttSubscribeTopic, receivedFromCloud);
}

static void setToggleState(uint8_t passedToggleState) {
    toggleState = passedToggleState;
}

static uint8_t getToggleState(void) {
    return toggleState;
}

uint32_t initDeviceShadow(void *payload) {
    static uint32_t previousTime = 0;
    if (CLOUD_checkIsConnected()) {
        // Get the current time. This uses the C standard library time functions
        uint32_t timeNow = TIME_getCurrent();
        if (previousTime == 0) {
            previousTime = timeNow;
        } else if ((TIME_getDiffTime(timeNow, previousTime)) >= UPDATE_DEVICE_SHADOW_BUFFER_TIME) {
            updateDeviceShadow();
            return 0;
        }
    }
    return DEVICE_SHADOW_INIT_INTERVAL;
}

static void updateDeviceShadow(void) {
    static char payload[PAYLOAD_SIZE];
    static char topic[PUBLISH_TOPIC_SIZE];
    int payloadLength = 0;

    memset((void*) topic, 0, sizeof (topic));
    sprintf(topic, "$aws/things/%s/shadow/update", cid);
    if (shared_networking_params.haveAPConnection) {
        payloadLength = sprintf(payload, "{\"state\":{\"reported\":{\"toggle\":%d}}}", getToggleState());
    }
    if (payloadLength > 0) {
        CLOUD_publishData((uint8_t*) topic, (uint8_t*) payload, payloadLength);
    }
}

#if USE_CUSTOM_ENDPOINT_URL

void loadCustomAWSEndpoint(void) {
    memset(awsEndpoint, '\0', AWS_ENDPOINT_LEN);
    sprintf(awsEndpoint, "%s", CFG_MQTT_HOSTURL);
    debug_printIoTAppMsg("Custom AWS Endpoint is used : %s", awsEndpoint);
}
#else

void loadDefaultAWSEndpoint(void) {
    memset(awsEndpoint, '\0', AWS_ENDPOINT_LEN);
    wifi_readAWSEndpointFromWinc();
    if (awsEndpoint[0] == 0xFF) {
        sprintf(awsEndpoint, "%s", AWS_MCHP_SANDBOX_URL);
        debug_printIoTAppMsg("Using the AWS Sandbox endpoint : %s", awsEndpoint);
    }
}
#endif

// This scheduler will check all tasks and timers that are due and service them

void runScheduler(void) {
    timeout_callNextCallback();
}

// This gets called by the scheduler approximately every 100ms

uint32_t MAIN_dataTask(void *payload) {
    static uint32_t previousTransmissionTime = 0;

    // Get the current time. This uses the C standard library time functions
    uint32_t timeNow = TIME_getCurrent();

    // Example of how to send data when MQTT is connected every 1 second based on the system clock
    if (CLOUD_checkIsConnected()) {
        // How many seconds since the last time this loop ran?
        int32_t delta = TIME_getDiffTime(timeNow, previousTransmissionTime);

        if (delta >= CFG_SEND_INTERVAL) {
            previousTransmissionTime = timeNow;
            // Call the data task in main.c
            sendToCloud();
        }
    } else {
        ledParameterYellow.onTime = SOLID_OFF;
        ledParameterYellow.offTime = SOLID_ON;
        LED_control(&ledParameterYellow);
    }

    // Blue LED
    if (!shared_networking_params.amConnectingAP) {
        if (shared_networking_params.haveAPConnection) {
            ledParameterBlue.onTime = SOLID_ON;
            ledParameterBlue.offTime = SOLID_OFF;
            LED_control(&ledParameterBlue);
        }

        // Green LED if we are in Access Point
        if (!shared_networking_params.amConnectingSocket) {
            if (CLOUD_checkIsConnected()) {
                ledParameterGreen.onTime = SOLID_ON;
                ledParameterGreen.offTime = SOLID_OFF;
                LED_control(&ledParameterGreen);
            } else if (shared_networking_params.haveDataConnection == 1) {
                ledParameterGreen.onTime = LED_BLINK;
                ledParameterGreen.offTime = LED_BLINK;
                LED_control(&ledParameterGreen);
            }
        }
    }

    // RED LED
    if (shared_networking_params.haveError) {
        ledParameterRed.onTime = SOLID_ON;
        ledParameterRed.offTime = SOLID_OFF;
        LED_control(&ledParameterRed);
    } else {
        ledParameterRed.onTime = SOLID_OFF;
        ledParameterRed.offTime = SOLID_ON;
        LED_control(&ledParameterRed);
    }

    // This is milliseconds managed by the RTC and the scheduler, this return 
    // makes the timer run another time, returning 0 will make it stop
    return MAIN_DATA_TASK_INTERVAL;
}

void application_post_provisioning(void) {
    CLOUD_setupTask(attDeviceID);
    timeout_create(&MAIN_dataTasksTimer, MAIN_DATA_TASK_INTERVAL);
}

// React to the WIFI state change here. Status of 1 means connected, Status of 0 means disconnected

static void wifiConnectionStateChanged(uint8_t status) {
    // If we have no AP access we want to retry
    if (status != 1) {
        CLOUD_reset();
    }
}
