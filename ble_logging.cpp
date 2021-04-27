/*
 * Mbed-OS Microcontroller Library
 * Copyright (c) 2020 Embedded Planet
 * Copyright (c) 2020 ARM Limited
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
 * limitations under the License
 */

#include "ble_logging.h"

#define TRACE_GROUP BLE_UTILS_TRACE_GROUP

void ble_log_error(ble_error_t error, const char *msg) {
    switch(error) {
        case BLE_ERROR_NONE:
            tr_error("%s: BLE_ERROR_NONE: No error", msg);
            break;
        case BLE_ERROR_BUFFER_OVERFLOW:
            tr_error("%s: BLE_ERROR_BUFFER_OVERFLOW: The requested action would cause a buffer overflow and has been aborted", msg);
            break;
        case BLE_ERROR_NOT_IMPLEMENTED:
            tr_error("%s: BLE_ERROR_NOT_IMPLEMENTED: Requested a feature that isn't yet implement or isn't supported by the target HW", msg);
            break;
        case BLE_ERROR_PARAM_OUT_OF_RANGE:
            tr_error("%s: BLE_ERROR_PARAM_OUT_OF_RANGE: One of the supplied parameters is outside the valid range", msg);
            break;
        case BLE_ERROR_INVALID_PARAM:
            tr_error("%s: BLE_ERROR_INVALID_PARAM: One of the supplied parameters is invalid", msg);
            break;
        case BLE_STACK_BUSY:
            tr_error("%s: BLE_STACK_BUSY: The stack is busy", msg);
            break;
        case BLE_ERROR_INVALID_STATE:
            tr_error("%s: BLE_ERROR_INVALID_STATE: Invalid state", msg);
            break;
        case BLE_ERROR_NO_MEM:
            tr_error("%s: BLE_ERROR_NO_MEM: Out of Memory", msg);
            break;
        case BLE_ERROR_OPERATION_NOT_PERMITTED:
            tr_error("%s: BLE_ERROR_OPERATION_NOT_PERMITTED", msg);
            break;
        case BLE_ERROR_INITIALIZATION_INCOMPLETE:
            tr_error("%s: BLE_ERROR_INITIALIZATION_INCOMPLETE", msg);
            break;
        case BLE_ERROR_ALREADY_INITIALIZED:
            tr_error("%s: BLE_ERROR_ALREADY_INITIALIZED", msg);
            break;
        case BLE_ERROR_UNSPECIFIED:
            tr_error("%s: BLE_ERROR_UNSPECIFIED: Unknown error", msg);
            break;
        case BLE_ERROR_INTERNAL_STACK_FAILURE:
            tr_error("%s: BLE_ERROR_INTERNAL_STACK_FAILURE: internal stack failure", msg);
            break;
        case BLE_ERROR_NOT_FOUND:
            tr_error("%s: BLE_ERROR_NOT_FOUND", msg);
            break;
        default:
            tr_error("%s: Unknown error", msg);
    }
}

void ble_log_address(const ble::address_t &addr) {
    char mac_string[19];
    ble_log_sprintf_address(mac_string, addr);
    tr_info("%s", mac_string);
}

void ble_log_local_mac_address(BLE &ble) {
    /* Print out device MAC address to the console*/
    ble::own_address_type_t addr_type;
    ble::address_t address;
    char mac_string[19];
    ble.gap().getAddress(addr_type, address);
    ble_log_sprintf_address(mac_string, address);
    tr_info("Device MAC address: %s", mac_string);
}

void ble_log_sprintf_address(char *buf, const ble::address_t &addr) {
    snprintf(buf, 19, "%02x:%02x:%02x:%02x:%02x:%02x",
            addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
}

const char* phy_to_string(ble::phy_t phy) {
    switch(phy.value()) {
        case ble::phy_t::LE_1M:
            return "LE 1M";
        case ble::phy_t::LE_2M:
            return "LE 2M";
        case ble::phy_t::LE_CODED:
            return "LE coded";
        default:
            return "invalid PHY";
    }
}
