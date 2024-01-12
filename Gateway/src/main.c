/*
 * Copyright 2023 u-blox Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
    http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


/** @file
 * @brief This application works together with the sensor_broadcaster application.
 * sensor_broadcaster application broadcasts the measurements of a LightRanger9 sensor
 * via Bluetooth advertisements.
 * 
 * Required: One XPLR-IOT-1 device. Account to Thingstream with an "MQTT now" thing enabled.
 * 
 * This application:
 *  - Connects to Wi-Fi via NINA-W156
 *  - Connects to Thingstream via MQTT
 *  - Scans for Bluetooth LE devices
 *  - Finds amongst scanned devices the one named the same as in sensor_broadcaster
 *  - Gets the address of that device and then parses the data from this device only.
 *  - Each measurement will be broadcasted several times from sensor_broadcaster. This
 *    application recognizes the measurement id and part of each measurement and if already
 *    read it ignores it, until the next measurement comes (different measurement part or id)
 * -  Each time a new measurement is received, a JSON message is prepared and sent 
 *    to Thingstream (you can then see those measurements in the Dashboard that comes
 *    with this example)
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>

#include "ubxlib.h"
#include "nina_config.h"

/* ----------------------------------------------------------------
 * APPLICATION DEFINITIONS
 * -------------------------------------------------------------- */

#define MEAS_DIST_RES_BUFF_LEN  (2560U)
#define MEAS_HEADER_BUFF_LEN     (512U)
#define MEAS_COMPLETE_BUFF_LEN  (MEAS_DIST_RES_BUFF_LEN + MEAS_HEADER_BUFF_LEN)

#define LIGHTRANGER9_OBJECT_MAP_SIZE    (64U)

// Cradentials of the Wi-FI network the user wants to connect to
#define WIFI_SSID           "your_ssid"
#define WIFI_PASSWORD       "your_password"

// Topic name where the received measurements are going to be published
// Should be defined to Thingstream as well
#define MQTT_TOPIC          "timeofflight"

// MQTT broked credentials
#define MQTT_BROKER_NAME    "mqtt.thingstream.io"
#define MQTT_PORT           1883  
#define MQTT_DEVICE_ID      "device:xxxx-xxxxx-xxx-xxx"
#define MQTT_USERNAME       "Paste and copy IP thing username here"
#define MQTT_PASSWORD       "Paste and copy IP thing password here"

// The name of the broadcaster (under which name the broadcaster advertises)
#define BROADCASTER_NAME    "LIGHTR9"

#define BT_AD_EFFECTIVE_PAYLOAD 210
#define BT_ADD_MAIN_BUFF_CHUNK	206

/* ----------------------------------------------------------------
 * GLOBALS
 * -------------------------------------------------------------- */

/**
 * @brief Line formatter for json
 * 
 */
const char format_str[] = "{\"resno\":%d,\"temp\":%d,\"valres\":%d,\"ambli\":%d,\"phocnt\":%d,\"refcnt\":%d,\"syst\":%.2f,\"res\":%s}";

/**
 * @brief Distance measurements struct
 */
typedef struct lightranger9_meas_result_type {
    uint8_t confidence;
    uint16_t distance_mm;
} lightranger9_meas_result_t;

typedef struct __attribute__((__packed__)) bt_data_header_type {
    uint8_t id[2];
    uint8_t part_no;
    uint8_t parts_total;
} bt_data_header_t;

/** Structure holding the measurements of the LightRanger9 (TMF8828) sensor .
 * Note: This struct declaration should be the same as the one used by the
 * broadcaster (check lightranger9.h)
 */
typedef struct __attribute__((__packed__)) lightranger9_btdata_type {
    uint8_t result_number;
    int8_t temperature;
    uint8_t valid_results;
    uint32_t ambient_light;
    uint32_t photon_count;
    uint32_t reference_count;
    float sys_tick_sec;
    lightranger9_meas_result_t obj1[LIGHTRANGER9_OBJECT_MAP_SIZE];
    lightranger9_meas_result_t obj2[LIGHTRANGER9_OBJECT_MAP_SIZE];
} lightranger9_btdata_t;

static bt_data_header_t header;
static bt_data_header_t prev_header;
static uint8_t buffer[sizeof(lightranger9_btdata_t)];
lightranger9_btdata_t gMeasurement = {0};

/**
 * Helper buffer for distance and confidence
 */
char dist_res[MEAS_DIST_RES_BUFF_LEN];

/** Message to be pubished via MQTT*/
char gMessageToPublish[MEAS_COMPLETE_BUFF_LEN] = { 0 };

/** Has the address of the Sensor broadcaster been obtained? */
static bool gAddressObtained = false;

/** The address of the broadcaster */
bt_addr_t gAddress;

/** The last measurement(message) ID obtained*/
uint32_t gLastMeasId = 0;

/** When a new measurement has been received */
static bool gMeasReceived = false;


/* ----------------------------------------------------------------
 * MACROS
 * -------------------------------------------------------------- */

/** Macro to verify that a condition is met. If the condition is met,
 *  nothing happens. If the condition in not met the failed function is
 *  called. 
 *  (In this implementation of the failed function the fail_msg is typed and
 *  the application halts via an infinite loop) */
#define VERIFY(cond, fail_msg) \
    if (!(cond)) {\
        failed(fail_msg); \
    }

/* ----------------------------------------------------------------
 * STATIC FUNCTION DECLARATION
 * -------------------------------------------------------------- */

/** 
 * @brief The scan callback to be executed when a new device is found be the BLE scanner
 * 
 * @param addr       See bt_le_scan_cb_t description.
 * @param rssi       See bt_le_scan_cb_t description.
 * @param adv_type   See bt_le_scan_cb_t description.
 * @param buf        See bt_le_scan_cb_t description.
 */
static void scan_cb(const bt_addr_le_t *addr,
                    int8_t rssi,
                    uint8_t adv_type,
                    struct net_buf_simple *buf);


/** 
 * @brief Callback to be executed when the device disconnects from MQTT broker.
 *
 * @param adv_type  See uMqttClientSetDisconnectCallback description.
 * @param buf       See uMqttClientSetDisconnectCallback description.
*/
static void mqttDisconnectCb(int32_t errorCode, void *pParam);


/** Function to be called when something fails. Halts execution. 
 * 
 * @param msg       Message to be typed before halt.
 */
static void failed(const char *msg);


/** 
 *  @brief To be used as a parameter of bt_data_parse() within the scan callback.
 *  Checks if the scanned device name is the one requested by
 *  BROADCASTER_NAME definition
 * 
 *  @param data       see bt_data_parse() description.
 *  @param user_data  see bt_data_parse() description. Returns true if 
 *                    requested device name is found. 
 *  @return           see bt_data_parse() description.
 */
static bool adv_check_name(struct bt_data *data, void *name_found);


/** 
 *  @brief To be used as a parameter of bt_data_parse() within the scan callback.
 *  Checks the advertising packet type and the data within it.
 *  If a NEW measurement has been received, it populates the gMeasurement struct
 *  and sets the gMeasReceived boolean to true.
 * 
 *  @param data       see bt_data_parse() description.
 *  @param user_data  see bt_data_parse() description.
 *  @return           see bt_data_parse() description.
 */
static bool adv_data_found(struct bt_data *data, void *user_data);


/**
 * @brief Used to convert the distance measurement part
 * to a JSON string to be used in mqttMeasurementToJson()
 * 
 * @param meas     measurements struct.
 * @param json     JSON payload.
 * @param max_len  maximum length of JSON buffer.
 * @return         true on success otherwise false
 */
static bool mqttMeasurementToJsonHelper(lightranger9_btdata_t *meas, char *json, uint16_t max_len);


/**
 * @brief Used to convert the whole measurements struct from LightRanger9
 * to a JSON payload for MQTT transmission.
 * Uses mqttMeasurementToJsonHelper()
 * 
 * @param meas     measurements struct.
 * @param json     JSON payload.
 * @param max_len  maximum length of JSON buffer.
 * @return         true on success otherwise false
 */
static bool mqttMeasurementToJson(lightranger9_btdata_t *meas, char *json, uint16_t max_len);


/** 
 * Function description goes here.
 *
 * @param param1  param1 desc.
 * @param param2  param2 desc.
 * @return        zero on success else negative error code.
 */


/* ----------------------------------------------------------------
 * STATIC FUNCTION IMPLEMENTATION
 * -------------------------------------------------------------- */

static void failed(const char *msg)
{
    printk( "%s", msg );
    while( 1 );
}


static bool adv_check_name(struct bt_data *data, void *name_found)
{
    bool found = *((bool *)name_found);
    bool ret = false;

    if (!found) {
        if (data->type == BT_DATA_MANUFACTURER_DATA) {
            /**
             * If we got BT_DATA_MANUFACTURER_DATA then 
             * there must be a device name at the end of the received data.
             * Return true so the calling data parser function can go to the next
             * section --> BT_DATA_NAME_COMPLETE (0x09)
             */
            ret = true;
        } else if (data->type == BT_DATA_NAME_COMPLETE) {
            if (memcmp(data->data, BROADCASTER_NAME, strlen(BROADCASTER_NAME)) == 0) {
                // name found, set name_found to true
                memset(name_found, 1, 1);
            }
            ret = false;
        } else {
            ret = false;
        }
    } else {
        ret = false;
    }

    return ret;
}


static bool adv_data_found(struct bt_data *data, void *user_data)
{
    static uint8_t cnt = 0;
    /**
     * check advertisement's AD type byte.
     * BT_DATA_MANUFACTURER_DATA contains measurement data
     * we are only interested in that
     */
    if( data->type == BT_DATA_MANUFACTURER_DATA ) {
        /**
         * Get current header
         * A header contains 4 bytes
         * First 2 bytes are a random id [populated by srand in the broadcaster]
         * 3rd byte is current part of total
         * 4th byte is total parts excpected
         */
        memcpy(&header, data->data, sizeof(header));

        /**
         * If current id is different from previous id
         * Reset the buffer to 0
         * Copy current measurements payload/chunk to buffer
         */
        if (memcmp(&prev_header, &header, sizeof(header.id)) != 0) {
            memset(buffer, 0, sizeof(buffer));
            memcpy(buffer + ((header.part_no - 1) * BT_ADD_MAIN_BUFF_CHUNK),
                   data->data + sizeof(header),
                   data->data_len - sizeof(header));
            cnt = 1;
            memcpy(&prev_header, &header, sizeof(header));
        } else {
            /**
             * If the header is the same check which part it is
             */
            if (header.part_no != prev_header.part_no) {
                memcpy(buffer + ((header.part_no - 1) * BT_ADD_MAIN_BUFF_CHUNK),
                       data->data + sizeof(header),
                       data->data_len - sizeof(header));
                cnt++;
                memcpy(&prev_header, &header, sizeof(header));

                if (cnt == header.parts_total) {
                    memcpy(&gMeasurement, buffer, sizeof(gMeasurement));
                    gMeasReceived = true;
                    cnt = 0;
                    memset(&prev_header, 0, sizeof(prev_header));
                    memset(&header, 0, sizeof(header));
                }
            } else {
                // do nothing
            }
        }
    } else {
        //do nothing
    }

    return false;
}

static void scan_cb(const bt_addr_le_t *addr,
                    int8_t rssi,
                    uint8_t adv_type,
                    struct net_buf_simple *buf)
{
    char ble_addr[BT_ADDR_LE_STR_LEN] = { 0 };
    int memcpy_ret;

    // search for Broadcaster device name and obtain its address
    if (!gAddressObtained) {
        //parse advertisement packet and search for device name
        bool name_found = false;
        bt_data_parse(buf, adv_check_name, &name_found);
        
        if (name_found) {
            printf( "Found Broadcaster Name." );
            //save address
            gAddress = addr->a;
            gAddressObtained = true;

            /**
             * Convert address to string and print
             */
            bt_addr_le_to_str(addr, ble_addr, sizeof(ble_addr));
            printk("Address: %s\r\n", ble_addr);
        } else {
            // do nothing
        }
    } else {
        /**
         * if the scanned device has the expected address
         */
        memcpy_ret = memcmp(addr->a.val, gAddress.val, sizeof(gAddress.val) );
        if (memcpy_ret == 0) {
            // parse data to get measurement
            bt_data_parse(buf, adv_data_found, NULL);
        } else {
            // do nothing
        }
    }   
}

static void mqttDisconnectCb(int32_t errorCode, void *pParam)
{
    printk("MQTT Disconnected! \r\n");
}

static bool mqttMeasurementToJsonHelper(lightranger9_btdata_t *meas, char *json, uint16_t max_len)
{
    uint8_t cnt;
    uint16_t current_len = 0;
    bool ret;

    /**
     * Convert measurements to json array
     */
    current_len = snprintk(json,
                           max_len - current_len,
                           "{\"map1\":[");
    for (cnt = 0; cnt < LIGHTRANGER9_OBJECT_MAP_SIZE * 2; cnt++) {
        /**
         * Measurements from 1st Object Map
         */
        if (cnt < (LIGHTRANGER9_OBJECT_MAP_SIZE)) {
            current_len += snprintk(json + current_len,
                                    max_len - current_len,
                                    "%d,",
                                    meas->obj1[cnt].distance_mm);
        } else {
            // do nothing
        }

        /**
         * Terminate first array
         * Start 2nd array
         */
        if (cnt == LIGHTRANGER9_OBJECT_MAP_SIZE) {
            current_len --;
            current_len += snprintk(json + current_len,
                                    max_len - current_len,
                                    "],\"map2\":[");
        } else {
            // do nothing
        }

        /**
         * Measurements from 2nd Object Map
         */
        if (cnt >= LIGHTRANGER9_OBJECT_MAP_SIZE) {
            current_len += snprintk(json + current_len,
                                    max_len - current_len,
                                    "%d,",
                                    meas->obj2[cnt - LIGHTRANGER9_OBJECT_MAP_SIZE].distance_mm);
        } else {
            // do nothing
        }
    }
    /**
     * Remove trailing comma by overwriting it 
     * with a JSON bracket array terminator
     */
    current_len --;
    current_len += snprintk(json + current_len,
                            max_len - current_len,
                            "]}");

    if (current_len >= max_len) {
        ret = false;
    } else {
        ret = true;
    }

    return ret;
}

static bool mqttMeasurementToJson(lightranger9_btdata_t *meas, char *json, uint16_t max_len)
{
    int print_ret;
    bool ret;
    
    /**
     * Reset buffers to 0
     */
    memset(dist_res, 0, sizeof(dist_res));
    memset(json, 0, max_len);

    ret = mqttMeasurementToJsonHelper(meas, dist_res, sizeof(dist_res));
    if (ret) {
        /**
         * Converting measurements struct to a JSON payload
         */
        print_ret = snprintk(json, 
                             max_len,
                             format_str,
                             meas->result_number,
                             meas->temperature,
                             meas->valid_results,
                             meas->ambient_light,
                             meas->photon_count,
                             meas->reference_count,
                             meas->sys_tick_sec,
                             dist_res);

        /**
         * Check that we printed everything in the reply string
         */
        if (print_ret == 0) {
            printk("Measurement to string failed. 0 bytes written to string.");
            ret = false;
        } else if (print_ret >= max_len) {
            printk("Measurement to string failed. buffer not large enough ret [%d] > max_len [%d]!",
                   print_ret,
                   max_len);
            ret = false;
        } else {
            ret = true;
        }
    } else {
        // do nothing
    }

    return ret;
}

/* ----------------------------------------------------------------
 * MAIN APPLICATION IMPLEMENTATION
 * -------------------------------------------------------------- */

void main(void)
{
    int mqtt_ret;
    uMqttClientContext_t *mqttClientCtx; 
    static uDeviceHandle_t gDevHandle = NULL;


    // Wi-Fi module config for use by ubxlib
    static const uDeviceCfg_t deviceCfg = {
        .deviceType = U_DEVICE_TYPE_SHORT_RANGE,
        .deviceCfg = {
            .cfgSho = {
                .moduleType = U_SHORT_RANGE_MODULE_TYPE_NINA_W15
            }
        },
        .transportType = U_DEVICE_TRANSPORT_TYPE_UART,
        .transportCfg = {
            .cfgUart = {
                .uart = 2,     //as defined in board overlay file
                .baudRate = 115200,
                .pinTxd = -1,  //defined in board overlay file
                .pinRxd = -1,  //defined in board overlay file
                .pinCts = -1,  //defined in board overlay file
                .pinRts = -1   //defined in board overlay file
            }
        }
    };

    // Connection to Wifi network configuration- if Network requires password
    static const uNetworkCfgWifi_t  wifiConfig = {
        .type = U_NETWORK_TYPE_WIFI,
        .pSsid = WIFI_SSID,
        .authentication = 2, 
        .pPassPhrase = WIFI_PASSWORD
    };

    // MQTT Configuration parameters
    const uMqttClientConnection_t mqttConnection = {
            .pBrokerNameStr = MQTT_BROKER_NAME,
            .localPort = MQTT_PORT,  
            .pClientIdStr = MQTT_DEVICE_ID,
            .pUserNameStr = MQTT_USERNAME,
            .pPasswordStr = MQTT_PASSWORD
    };

    // BLE scanning parameters
    struct bt_le_scan_param scan_param = {
            .type       = BT_HCI_LE_SCAN_ACTIVE,
            .options    = BT_LE_SCAN_OPT_FILTER_DUPLICATE,
            .interval   = 0x0010,
            .window     = 0x0010,
    };

    // Initialize to unknown id
    memset(&prev_header, 0, sizeof(header));
    memset(&header, 0, sizeof(header));
    
    printk("Time Of Flight Gateway Version: 1.0 \r\n\r\n");

    // Initialize NINA-W156 Wi-Fi module hardware (set appropriate pins)    
    nina15InitPower();
    printk("NINA-W15 Powered on \r\n");
    ninaNoraCommEnable();

    // Set up Connection to Wi-Fi using ubxlib library (NINA-W156)
    VERIFY(uPortInit() == 0, "uPortInit failed\n");
    VERIFY(uDeviceInit() == 0, "uDeviceInit failed\n");
    VERIFY(uDeviceOpen( &deviceCfg, &gDevHandle ) == 0, "uDeviceOpen failed\n");

    uAtClientDebugSet(gDevHandle, false);

    printk("Bring up Wi-Fi\n");
    VERIFY(uNetworkInterfaceUp( gDevHandle, U_NETWORK_TYPE_WIFI, &wifiConfig ) == 0 , "Could not connect to network");
    printk("Wi-Fi connected\n");
    
    // Set up Connection to MQTT (Thingstream) using ubxlib library (NINA-W156)
    printk("Setup up MQTT\n");
    mqttClientCtx = pUMqttClientOpen( gDevHandle, NULL);
    VERIFY(mqttClientCtx != NULL, "Could not open MQTT Client");

    printk("uMqttClientConnect...");
    VERIFY(uMqttClientConnect( mqttClientCtx, &mqttConnection ) == 0, "uMqttClientConnect failed\n");
    printk("ok\n");

    VERIFY(uMqttClientSetDisconnectCallback( mqttClientCtx, mqttDisconnectCb, (void *)mqttClientCtx) == 0, "Failed to set MQTT disconnection callback \r\n");

    // Setup/Initialize BLE in NORA-B1
    printk("Starting BLE\n");
    VERIFY(bt_enable(NULL) == 0, "Bluetooth init failed\n"); 
    printk("Bluetooth initialized\n");

    // Start Scanning for BLE devices and setup callback for incoming advertising packets
    VERIFY(bt_le_scan_start(&scan_param, scan_cb) == 0, "Scanning failed to start\n");
    printk("\nWaiting for sensor advertisements\n");

    do {
        // if new measurement has been received, publish it to MQTT
        if (gMeasReceived) {
            // clear any previous message bytes
            memset(gMessageToPublish, 0, sizeof(gMessageToPublish));

            /**
             * Prepare a JSON message containing the measurements
             * and print the payload
             */
            mqttMeasurementToJson(&gMeasurement, gMessageToPublish, sizeof(gMessageToPublish));
            printk("%s\n\n", gMessageToPublish);

            /**
             * Publish the JSON message
             */
            mqtt_ret = uMqttClientPublish(mqttClientCtx,
                                          MQTT_TOPIC,
                                          gMessageToPublish,
                                          strlen(gMessageToPublish),
                                          0,
                                          0);
            if (mqtt_ret == 0) {
                printk("Published\r\n\r\n");
            } else {
                printk("Publish failed\r\n");
            }

            /**
             * Wait for next measurement
             */
            gMeasReceived = false;
        } else {
            // do nothing
        }
    } while(uMqttClientIsConnected(mqttClientCtx));

    // When disconnected from broker the application stops
    printk("Application stopped\r\n");
    bt_le_scan_stop();
    
    return;
}

// EOF //