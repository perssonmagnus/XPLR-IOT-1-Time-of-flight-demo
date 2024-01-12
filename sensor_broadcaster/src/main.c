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
 * @brief This file implements a LightRanger9 (TMF8828) measurements broadcaster. 
 * 
 * Hardware Setup: Hardware that can be used for this implementation
 * is e a MINI-NORA-B1 Evaluation kit with a LightRanger9 (TMF8828) clickboard
 * attached to it.
 * 
 * The firmware sets up and then reads the sensor and broadcasts its
 * measurements via Bluetooth LE advertisement data.
 */

#include <kernel.h>
#include <stdio.h>
#include <drivers/sensor.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include "lightranger9.h"
#include "../bluetooth_brodcaster/bt_broadcaster.h"

/* ----------------------------------------------------------------
 * APPLICATION CONFIGURATION
 * -------------------------------------------------------------- */

/**
 * @brief Change this flag to 1 if you wish toi get
 * printing functionality for measurement data
 */
#define ENABLE_MEASUREMENT_DATA_PRINTING	1

/* ----------------------------------------------------------------
 * ZEPHYR RELATED DEFINITIONS/DECLARATIONS
 * -------------------------------------------------------------- */

#if !DT_HAS_COMPAT_STATUS_OKAY(mikroe_lightranger9)
    #error "No mikroe,lightranger9 compatible node found in the device tree"
#endif

/* ----------------------------------------------------------------
 * GLOBALS
 * -------------------------------------------------------------- */

/**
 * Measurement data
 */
static lightranger9_measurement_t bt_data;

/**
 * Simple ready flag.
 * Shows if data was parsed and is ready to be sent via BT.
 */
bool ready_flag;

/* ----------------------------------------------------------------
 * FUNCTION
 * -------------------------------------------------------------- */

#if 1 == ENABLE_MEASUREMENT_DATA_PRINTING
/**
 * @brief Print measurement
 * 
 * @param measurement  measurement data
 */
void static print_measurement(lightranger9_measurement_t *measurement);
#endif

void main( void )
{
    // Get sensor device
    const struct device *tmf  = DEVICE_DT_GET_ANY(mikroe_lightranger9);
    lightranger9_meas_cpt_t sens_data = {0};
    int ret;
    ready_flag = false;

    printk("Time Of Flight - Sensor Broadcaster Version: 1.0 \r\n\r\n");

    // check if device has been initialized properly by zephyr
    if ( !device_is_ready( tmf ) ) {
        printk( "Device %s is not ready", tmf->name );
        return;
    }

    /**
     * Enable Bluetooth functionality
     */
    ret = bt_enable(NULL);
    if (ret) {
        printk( "Could not enable Bluetooth error code (%d)", ret);
        return;
    }

    ret = bt_broadcaster_create();

    printk("Waiting for sensor measurements...\n");

    /**
     * Get sensor measurements
     */
    while ( true ) {
        /**
         * Wait for interrupt to go down.
         * Then a capture is ready.
         */
        while (lightranger9_get_interrupt_pin(tmf));

        if ( sensor_sample_fetch( tmf ) ) {
            printk( "Failed to fetch sample from LightRanger9!" );
            return;
        }

        /**
         * Copy sensor data to local structure
         * Usually the way this is done on Zephyr drivers is by 
         * calling lightranger9_channel_get() with the appropriate sensor enum which
         * returns a sensor_value type struct, containing the integer and decimal value
         * Due to the nature of the data from the ToF sensor (measurements are stored on an array)
         * we get the data from an indirect manner
         */
        lightranger9_get_measurements(tmf, &sens_data);
        ready_flag =  lightranger9_parse_measurement(&sens_data, &bt_data);
        memset(&sens_data, 0, sizeof(sens_data));

        /**
         * Broadcast a parsed measurement
         */
        if (ready_flag) {
            printk("Got new sensor measurement! Broadcasting...\n");
#if 1 == ENABLE_MEASUREMENT_DATA_PRINTING
            print_measurement(&bt_data);
#endif
            bt_broadcaster_send_message((uint8_t*)(&bt_data), sizeof(bt_data));
            memset(&bt_data, 0, sizeof(bt_data));
            ready_flag = false;
        }
    }
}


#if 1 == ENABLE_MEASUREMENT_DATA_PRINTING
void static print_measurement(lightranger9_measurement_t *measurement)
{
    uint8_t idx;

    printk("Result number: %d\n", measurement->result_number);
    printk("Die temperature: %d\n", measurement->temperature);
    printk("Valid results: %d\n", measurement->valid_results);
    printk("Ambient light: %d\n", measurement->ambient_light);
    printk("Photon count: %d\n", measurement->photon_count);
    printk("Reference count: %d\n", measurement->reference_count);
    printk("Systick: %.2f\n", measurement->sys_tick_sec);

    printk("\nObject Map 1");
    for (idx = 0; idx < LIGHTRANGER9_OBJECT_MAP_SIZE; idx ++) {
        if ((idx % 8) == 0) {
            printk("\n");
        }
        printk("%8d", measurement->obj1[idx].distance_mm);
    }

    printk("\n\nObject Map 2");
    for (idx = 0; idx < LIGHTRANGER9_OBJECT_MAP_SIZE; idx ++) {
        if ((idx % 8) == 0) {
            printk("\n");
        }
        printk("%8d", measurement->obj2[idx].distance_mm);
    }

    printk("\n\n\n\n");
}
#endif