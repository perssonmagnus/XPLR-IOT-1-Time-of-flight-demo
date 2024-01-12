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

#ifndef BLUETOOTH_BROADCASTER_BT_BROADCASTER_H_
#define BLUETOOTH_BROADCASTER_BT_BROADCASTER_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create a Bluetooth Broadcaster
 * 
 * @return 0 on success else negative error on failure
 */
int bt_broadcaster_create(void);

/**
 * @brief Broadcasts a message of max 254 bytes
 * 
 * @param buf data buffer to broadcast
 * @param len data length
 * @return    0 on success else negative error on failure
 */
int bt_broadcaster_send_message(uint8_t *buf, uint16_t len);

/**
 * @brief Stops and deletes broadcaster
 * 
 * @return  0 on success else negative error on failure
 */
int bt_broadcaster_delete(void);

#ifdef __cplusplus
}
#endif

#endif /* BLUETOOTH_BROADCASTER_BT_BROADCASTER_H_ */