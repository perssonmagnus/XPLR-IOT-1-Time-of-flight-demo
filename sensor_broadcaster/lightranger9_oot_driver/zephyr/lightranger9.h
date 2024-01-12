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

#ifndef ZEPHYR_DRIVERS_SENSOR_LIGHTRANGER9_LIGHTRANGER9_H_
#define ZEPHYR_DRIVERS_SENSOR_LIGHTRANGER9_LIGHTRANGER9_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <device.h>
#include <kernel.h>
#include <drivers/i2c.h>
#include <drivers/sensor.h>
#include <drivers/gpio.h>

#define LIGHTRANGER9_REG_APPID                      0x00
#define LIGHTRANGER9_REG_MINOR                      0x01
#define LIGHTRANGER9_REG_ENABLE                     0xE0
#define LIGHTRANGER9_REG_INT_STATUS                 0xE1
#define LIGHTRANGER9_REG_INT_ENAB                   0xE2
#define LIGHTRANGER9_REG_ID                         0xE3
#define LIGHTRANGER9_REG_REVID                      0xE4

/**
 * @brief LightRanger 9 main application registers list.
 * @details Specified main application registers list 
 * (appid = 0x03, any cid_rid) of LightRanger 9 Click driver.
 */
#define LIGHTRANGER9_REG_PATCH                      0x02
#define LIGHTRANGER9_REG_BUILD_TYPE                 0x03
#define LIGHTRANGER9_REG_APPLICATION_STATUS         0x04
#define LIGHTRANGER9_REG_MEASURE_STATUS             0x05
#define LIGHTRANGER9_REG_ALGORITHM_STATUS           0x06
#define LIGHTRANGER9_REG_CALIBRATION_STATUS         0x07
#define LIGHTRANGER9_REG_CMD_STAT                   0x08
#define LIGHTRANGER9_REG_PREV_CMD                   0x09
#define LIGHTRANGER9_REG_MODE                       0x10
#define LIGHTRANGER9_REG_LIVE_BEAT                  0x0A
#define LIGHTRANGER9_REG_ACTIVE_RANGE               0x19
#define LIGHTRANGER9_REG_SERIAL_NUMBER_0            0x1C
#define LIGHTRANGER9_REG_SERIAL_NUMBER_1            0x1D
#define LIGHTRANGER9_REG_SERIAL_NUMBER_2            0x1E
#define LIGHTRANGER9_REG_SERIAL_NUMBER_3            0x1F
#define LIGHTRANGER9_REG_CONFIG_RESULT              0x20
#define LIGHTRANGER9_REG_TID                        0x21
#define LIGHTRANGER9_REG_SIZE_LSB                   0x22
#define LIGHTRANGER9_REG_SIZE_MSB                   0x23

/**
 * @brief LightRanger 9 measurements results registers list.
 * @details Specified measurements results registers list 
 * (appid = 0x03, cid_rid = 0x10) of LightRanger 9 Click driver.
 */
#define LIGHTRANGER9_REG_BLOCKREAD                  0x20
#define LIGHTRANGER9_REG_RESULT_NUMBER              0x24
#define LIGHTRANGER9_REG_TEMPERATURE                0x25
#define LIGHTRANGER9_REG_NUMBER_VALID_RESULTS       0x26
#define LIGHTRANGER9_REG_AMBIENT_LIGHT_0            0x28
#define LIGHTRANGER9_REG_AMBIENT_LIGHT_1            0x29
#define LIGHTRANGER9_REG_AMBIENT_LIGHT_2            0x2A
#define LIGHTRANGER9_REG_AMBIENT_LIGHT_3            0x2B
#define LIGHTRANGER9_REG_PHOTON_COUNT_0             0x2C
#define LIGHTRANGER9_REG_PHOTON_COUNT_1             0x2D
#define LIGHTRANGER9_REG_PHOTON_COUNT_2             0x2E
#define LIGHTRANGER9_REG_PHOTON_COUNT_3             0x2F
#define LIGHTRANGER9_REG_REFERENCE_COUNT_0          0x30
#define LIGHTRANGER9_REG_REFERENCE_COUNT_1          0x31
#define LIGHTRANGER9_REG_REFERENCE_COUNT_2          0x32
#define LIGHTRANGER9_REG_REFERENCE_COUNT_3          0x33
#define LIGHTRANGER9_REG_SYS_TICK_0                 0x34
#define LIGHTRANGER9_REG_SYS_TICK_1                 0x35
#define LIGHTRANGER9_REG_SYS_TICK_2                 0x36
#define LIGHTRANGER9_REG_SYS_TICK_3                 0x37
#define LIGHTRANGER9_REG_RES_CONFIDENCE_0           0x38
#define LIGHTRANGER9_REG_RES_DISTANCE_0_LSB         0x39
#define LIGHTRANGER9_REG_RES_DISTANCE_0_MSB         0x3A
#define LIGHTRANGER9_REG_RES_CONFIDENCE_1           0x3B
#define LIGHTRANGER9_REG_RES_DISTANCE_1_LSB         0x3C
#define LIGHTRANGER9_REG_RES_DISTANCE_1_MSB         0x3D
#define LIGHTRANGER9_REG_RES_CONFIDENCE_2           0x3E
#define LIGHTRANGER9_REG_RES_DISTANCE_2_LSB         0x3F
#define LIGHTRANGER9_REG_RES_DISTANCE_2_MSB         0x40
#define LIGHTRANGER9_REG_RES_CONFIDENCE_3           0x41
#define LIGHTRANGER9_REG_RES_DISTANCE_3_LSB         0x42
#define LIGHTRANGER9_REG_RES_DISTANCE_3_MSB         0x43
#define LIGHTRANGER9_REG_RES_CONFIDENCE_4           0x44
#define LIGHTRANGER9_REG_RES_DISTANCE_4_LSB         0x45
#define LIGHTRANGER9_REG_RES_DISTANCE_4_MSB         0x46
#define LIGHTRANGER9_REG_RES_CONFIDENCE_5           0x47
#define LIGHTRANGER9_REG_RES_DISTANCE_5_LSB         0x48
#define LIGHTRANGER9_REG_RES_DISTANCE_5_MSB         0x49
#define LIGHTRANGER9_REG_RES_CONFIDENCE_6           0x4A
#define LIGHTRANGER9_REG_RES_DISTANCE_6_LSB         0x4B
#define LIGHTRANGER9_REG_RES_DISTANCE_6_MSB         0x4C
#define LIGHTRANGER9_REG_RES_CONFIDENCE_7           0x4D
#define LIGHTRANGER9_REG_RES_DISTANCE_7_LSB         0x4E
#define LIGHTRANGER9_REG_RES_DISTANCE_7_MSB         0x4F
#define LIGHTRANGER9_REG_RES_CONFIDENCE_8           0x50
#define LIGHTRANGER9_REG_RES_DISTANCE_8_LSB         0x51
#define LIGHTRANGER9_REG_RES_DISTANCE_8_MSB         0x52
#define LIGHTRANGER9_REG_RES_CONFIDENCE_9           0x53
#define LIGHTRANGER9_REG_RES_DISTANCE_9_LSB         0x54
#define LIGHTRANGER9_REG_RES_DISTANCE_9_MSB         0x55
#define LIGHTRANGER9_REG_RES_CONFIDENCE_10          0x56
#define LIGHTRANGER9_REG_RES_DISTANCE_10_LSB        0x57
#define LIGHTRANGER9_REG_RES_DISTANCE_10_MSB        0x58
#define LIGHTRANGER9_REG_RES_CONFIDENCE_11          0x59
#define LIGHTRANGER9_REG_RES_DISTANCE_11_LSB        0x5A
#define LIGHTRANGER9_REG_RES_DISTANCE_11_MSB        0x5B
#define LIGHTRANGER9_REG_RES_CONFIDENCE_12          0x5C
#define LIGHTRANGER9_REG_RES_DISTANCE_12_LSB        0x5D
#define LIGHTRANGER9_REG_RES_DISTANCE_12_MSB        0x5E
#define LIGHTRANGER9_REG_RES_CONFIDENCE_13          0x5F
#define LIGHTRANGER9_REG_RES_DISTANCE_13_LSB        0x60
#define LIGHTRANGER9_REG_RES_DISTANCE_13_MSB        0x61
#define LIGHTRANGER9_REG_RES_CONFIDENCE_14          0x62
#define LIGHTRANGER9_REG_RES_DISTANCE_14_LSB        0x63
#define LIGHTRANGER9_REG_RES_DISTANCE_14_MSB        0x64
#define LIGHTRANGER9_REG_RES_CONFIDENCE_15          0x65
#define LIGHTRANGER9_REG_RES_DISTANCE_15_LSB        0x66
#define LIGHTRANGER9_REG_RES_DISTANCE_15_MSB        0x67
#define LIGHTRANGER9_REG_RES_CONFIDENCE_16          0x68
#define LIGHTRANGER9_REG_RES_DISTANCE_16_LSB        0x69
#define LIGHTRANGER9_REG_RES_DISTANCE_16_MSB        0x6A
#define LIGHTRANGER9_REG_RES_CONFIDENCE_17          0x6B
#define LIGHTRANGER9_REG_RES_DISTANCE_17_LSB        0x6C
#define LIGHTRANGER9_REG_RES_DISTANCE_17_MSB        0x6D
#define LIGHTRANGER9_REG_RES_CONFIDENCE_18          0x6E
#define LIGHTRANGER9_REG_RES_DISTANCE_18_LSB        0x6F
#define LIGHTRANGER9_REG_RES_DISTANCE_18_MSB        0x70
#define LIGHTRANGER9_REG_RES_CONFIDENCE_19          0x71
#define LIGHTRANGER9_REG_RES_DISTANCE_19_LSB        0x72
#define LIGHTRANGER9_REG_RES_DISTANCE_19_MSB        0x73
#define LIGHTRANGER9_REG_RES_CONFIDENCE_20          0x74
#define LIGHTRANGER9_REG_RES_DISTANCE_20_LSB        0x75
#define LIGHTRANGER9_REG_RES_DISTANCE_20_MSB        0x76
#define LIGHTRANGER9_REG_RES_CONFIDENCE_21          0x77
#define LIGHTRANGER9_REG_RES_DISTANCE_21_LSB        0x78
#define LIGHTRANGER9_REG_RES_DISTANCE_21_MSB        0x79
#define LIGHTRANGER9_REG_RES_CONFIDENCE_22          0x7A
#define LIGHTRANGER9_REG_RES_DISTANCE_22_LSB        0x7B
#define LIGHTRANGER9_REG_RES_DISTANCE_22_MSB        0x7C
#define LIGHTRANGER9_REG_RES_CONFIDENCE_23          0x7D
#define LIGHTRANGER9_REG_RES_DISTANCE_23_LSB        0x7E
#define LIGHTRANGER9_REG_RES_DISTANCE_23_MSB        0x7F
#define LIGHTRANGER9_REG_RES_CONFIDENCE_24          0x80
#define LIGHTRANGER9_REG_RES_DISTANCE_24_LSB        0x81
#define LIGHTRANGER9_REG_RES_DISTANCE_24_MSB        0x82
#define LIGHTRANGER9_REG_RES_CONFIDENCE_25          0x83
#define LIGHTRANGER9_REG_RES_DISTANCE_25_LSB        0x84
#define LIGHTRANGER9_REG_RES_DISTANCE_25_MSB        0x85
#define LIGHTRANGER9_REG_RES_CONFIDENCE_26          0x86
#define LIGHTRANGER9_REG_RES_DISTANCE_26_LSB        0x87
#define LIGHTRANGER9_REG_RES_DISTANCE_26_MSB        0x88
#define LIGHTRANGER9_REG_RES_CONFIDENCE_27          0x89
#define LIGHTRANGER9_REG_RES_DISTANCE_27_LSB        0x8A
#define LIGHTRANGER9_REG_RES_DISTANCE_27_MSB        0x8B
#define LIGHTRANGER9_REG_RES_CONFIDENCE_28          0x8C
#define LIGHTRANGER9_REG_RES_DISTANCE_28_LSB        0x8D
#define LIGHTRANGER9_REG_RES_DISTANCE_28_MSB        0x8E
#define LIGHTRANGER9_REG_RES_CONFIDENCE_29          0x8F
#define LIGHTRANGER9_REG_RES_DISTANCE_29_LSB        0x90
#define LIGHTRANGER9_REG_RES_DISTANCE_29_MSB        0x91
#define LIGHTRANGER9_REG_RES_CONFIDENCE_30          0x92
#define LIGHTRANGER9_REG_RES_DISTANCE_30_LSB        0x93
#define LIGHTRANGER9_REG_RES_DISTANCE_30_MSB        0x94
#define LIGHTRANGER9_REG_RES_CONFIDENCE_31          0x95
#define LIGHTRANGER9_REG_RES_DISTANCE_31_LSB        0x96
#define LIGHTRANGER9_REG_RES_DISTANCE_31_MSB        0x97
#define LIGHTRANGER9_REG_RES_CONFIDENCE_32          0x98
#define LIGHTRANGER9_REG_RES_DISTANCE_32_LSB        0x99
#define LIGHTRANGER9_REG_RES_DISTANCE_32_MSB        0x9A
#define LIGHTRANGER9_REG_RES_CONFIDENCE_33          0x9B
#define LIGHTRANGER9_REG_RES_DISTANCE_33_LSB        0x9C
#define LIGHTRANGER9_REG_RES_DISTANCE_33_MSB        0x9D
#define LIGHTRANGER9_REG_RES_CONFIDENCE_34          0x9E
#define LIGHTRANGER9_REG_RES_DISTANCE_34_LSB        0x9F
#define LIGHTRANGER9_REG_RES_DISTANCE_34_MSB        0xA0
#define LIGHTRANGER9_REG_RES_CONFIDENCE_35          0xA1
#define LIGHTRANGER9_REG_RES_DISTANCE_35_LSB        0xA2
#define LIGHTRANGER9_REG_RES_DISTANCE_35_MSB        0xA3

/**
 * @brief LightRanger 9 configuration page registers list.
 * @details Specified configuration page registers list 
 * (appid = 0x03, cid_rid = 0x16) of LightRanger 9 Click driver.
 */
#define LIGHTRANGER9_REG_PERIOD_MS_LSB              0x24
#define LIGHTRANGER9_REG_PERIOD_MS_MSB              0x25
#define LIGHTRANGER9_REG_KILO_ITERATIONS_LSB        0x26
#define LIGHTRANGER9_REG_KILO_ITERATIONS_MSB        0x27
#define LIGHTRANGER9_REG_INT_THRESHOLD_LOW_LSB      0x28
#define LIGHTRANGER9_REG_INT_THRESHOLD_LOW_MSB      0x29
#define LIGHTRANGER9_REG_INT_THRESHOLD_HIGH_LSB     0x2A
#define LIGHTRANGER9_REG_INT_THRESHOLD_HIGH_MSB     0x2B
#define LIGHTRANGER9_REG_INT_ZONE_MASK_0            0x2C
#define LIGHTRANGER9_REG_INT_ZONE_MASK_1            0x2D
#define LIGHTRANGER9_REG_INT_ZONE_MASK_2            0x2E
#define LIGHTRANGER9_REG_INT_PERSISTANCE            0x2F
#define LIGHTRANGER9_REG_CONFIDENCE_THRESHOLD       0x30
#define LIGHTRANGER9_REG_GPIO_0                     0x31
#define LIGHTRANGER9_REG_GPIO_1                     0x32
#define LIGHTRANGER9_REG_POWER_CFG                  0x33
#define LIGHTRANGER9_REG_SPAD_MAP_ID                0x34
#define LIGHTRANGER9_REG_ALG_SETTING_0              0x35
#define LIGHTRANGER9_REG_HIST_DUMP                  0x39
#define LIGHTRANGER9_REG_SPREAD_SPECTRUM            0x3A
#define LIGHTRANGER9_REG_I2C_SLAVE_ADDRESS          0x3B
#define LIGHTRANGER9_REG_OSC_TRIM_VALUE_LSB         0x3C
#define LIGHTRANGER9_REG_OSC_TRIM_VALUE_MSB         0x3D
#define LIGHTRANGER9_REG_I2C_ADDR_CHANGE            0x3E

/**
 * @brief LightRanger 9 user defined SPAD configuration registers list.
 * @details Specified user defined SPAD configuration registers list 
 * (appid = 0x03, cid_rid = 0x17/0x18) of LightRanger 9 Click driver.
 */
#define LIGHTRANGER9_REG_SPAD_ENABLE_FIRST          0x24
#define LIGHTRANGER9_REG_SPAD_ENABLE_LAST           0x41
#define LIGHTRANGER9_REG_SPAD_TDC_FIRST             0x42
#define LIGHTRANGER9_REG_SPAD_TDC_LAST              0x8C
#define LIGHTRANGER9_REG_SPAD_X_OFFSET_2            0x8D
#define LIGHTRANGER9_REG_SPAD_Y_OFFSET_2            0x8E
#define LIGHTRANGER9_REG_SPAD_X_SIZE                0x8F
#define LIGHTRANGER9_REG_SPAD_Y_SIZE                0x90

/**
 * @brief LightRanger 9 factory calibration registers list.
 * @details Specified factory calibration registers list 
 * (appid = 0x03, cid_rid = 0x19) of LightRanger 9 Click driver.
 */
#define LIGHTRANGER9_REG_FACTORY_CALIBRATION_FIRST  0x24
#define LIGHTRANGER9_REG_CROSSTALK_ZONE1            0x60
#define LIGHTRANGER9_REG_CROSSTALK_ZONE2            0x64
#define LIGHTRANGER9_REG_CROSSTALK_ZONE3            0x68
#define LIGHTRANGER9_REG_CROSSTALK_ZONE4            0x6C
#define LIGHTRANGER9_REG_CROSSTALK_ZONE5            0x70
#define LIGHTRANGER9_REG_CROSSTALK_ZONE6            0x74
#define LIGHTRANGER9_REG_CROSSTALK_ZONE7            0x78
#define LIGHTRANGER9_REG_CROSSTALK_ZONE8            0x7C
#define LIGHTRANGER9_REG_CROSSTALK_ZONE9            0x80
#define LIGHTRANGER9_REG_CROSSTALK_ZONE1_TMUX       0xB8
#define LIGHTRANGER9_REG_CROSSTALK_ZONE2_TMUX       0xBC
#define LIGHTRANGER9_REG_CROSSTALK_ZONE3_TMUX       0xC0
#define LIGHTRANGER9_REG_CROSSTALK_ZONE4_TMUX       0xC4
#define LIGHTRANGER9_REG_CROSSTALK_ZONE5_TMUX       0xC8
#define LIGHTRANGER9_REG_CROSSTALK_ZONE6_TMUX       0xCC
#define LIGHTRANGER9_REG_CROSSTALK_ZONE7_TMUX       0xD0
#define LIGHTRANGER9_REG_CROSSTALK_ZONE8_TMUX       0xD4
#define LIGHTRANGER9_REG_CROSSTALK_ZONE9_TMUX       0xD8
#define LIGHTRANGER9_REG_CALIBRATION_STATUS_FC      0xDC
#define LIGHTRANGER9_REG_FACTORY_CALIBRATION_LAST   0xDF

/**
 * @brief LightRanger 9 raw data histograms registers list.
 * @details Specified raw data histograms registers list 
 * (appid = 0x03, cid_rid = 0x81) of LightRanger 9 Click driver.
 */
#define LIGHTRANGER9_REG_SUBPACKET_NUMBER           0x24
#define LIGHTRANGER9_REG_SUBPACKET_PAYLOAD          0x25
#define LIGHTRANGER9_REG_SUBPACKET_CONFIG           0x26
#define LIGHTRANGER9_REG_SUBPACKET_DATA0            0x27
#define LIGHTRANGER9_REG_SUBPACKET_DATA127          0xA6

/**
 * @brief LightRanger 9 bootloader registers list.
 * @details Specified bootloader registers list 
 * (appid = 0x80) of LightRanger 9 Click driver.
 */
#define LIGHTRANGER9_REG_BL_CMD_STAT                0x08
#define LIGHTRANGER9_REG_BL_SIZE                    0x09
#define LIGHTRANGER9_REG_BL_DATA                    0x0A

/*! @} */ // lightranger9_reg

/**
 * @defgroup lightranger9_set LightRanger 9 Registers Settings
 * @brief Settings for registers of LightRanger 9 Click driver.
 */

/**
 * @addtogroup lightranger9_set
 * @{
 */

/**
 * @brief LightRanger 9 timeout value.
 * @details Specified timeout value of LightRanger 9 Click driver.
 */
#define LIGHTRANGER9_TIMEOUT                        5000

/**
 * @brief LightRanger 9 enable register settings.
 * @details Specified enable register settings of LightRanger 9 Click driver.
 */
#define LIGHTRANGER9_ENABLE_CPU_READY               0x40
#define LIGHTRANGER9_ENABLE_POWERUP_BL              0x00
#define LIGHTRANGER9_ENABLE_POWERUP_BL_NO_SLP       0x10
#define LIGHTRANGER9_ENABLE_POWERUP_RAM             0x20
#define LIGHTRANGER9_ENABLE_PON                     0x01

/**
 * @brief LightRanger 9 int enable register settings.
 * @details Specified int enable register settings of LightRanger 9 Click driver.
 */
#define LIGHTRANGER9_INT_ENAB_REG_STATUS            0x40
#define LIGHTRANGER9_INT_ENAB_COMMAND               0x20
#define LIGHTRANGER9_INT_ENAB_HIST_READY            0x08
#define LIGHTRANGER9_INT_ENAB_MEAS_READY            0x02

/**
 * @brief LightRanger 9 int status register settings.
 * @details Specified int status register settings of LightRanger 9 Click driver.
 */
#define LIGHTRANGER9_INT_STATUS_REG_STATUS          0x40
#define LIGHTRANGER9_INT_STATUS_COMMAND             0x20
#define LIGHTRANGER9_INT_STATUS_HIST_READY          0x08
#define LIGHTRANGER9_INT_STATUS_MEAS_READY          0x02

/**
 * @brief LightRanger 9 cmd stat register write settings.
 * @details Specified cmd stat register write settings of LightRanger 9 Click driver.
 */
#define LIGHTRANGER9_CMD_STAT_MEASURE               0x10
#define LIGHTRANGER9_CMD_STAT_CLEAR_STATUS          0x11
#define LIGHTRANGER9_CMD_STAT_GPIO                  0x12
#define LIGHTRANGER9_CMD_STAT_WRITE_CFG_PAGE        0x15
#define LIGHTRANGER9_CMD_STAT_LOAD_CFG_PAGE_COMMON  0x16
#define LIGHTRANGER9_CMD_STAT_LOAD_CFG_PAGE_SPAD_1  0x17
#define LIGHTRANGER9_CMD_STAT_LOAD_CFG_PAGE_SPAD_2  0x18
#define LIGHTRANGER9_CMD_STAT_LOAD_CFG_PAGE_F_Y_CAL 0x19
#define LIGHTRANGER9_CMD_STAT_FACTORY_CALIBRATION   0x20
#define LIGHTRANGER9_CMD_STAT_I2C_SLAVE_ADDRESS     0x21
#define LIGHTRANGER9_CMD_STAT_FORCE_TMF8820         0x65
#define LIGHTRANGER9_CMD_STAT_FORCE_TMF8828         0x6C
#define LIGHTRANGER9_CMD_STAT_RESET                 0xFE
#define LIGHTRANGER9_CMD_STAT_STOP                  0xFF

/**
 * @brief LightRanger 9 cmd stat register read settings.
 * @details Specified cmd stat register read settings of LightRanger 9 Click driver.
 */
#define LIGHTRANGER9_CMD_STAT_OK                    0x00
#define LIGHTRANGER9_CMD_STAT_ACCEPTED              0x01
#define LIGHTRANGER9_CMD_ERR_CONFIG                 0x02
#define LIGHTRANGER9_CMD_ERR_APPLICATION            0x03
#define LIGHTRANGER9_CMD_ERR_WAKEUP_TIMED           0x04
#define LIGHTRANGER9_CMD_ERR_RESET_UNEXPECTED       0x05
#define LIGHTRANGER9_CMD_ERR_UNKNOWN_CMD            0x06
#define LIGHTRANGER9_CMD_ERR_NO_REF_SPAD            0x07
#define LIGHTRANGER9_CMD_ERR_UNKNOWN_CID            0x09
#define LIGHTRANGER9_CMD_WARNING_CFG_SPAD_1         0x0A
#define LIGHTRANGER9_CMD_WARNING_CFG_SPAD_2         0x0B
#define LIGHTRANGER9_CMD_WARNING_OSC_TRIP           0x0C
#define LIGHTRANGER9_CMD_WARNING_I2C_ADDRESS        0x0D
#define LIGHTRANGER9_CMD_ERR_UNKNOWN_MODE           0x0E

/**
 * @brief LightRanger 9 config result register settings.
 * @details Specified config result register settings of LightRanger 9 Click driver.
 */
#define LIGHTRANGER9_CONFIG_RESULT_MEAS             0x10
#define LIGHTRANGER9_CONFIG_RESULT_COMMON_CID       0x16
#define LIGHTRANGER9_CONFIG_RESULT_SPAD_1_CID       0x17
#define LIGHTRANGER9_CONFIG_RESULT_SPAD_2_CID       0x18
#define LIGHTRANGER9_CONFIG_RESULT_FAC_CALIB_CID    0x19
#define LIGHTRANGER9_CONFIG_RESULT_HIST_RAW_CID     0x81

/**
 * @brief LightRanger 9 bootloader commands list.
 * @details Specified bootloader commands list of LightRanger 9 Click driver.
 */
#define LIGHTRANGER9_BL_CMD_RAMREMAP_RESET          0x11
#define LIGHTRANGER9_BL_CMD_DOWNLOAD_INIT           0x14
#define LIGHTRANGER9_BL_CMD_RAM_BIST                0x2A
#define LIGHTRANGER9_BL_CMD_I2C_BIST                0x2C
#define LIGHTRANGER9_BL_CMD_W_RAM                   0x41
#define LIGHTRANGER9_BL_CMD_ADDR_RAM                0x43

/**
 * @brief LightRanger 9 bootloader commands status list.
 * @details Specified bootloader commands status list of LightRanger 9 Click driver.
 */
#define LIGHTRANGER9_BL_CMD_STAT_READY              0x00
#define LIGHTRANGER9_BL_CMD_STAT_ERR_SIZE           0x01
#define LIGHTRANGER9_BL_CMD_STAT_ERR_CSUM           0x02
#define LIGHTRANGER9_BL_CMD_STAT_ERR_RANGE          0x03
#define LIGHTRANGER9_BL_CMD_STAT_ERR_MORE           0x04

/**
 * @brief LightRanger 9 app id settings.
 * @details Specified app id settings of LightRanger 9 Click driver.
 */
#define LIGHTRANGER9_APP_ID_BOOTLOADER              0x80
#define LIGHTRANGER9_APP_ID_MEASUREMENT             0x03

/**
 * @brief LightRanger 9 capture settings.
 * @details Specified capture settings of LightRanger 9 Click driver.
 */
#define LIGHTRANGER9_BLOCKREAD_SIZE                 132
#define LIGHTRANGER9_MAX_MEAS_RESULTS               36
#define LIGHTRANGER9_SUBCAPTURE_0                   0
#define LIGHTRANGER9_SUBCAPTURE_1                   1
#define LIGHTRANGER9_SUBCAPTURE_2                   2
#define LIGHTRANGER9_SUBCAPTURE_3                   3
#define LIGHTRANGER9_SUBCAPTURE_MASK                0x03
#define LIGHTRANGER9_RESULT_NUMBER_MASK             0x3F
#define LIGHTRANGER9_SYS_TICK_TO_SEC                0.0000002
#define LIGHTRANGER9_OBJECT_MAP_SIZE                64

/**
 * @brief LightRanger 9 default measurement period and confidence threshold.
 * @details Specified default measurement period and confidence threshold of LightRanger 9 Click driver.
 */
#define LIGHTRANGER9_DEFAULT_MEASUREMENT_PERIOD_MS  1000
#define LIGHTRANGER9_CONFIDENCE_THRESHOLD           100

/**
 * @brief LightRanger 9 device ID value.
 * @details Specified device ID value of LightRanger 9 Click driver.
 */
#define LIGHTRANGER9_DEVICE_ID                      0x08

enum lightranger9_channel {
    LIGHTRANGER9_SENSOR_CHAN_SUB_CAPTURE = SENSOR_CHAN_PRIV_START,
    LIGHTRANGER9_SENSOR_CHAN_RESULT_NUMBER,
    LIGHTRANGER9_SENSOR_CHAN_SENSOR_TEMPERATURE,
    LIGHTRANGER9_SENSOR_CHAN_VALID_RESULTS,
    LIGHTRANGER9_SENSOR_CHAN_AMBIENT_LIGHT,
    LIGHTRANGER9_SENSOR_CHAN_PHOTON_COUNT,
    LIGHTRANGER9_SENSOR_CHAN_REFERENCE_COUNT,
    LIGHTRANGER9_SENSOR_CHAN_SYS_TICK_SEC
};

/**
 * @brief LightRanger 9 Click return value data.
 * @details Predefined enum values for driver return values.
 */
typedef enum
{
    LIGHTRANGER9_OK = 0,
    LIGHTRANGER9_ERROR = -1
} lightranger9_return_value_t;

/**
 * @brief Configuration of LIGHTRANGER9 module
 */
typedef struct lightranger9_config_type {
    struct i2c_dt_spec bus;
    const struct device *control_ctrl;
    gpio_pin_t control_pin;
    gpio_dt_flags_t control_flags;
    gpio_pin_t int_pin;
    gpio_dt_flags_t int_flags;
    gpio_pin_t gpio1_pin;
    gpio_dt_flags_t gpio1_flags;
} lightranger9_config_t;

/**
 * @brief Distance measurements struct
 */
typedef struct {
    uint8_t confidence;
    uint16_t distance_mm;
} lightranger9_meas_result_t;

/**
 * @brief A single capture of measurements
 */
typedef struct __attribute__((__packed__)) lightranger9_meas_cpt_type {
    uint8_t sub_capture;
    uint8_t result_number;
    int8_t temperature;
    uint8_t valid_results;
    uint32_t ambient_light;
    uint32_t photon_count;
    uint32_t reference_count;
    float sys_tick_sec;
    lightranger9_meas_result_t result[LIGHTRANGER9_MAX_MEAS_RESULTS];
} lightranger9_meas_cpt_t;

/**
 * @brief Contains measurements
 */
typedef struct __attribute__((__packed__)) lightranger9_measurement_type {
    uint8_t result_number;
    int8_t temperature;
    uint8_t valid_results;
    uint32_t ambient_light;
    uint32_t photon_count;
    uint32_t reference_count;
    float sys_tick_sec;
    lightranger9_meas_result_t obj1[LIGHTRANGER9_OBJECT_MAP_SIZE];
    lightranger9_meas_result_t obj2[LIGHTRANGER9_OBJECT_MAP_SIZE];
} lightranger9_measurement_t;

/**
 * @brief Gets measurements from sensor.
 * This is not the correct way of getting data from the sensor
 * but channel get functions return one value  each time.
 * Instead LIGHTRANGER9 return an array of measured distances.
 * 
 * @param dev        sensor device.
 * @param sens_data  lightranger9 data struct.
 */
void lightranger9_get_measurements(const struct device *dev, lightranger9_meas_cpt_t *sens_data);

/**
 * @brief Called to clear interrupts so we can move to next measurement
 * 
 * @param dev  sensor device.
 */
void lightranger9_clear_ints(const struct device *dev);

/**
 * @brief Gets interrupt status pin
 * 
 * @param dev  sensor device.
 * @return     true if pin high (still on measurement) otherwise false.
 */
bool lightranger9_get_interrupt_pin(const struct device *dev);

/**
 * @brief Parses measurements from caprtures.
 * NOTE: for a single measurements 4 discrete captures are required.
 * 
 * @param capture       a capture to convert.
 * @param measurements  parsed measurement.
 * @return              true if we parsed 4 captures to a single complete
 *                      measurement otherwise false.
 */
bool lightranger9_parse_measurement(lightranger9_meas_cpt_t *capture,
                                    lightranger9_measurement_t *measurements);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_DRIVERS_SENSOR_LIGHTRANGER9_LIGHTRANGER9_H_ */