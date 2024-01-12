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

#include <sys/printk.h>
#include <logging/log.h>
#include <bluetooth/bluetooth.h>
#include "bt_broadcaster.h"

LOG_MODULE_REGISTER(BT_BROADCASTER, CONFIG_UART_CONSOLE_LOG_LEVEL);

/**
 * Maximum length of data we can put on an advertising package:
 */
#define BT_AD_EFFECTIVE_PAYLOAD 210
#define BT_ADD_MAIN_BUFF_CHUNK	206

/**
 * A header struct which will be in front of every message
 * (but after BT specific headers, populated by advertisement function)
 * 
 * - id: a 2 byte randomly generated header
 * - part_no: current part number of total packets to be sent.
 * - parts_total: total part expected by receiver
 */
typedef struct __attribute__((__packed__)) bt_data_header_type {
    uint8_t id[2];
    uint8_t part_no;
    uint8_t parts_total;
} bt_data_header_t;

static struct bt_data ad;
static struct bt_le_ext_adv *adv = NULL;
uint8_t tmp_buff[BT_AD_EFFECTIVE_PAYLOAD];
bt_data_header_t header;

int bt_broadcaster_create(void)
{
    int ret;
    struct bt_le_adv_param adv_param = {
        .id = BT_ID_DEFAULT,
        .sid = 0U, /* Supply unique SID when creating advertising set */
        .secondary_max_skip = 0U,
        .options = (BT_LE_ADV_OPT_EXT_ADV | BT_LE_ADV_OPT_USE_NAME),
        .interval_min = BT_GAP_ADV_FAST_INT_MIN_2,
        .interval_max = BT_GAP_ADV_FAST_INT_MAX_2,
        .peer = NULL,
    };

    if (adv == NULL) {
        ret = bt_le_ext_adv_create(&adv_param, NULL, &adv);
        if (!ret) {
            ret = bt_le_ext_adv_start(adv, BT_LE_EXT_ADV_START_DEFAULT);
            if (ret) {
                LOG_ERR("Failed to start advertiser with error (%d)", ret);
            }
        } else {
            LOG_ERR("Failed to create advertiser with error (%d)", ret);
        }
    } else {
        LOG_ERR("Could not create a broadcaster (adv != null)! could be already started!");
        ret = -ENOENT;
    }
    
    return ret;
}


int bt_broadcaster_send_message(uint8_t *buf, uint16_t len)
{
    int ret = -999;
    uint8_t parts;
    uint16_t cnt = 0;
    
    if ((buf != NULL) && (adv != NULL)) {
        /**
         * Calculates in how many parts the data should be split
         * Keep in mind that for each part the BT broadcaster function
         * will have to wait for 1sec so sending may packets will create
         * a total delay of x_parts*1sec
         */
        parts = ceiling_fraction(len, BT_AD_EFFECTIVE_PAYLOAD);
        header.parts_total = parts;
        ad.type = BT_DATA_MANUFACTURER_DATA;
        bt_rand(&header.id, sizeof(header.id));

        /* Set extended advertising data */
        for (cnt = 0; cnt < parts; cnt++) {
            memset(tmp_buff, 0, sizeof(tmp_buff));
            header.part_no = cnt + 1;
            /**
             * add header to payload
             */
            memcpy(tmp_buff,
                   &header,
                   sizeof(bt_data_header_t));
            
            /**
             * Add the remaining data
             */
            if (cnt == (parts - 1)) {
                memcpy(tmp_buff + sizeof(bt_data_header_t),
                       buf + (cnt * BT_ADD_MAIN_BUFF_CHUNK),
                       len - (cnt * BT_ADD_MAIN_BUFF_CHUNK));
                ad.data_len = (len - (cnt * BT_ADD_MAIN_BUFF_CHUNK)) + sizeof(bt_data_header_t);
            } else {
                memcpy(tmp_buff + sizeof(bt_data_header_t),
                        buf + (cnt * BT_ADD_MAIN_BUFF_CHUNK),
                        BT_ADD_MAIN_BUFF_CHUNK);
                ad.data_len = BT_AD_EFFECTIVE_PAYLOAD;
            }
            ad.data = tmp_buff;

            ret = bt_le_ext_adv_set_data(adv, &ad, 1, NULL, 0);
            /**
             * This is not elegant but for the sake of keeping the example
             * simple we are abusing the extended advertising data to send measurements
             * to a bluetooth device.
             * In order to achieve this we need a to wait for a second so the
             * BT has enough time to execute a couple of advertisements.
             */
            k_msleep(1500);
        }
    } else {
        ret = -ENOENT;
    }
    
    return ret;
}

int bt_broadcaster_delete(void)
{
    int ret;

    if (adv != NULL) {
        ret = bt_le_ext_adv_stop(adv);
        if (!ret) {
            ret = bt_le_ext_adv_delete(adv);
            if (ret) {
                adv = NULL;
                LOG_ERR("Failed to delete broadcaster with error (%d)!", ret);
            }
        } else {
            LOG_ERR("Failed to stop broadcaster with error (%d)!", ret);
        }
    } else {
        LOG_ERR("Broadcaster seems to not be initialized (adv = NULL)!");
        ret = -ENODATA;
    }
    
    return ret;
}