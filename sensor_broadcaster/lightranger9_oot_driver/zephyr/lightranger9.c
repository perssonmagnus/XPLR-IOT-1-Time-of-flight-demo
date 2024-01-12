/*
 * Copyright 2023 u-blox Ltd
 * 
 * Based and ported to Zephyr from MikroE drivers available on libstock
 * https://libstock.mikroe.com/projects/view/5000/lightranger-9-click
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

/**
 * Copyright (C) 2020 MikroElektronika d.o.o.
 * Contact: https://www.mikroe.com/contact

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
 * OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#define DT_DRV_COMPAT mikroe_lightranger9

#include <stdio.h>

#include <device.h>
#include <drivers/i2c.h>
#include <kernel.h>
#include <drivers/sensor.h>
#include <sys/__assert.h>
#include <logging/log.h>
#include <drivers/gpio.h>
#include <drivers/sensor.h>
#include <string.h>
#include <init.h>
#include <hal/nrf_gpio.h>

#include "tof_bin_image.h"
#include "lightranger9.h"

LOG_MODULE_REGISTER(LIGHTRANGER9, CONFIG_SENSOR_LOG_LEVEL);

/* ----------------------------------------------------------------
 * COMPILE-TIME MACROS
 * -------------------------------------------------------------- */

#define BL_MAX_CHUNK_BYTES      128
#define BL_DOWNLOAD_INIT_SEED   0x29
#define BL_START_ADDRESS        0x0000

/* ----------------------------------------------------------------
 * TYPES
 * -------------------------------------------------------------- */

static lightranger9_meas_cpt_t lightranger9_meas_cpt;
                                                    
static const lightranger9_config_t lightranger9_config = {
    .bus = I2C_DT_SPEC_INST_GET(0),
    .control_ctrl = DEVICE_DT_GET(DT_GPIO_CTLR(DT_DRV_INST(0), control_gpios)),
    .control_pin = DT_INST_GPIO_PIN_BY_IDX(0, control_gpios, 0),
    .control_flags = DT_INST_GPIO_FLAGS_BY_IDX(0, control_gpios, 0),
    .int_pin = DT_INST_GPIO_PIN_BY_IDX(0, control_gpios, 1),
    .int_flags = DT_INST_GPIO_FLAGS_BY_IDX(0, control_gpios, 1),
    .gpio1_pin = DT_INST_GPIO_PIN_BY_IDX(0, control_gpios, 2),
    .gpio1_flags = DT_INST_GPIO_FLAGS_BY_IDX(0, control_gpios, 2)
};

/* ----------------------------------------------------------------
 * STATIC FUNCTIONS
 * -------------------------------------------------------------- */

static int lightranger9_clear_interrupts (const struct device *dev);

/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * -------------------------------------------------------------- */

void lightranger9_get_measurements(const struct device *dev, lightranger9_meas_cpt_t *sens_data)
{
    lightranger9_meas_cpt_t *data = dev->data;
    memcpy(sens_data, data, sizeof(lightranger9_meas_cpt_t));
}

void lightranger9_clear_ints(const struct device *dev)
{
    lightranger9_clear_interrupts(dev);
}

bool lightranger9_get_interrupt_pin(const struct device *dev)
{
    const lightranger9_config_t *cfg = dev->config;
    bool ret;

    ret = gpio_pin_get(cfg->control_ctrl, cfg->int_pin);

    return ret;
}

bool lightranger9_parse_measurement(lightranger9_meas_cpt_t *capture, lightranger9_measurement_t *parsed_data)
{
    bool ret;
    static uint8_t sub_capture_cnt = 0;
    uint8_t result_cnt = 0, row = 0, col = 0;

    for (result_cnt = 0; result_cnt < LIGHTRANGER9_MAX_MEAS_RESULTS; result_cnt++) {
        if (8 == (result_cnt % 9)) {
            continue;
        }
        row = (((result_cnt % 9) / 2) * 2) + (capture->sub_capture / 2);
        col = (((result_cnt % 9) % 2) * 4) + ((result_cnt % 18) / 9) + ((capture->sub_capture % 2) * 2);
        if (result_cnt >= ( LIGHTRANGER9_MAX_MEAS_RESULTS / 2)) {
            parsed_data->obj2[(row * 8) + col].distance_mm = capture->result[result_cnt].distance_mm;
            parsed_data->obj2[(row * 8) + col].confidence  = capture->result[result_cnt].confidence;
        } else {
            parsed_data->obj1[(row * 8) + col].distance_mm = capture->result[result_cnt].distance_mm;
            parsed_data->obj1[(row * 8) + col].confidence  = capture->result[result_cnt].confidence;
        }
    }

    if (sub_capture_cnt < LIGHTRANGER9_SUBCAPTURE_3) {
        sub_capture_cnt++;
        ret = false;
    } else {
        parsed_data->result_number   = capture->result_number;
        parsed_data->temperature     = capture->temperature;
        parsed_data->valid_results   = capture->valid_results;
        parsed_data->ambient_light   = capture->ambient_light;
        parsed_data->photon_count    = capture->photon_count;
        parsed_data->reference_count = capture->reference_count;
        parsed_data->sys_tick_sec    = capture->sys_tick_sec;
        
        sub_capture_cnt = 0;
        ret = true;
    }

    return ret;
}

/* ----------------------------------------------------------------
 * STATIC FUNCTIONS
 * -------------------------------------------------------------- */

static uint8_t lightranger9_calculate_checksum(uint8_t *data_in, uint8_t len)
{
    uint16_t data_sum = 0;
    uint8_t cnt;

    for (cnt = 0; cnt < len; cnt++) {
        data_sum += data_in[cnt];
    }

    return (uint8_t)((~data_sum) & 0xFF);
}

static int lightranger9_generic_write(const struct device *dev,
                                      uint8_t reg,
                                      uint8_t *data_in,
                                      uint8_t len)
{
    const lightranger9_config_t *cfg = dev->config;
    uint8_t data_buf[256] = {0};
    uint8_t cnt;
    int ret;

    data_buf[0] = reg;
    for (cnt = 0; cnt < len; cnt++ ) {
        data_buf[cnt + 1] = data_in[cnt];
    }

    ret = i2c_write_dt(&cfg->bus, data_buf, len + 1);

    return ret;
}

static int lightranger9_read_register(const struct device *dev,
                                      uint8_t reg_addr,
                                      uint8_t *rx_buf,
                                      uint8_t rx_len)
{
    const lightranger9_config_t *cfg = dev->config;
    int ret;
    uint8_t tx_buf[1];

    tx_buf[0] = reg_addr;
    ret = i2c_write_read_dt(&cfg->bus, tx_buf, 1, rx_buf, rx_len);

    return ret;
}


static int lightranger9_write_register(const struct device *dev,
                                       uint8_t reg_addr,
                                       uint8_t data)
{
    return lightranger9_generic_write(dev, reg_addr, &data, 1);
}

bool lightranger9_check_communication(const struct device *dev)
{
    uint8_t device_id = 0;
    uint8_t intRet, ret;

    intRet = lightranger9_read_register(dev, LIGHTRANGER9_REG_ID, &device_id, 1);

    if (intRet == 0) {
        if (LIGHTRANGER9_DEVICE_ID == device_id) {
            ret = true;
        } else {
            ret = false;
        }
    } else {
        ret = false;
    }

    return ret;
}

static int lightranger9_write_bl_cmd(const struct device *dev,
                                     uint8_t cmd,
                                     uint8_t *data_in,
                                     uint8_t len)
{
    uint8_t data_buf[256] = {0};
    uint8_t cnt;

    data_buf[0] = cmd;
    data_buf[1] = len;

    for (cnt = 0; cnt < len; cnt++) {
        data_buf[cnt + 2] = data_in[cnt];
    }

    data_buf[len + 2] = lightranger9_calculate_checksum(data_buf, len + 2);
    return lightranger9_generic_write(dev, LIGHTRANGER9_REG_CMD_STAT, data_buf, len + 3);
}

static int lightranger9_read_bl_cmd_status(const struct device *dev, uint8_t *status)
{
    int ret = 0;
    uint8_t data_buf[3] = {0};

    ret = lightranger9_read_register(dev,
                                     LIGHTRANGER9_REG_CMD_STAT,
                                     data_buf,
                                     sizeof(data_buf));
    if (ret == 0) {
        if ((data_buf[1]) ||
            (data_buf[2] != lightranger9_calculate_checksum(data_buf, 2))) {
            ret = -1;
        } else {
            *status = data_buf[0];
            ret = 0;
        }
    } else {
        // do nothing
    }
    
    return ret;
}

static int lightranger9_download_fw_bin(const struct device *dev)
{
    uint8_t data_buf[BL_MAX_CHUNK_BYTES] = {0};
    int error_flag       = 0;
    uint16_t timeout_cnt = 0;
    uint32_t fw_index    = 0;
    uint8_t chunk_bytes  = 0;

    error_flag |= lightranger9_read_register(dev,
                                             LIGHTRANGER9_REG_APPID,
                                             data_buf,
                                             1);

    if (LIGHTRANGER9_APP_ID_BOOTLOADER == data_buf[0]) {
        data_buf[0] = BL_DOWNLOAD_INIT_SEED;
        error_flag |= lightranger9_write_bl_cmd(dev,
                                                LIGHTRANGER9_BL_CMD_DOWNLOAD_INIT,
                                                data_buf,
                                                1);
        timeout_cnt = 0;
        do {
            k_msleep(1);
            error_flag |= lightranger9_read_bl_cmd_status (dev, data_buf);
            if (++timeout_cnt > LIGHTRANGER9_TIMEOUT) {
                return -1;
            }
        } while (LIGHTRANGER9_BL_CMD_STAT_READY != data_buf[0]);
        
        data_buf[0] = BL_START_ADDRESS & 0xFF;
        data_buf[1] = (BL_START_ADDRESS >> 8) & 0xFF;
        error_flag |= lightranger9_write_bl_cmd(dev,
                                                LIGHTRANGER9_BL_CMD_ADDR_RAM,
                                                data_buf,
                                                2);

        timeout_cnt = 0;
        do {
            k_msleep(1);
            error_flag |= lightranger9_read_bl_cmd_status(dev, data_buf);
            if (++timeout_cnt > LIGHTRANGER9_TIMEOUT) {
                return -1;
            }
        } while (LIGHTRANGER9_BL_CMD_STAT_READY != data_buf[0]);
        
        while (fw_index < sizeof(tof_bin_image)) {
            if ((fw_index + BL_MAX_CHUNK_BYTES) <= sizeof(tof_bin_image)) {
                chunk_bytes = BL_MAX_CHUNK_BYTES;
            } else {
                chunk_bytes = sizeof(tof_bin_image) - fw_index;
            }

            memcpy (data_buf, &tof_bin_image[fw_index], chunk_bytes);
            error_flag |= lightranger9_write_bl_cmd(dev, LIGHTRANGER9_BL_CMD_W_RAM, data_buf, chunk_bytes);
            fw_index += chunk_bytes;
            timeout_cnt = 0;

            do {
                k_msleep(1);
                error_flag |= lightranger9_read_bl_cmd_status(dev, data_buf);
                if (++timeout_cnt > LIGHTRANGER9_TIMEOUT) {
                    return -1;
                }
            } while (LIGHTRANGER9_BL_CMD_STAT_READY != data_buf[0]);
        }
        
        error_flag |= lightranger9_write_bl_cmd(dev, LIGHTRANGER9_BL_CMD_RAMREMAP_RESET, NULL, 0);

        timeout_cnt = 0;
        do {
            k_msleep(1);
            error_flag |= lightranger9_read_register(dev, LIGHTRANGER9_REG_APPID, data_buf, 1);
            if (++timeout_cnt > LIGHTRANGER9_TIMEOUT) {
                return -1;
            }
        } while (LIGHTRANGER9_APP_ID_MEASUREMENT != data_buf[ 0 ]);
    }

    return error_flag;
}

static int lightranger9_clear_interrupts (const struct device *dev)
{
    uint8_t int_status;
    int error_flag = lightranger9_read_register(dev, LIGHTRANGER9_REG_INT_STATUS, &int_status, 1);
    error_flag |= lightranger9_write_register(dev, LIGHTRANGER9_REG_INT_STATUS, int_status);
    return error_flag;
}

static int lightranger9_sample_fetch(const struct device *dev,
                                     enum sensor_channel chan)
{
    lightranger9_meas_cpt_t *data = dev->data;
    uint8_t data_buf[LIGHTRANGER9_BLOCKREAD_SIZE] = {0};
    uint8_t cnt;
    uint8_t ret;

    ret  = lightranger9_clear_interrupts(dev);
    ret |= lightranger9_read_register(dev,
                                      LIGHTRANGER9_REG_BLOCKREAD,
                                      data_buf,
                                      LIGHTRANGER9_BLOCKREAD_SIZE);

    if (ret == 0) {
        data->sub_capture   = data_buf[LIGHTRANGER9_REG_RESULT_NUMBER - LIGHTRANGER9_REG_BLOCKREAD] & LIGHTRANGER9_SUBCAPTURE_MASK;
        data->result_number = (data_buf[LIGHTRANGER9_REG_RESULT_NUMBER - LIGHTRANGER9_REG_BLOCKREAD] >> 2) & LIGHTRANGER9_RESULT_NUMBER_MASK;
        data->temperature   = (int8_t)data_buf[LIGHTRANGER9_REG_TEMPERATURE - LIGHTRANGER9_REG_BLOCKREAD];
        data->valid_results = data_buf[LIGHTRANGER9_REG_NUMBER_VALID_RESULTS - LIGHTRANGER9_REG_BLOCKREAD];

        data->ambient_light = 
            ((uint32_t)data_buf[LIGHTRANGER9_REG_AMBIENT_LIGHT_3 - LIGHTRANGER9_REG_BLOCKREAD] << 24) | 
            ((uint32_t)data_buf[LIGHTRANGER9_REG_AMBIENT_LIGHT_2 - LIGHTRANGER9_REG_BLOCKREAD] << 16) | 
            ((uint16_t)data_buf[LIGHTRANGER9_REG_AMBIENT_LIGHT_1 - LIGHTRANGER9_REG_BLOCKREAD] <<  8) | 
            data_buf[LIGHTRANGER9_REG_AMBIENT_LIGHT_0 - LIGHTRANGER9_REG_BLOCKREAD];

        data->photon_count = 
            ((uint32_t)data_buf[LIGHTRANGER9_REG_PHOTON_COUNT_3 - LIGHTRANGER9_REG_BLOCKREAD] << 24) | 
            ((uint32_t)data_buf[LIGHTRANGER9_REG_PHOTON_COUNT_2 - LIGHTRANGER9_REG_BLOCKREAD] << 16) | 
            ((uint16_t)data_buf[LIGHTRANGER9_REG_PHOTON_COUNT_1 - LIGHTRANGER9_REG_BLOCKREAD] <<  8) | 
            data_buf[LIGHTRANGER9_REG_PHOTON_COUNT_0 - LIGHTRANGER9_REG_BLOCKREAD];

        data->reference_count = 
            (( uint32_t)data_buf[LIGHTRANGER9_REG_REFERENCE_COUNT_3 - LIGHTRANGER9_REG_BLOCKREAD] << 24) | 
            (( uint32_t)data_buf[LIGHTRANGER9_REG_REFERENCE_COUNT_2 - LIGHTRANGER9_REG_BLOCKREAD] << 16) | 
            (( uint16_t)data_buf[LIGHTRANGER9_REG_REFERENCE_COUNT_1 - LIGHTRANGER9_REG_BLOCKREAD] <<  8) | 
            data_buf[LIGHTRANGER9_REG_REFERENCE_COUNT_0 - LIGHTRANGER9_REG_BLOCKREAD];

        data->sys_tick_sec = 
            ((( uint32_t)data_buf[LIGHTRANGER9_REG_SYS_TICK_3 - LIGHTRANGER9_REG_BLOCKREAD] << 24) | 
             (( uint32_t)data_buf[LIGHTRANGER9_REG_SYS_TICK_2 - LIGHTRANGER9_REG_BLOCKREAD] << 16) | 
             (( uint16_t)data_buf[LIGHTRANGER9_REG_SYS_TICK_1 - LIGHTRANGER9_REG_BLOCKREAD] <<  8) | 
             data_buf[LIGHTRANGER9_REG_SYS_TICK_0 - LIGHTRANGER9_REG_BLOCKREAD ]) * LIGHTRANGER9_SYS_TICK_TO_SEC;

        for (cnt = 0; cnt < LIGHTRANGER9_MAX_MEAS_RESULTS; cnt++) {
            data->result[cnt].confidence = data_buf[LIGHTRANGER9_REG_RES_CONFIDENCE_0 - LIGHTRANGER9_REG_BLOCKREAD + (cnt * 3)];
            if (data->result[cnt].confidence >= LIGHTRANGER9_CONFIDENCE_THRESHOLD) {
                data->result[cnt].distance_mm = 
                    ((uint16_t)data_buf[LIGHTRANGER9_REG_RES_DISTANCE_0_MSB - LIGHTRANGER9_REG_BLOCKREAD + (cnt * 3)] << 8) | 
                    data_buf[LIGHTRANGER9_REG_RES_DISTANCE_0_LSB - LIGHTRANGER9_REG_BLOCKREAD + (cnt * 3)];
            } else {
                data->result[cnt].distance_mm = 0;
            }
        }
        return 0;
    }

    return -1;
}

static int lightranger9_channel_get(const struct device *dev,
                                    enum sensor_channel chan,
                                    struct sensor_value *val)
{
    // Does nothing

    return 0;
}

static int lightranger9_enable_device(const struct device *dev)
{
    const lightranger9_config_t *cfg = dev->config;
    int ret;
    
    /**
     * Claim pin from NETcore
     */
    nrf_gpio_pin_mcu_select(cfg->control_pin, NRF_GPIO_PIN_MCUSEL_APP);

    ret = gpio_pin_configure(cfg->control_ctrl, cfg->control_pin, GPIO_OUTPUT_LOW | cfg->control_flags);
    ret = gpio_pin_configure(cfg->control_ctrl, cfg->gpio1_pin,   GPIO_OUTPUT_LOW | cfg->gpio1_flags);
    ret = gpio_pin_configure(cfg->control_ctrl, cfg->int_pin,     GPIO_INPUT      | cfg->int_flags);
    k_msleep(500);
    ret = gpio_pin_set(cfg->control_ctrl, cfg->control_pin, 1);
    ret = gpio_pin_set(cfg->control_ctrl, cfg->gpio1_pin, 0);

    if (ret) {
        LOG_ERR("Could not enable device!");
    } else {
        //do nothing
    }
    
    return ret;
}

static int lightranger9_init(const struct device *dev)
{
    int error_flag = 0;

    error_flag = lightranger9_enable_device(dev);
    if (error_flag) {
        LOG_ERR("Failed to setup pin!");
        return -1;
    } else {
        // do nothing
    }

    k_msleep(100);
    if (!lightranger9_check_communication(dev)) {
        LOG_ERR("No Communication with device!");
        return -1;
    } else {
        // do nothing
    }

    // Activate oscillator
    error_flag |= lightranger9_write_register(dev,
                                              LIGHTRANGER9_REG_ENABLE,
                                              LIGHTRANGER9_ENABLE_PON);
    k_msleep(100);

    // Download FW bin if the bootloader is running
    error_flag |= lightranger9_download_fw_bin(dev);
    
    // Set measurement period
    error_flag |= lightranger9_write_register(dev,
                                              LIGHTRANGER9_REG_CONFIG_RESULT,
                                              LIGHTRANGER9_CONFIG_RESULT_COMMON_CID);
    error_flag |= lightranger9_write_register(dev,
                                              LIGHTRANGER9_REG_CMD_STAT,
                                              LIGHTRANGER9_CMD_STAT_LOAD_CFG_PAGE_COMMON);
    k_msleep(100);
    
    error_flag |= lightranger9_write_register(dev,
                                              LIGHTRANGER9_REG_PERIOD_MS_LSB,
                                              (uint8_t)((LIGHTRANGER9_DEFAULT_MEASUREMENT_PERIOD_MS)      & 0xFF));
    error_flag |= lightranger9_write_register(dev,
                                              LIGHTRANGER9_REG_PERIOD_MS_MSB,
                                              (uint8_t)((LIGHTRANGER9_DEFAULT_MEASUREMENT_PERIOD_MS >> 8) & 0xFF));
    error_flag |= lightranger9_write_register(dev,
                                              LIGHTRANGER9_REG_CMD_STAT,
                                              LIGHTRANGER9_CMD_STAT_WRITE_CFG_PAGE);
    k_msleep(100);

    //Enable measurement ready interrupt, start measurement and clear interrupts
    error_flag |= lightranger9_write_register(dev,
                                              LIGHTRANGER9_REG_INT_ENAB,
                                              LIGHTRANGER9_INT_ENAB_MEAS_READY);
    error_flag |= lightranger9_write_register(dev,
                                              LIGHTRANGER9_REG_CONFIG_RESULT,
                                              LIGHTRANGER9_CONFIG_RESULT_MEAS);
    error_flag |= lightranger9_write_register(dev,
                                              LIGHTRANGER9_REG_CMD_STAT,
                                              LIGHTRANGER9_CMD_STAT_MEASURE);
    error_flag |= lightranger9_clear_interrupts(dev);
    k_msleep(100);

    if (error_flag == 0) {
        LOG_DBG("Sensor initialized successfully!");
    } else {
        LOG_DBG("Sensor initialization failed!");
    }

    return error_flag;
}

static const struct sensor_driver_api lightranger9_api = {
    .sample_fetch = lightranger9_sample_fetch,
    .channel_get  = lightranger9_channel_get
};

DEVICE_DT_INST_DEFINE(0,
                      lightranger9_init,
                      NULL,
                      &lightranger9_meas_cpt,
                      &lightranger9_config,
                      POST_KERNEL,
                      CONFIG_SENSOR_INIT_PRIORITY,
                      &lightranger9_api);