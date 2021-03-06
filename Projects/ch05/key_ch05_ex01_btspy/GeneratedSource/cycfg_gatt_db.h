/***************************************************************************//**
* File Name: cycfg_gatt_db.h
* Version: 2.20
*
* Description:
* Definitions for constants used in the device's GATT database and function
* prototypes.
* This file should not be modified. It was automatically generated by
* Bluetooth Configurator 2.20.0.3018
*
********************************************************************************
* Copyright 2021 Cypress Semiconductor Corporation
* SPDX-License-Identifier: Apache-2.0
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#if !defined(CYCFG_GATT_DB_H)
#define CYCFG_GATT_DB_H

#include "stdint.h"

#define __UUID_SERVICE_GENERIC_ACCESS               0x1800
#define __UUID_CHARACTERISTIC_DEVICE_NAME           0x2A00
#define __UUID_CHARACTERISTIC_APPEARANCE            0x2A01
#define __UUID_SERVICE_GENERIC_ATTRIBUTE            0x1801
#define __UUID_SERVICE_BT101                        0x06u, 0xF9u, 0xF4u, 0xE4u, 0x53u, 0xACu, 0x49u, 0x8Fu, 0x1Du, 0x48u, 0xEFu, 0xA4u, 0xB5u, 0xDEu, 0xC2u, 0xFDu
#define __UUID_CHARACTERISTIC_BT101_COUNTER         0x14u, 0x43u, 0x05u, 0x41u, 0x15u, 0x1Cu, 0x23u, 0xB3u, 0x3Eu, 0x45u, 0xC7u, 0xC5u, 0xAAu, 0xECu, 0x0Au, 0x52u
#define __UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION    0x2902
#define __UUID_DESCRIPTOR_CHARACTERISTIC_USER_DESCRIPTION    0x2901

/* Service Generic Access */
#define HDLS_GAP                                    0x01
/* Characteristic Device Name */
#define HDLC_GAP_DEVICE_NAME                        0x02
#define HDLC_GAP_DEVICE_NAME_VALUE                  0x03
/* Characteristic Appearance */
#define HDLC_GAP_APPEARANCE                         0x04
#define HDLC_GAP_APPEARANCE_VALUE                   0x05

/* Service Generic Attribute */
#define HDLS_GATT                                   0x06

/* Service BT101 */
#define HDLS_BT101                                  0x07
/* Characteristic Counter */
#define HDLC_BT101_COUNTER                          0x08
#define HDLC_BT101_COUNTER_VALUE                    0x09
/* Descriptor Client Characteristic Configuration */
#define HDLD_BT101_COUNTER_CLIENT_CHAR_CONFIG       0x0A
/* Descriptor Characteristic User Description */
#define HDLD_BT101_COUNTER_CHAR_USER_DESCRIPTION    0x0B

/* External Lookup Table Entry */
typedef struct
{
    uint16_t handle;
    uint16_t max_len;
    uint16_t cur_len;
    uint8_t  *p_data;
} gatt_db_lookup_table_t;

/* External definitions */
extern const uint8_t  gatt_database[];
extern const uint16_t gatt_database_len;
extern gatt_db_lookup_table_t app_gatt_db_ext_attr_tbl[];
extern const uint16_t app_gatt_db_ext_attr_tbl_size;
extern uint8_t app_gap_device_name[];
extern const uint16_t app_gap_device_name_len;
extern uint8_t app_gap_appearance[];
extern const uint16_t app_gap_appearance_len;
extern uint8_t app_bt101_counter[];
extern const uint16_t app_bt101_counter_len;
extern uint8_t app_bt101_counter_client_char_config[];
extern const uint16_t app_bt101_counter_client_char_config_len;
extern uint8_t app_bt101_counter_char_user_description[];
extern const uint16_t app_bt101_counter_char_user_description_len;

#endif /* CYCFG_GATT_DB_H */

