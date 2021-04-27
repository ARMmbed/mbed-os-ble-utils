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

#ifndef _BLE_LOGGING_H_
#define _BLE_LOGGING_H_

#include "ble/BLE.h"

#include "mbed_trace.h"

#define BLE_UTILS_TRACE_GROUP "bleP"

/* Macros to avoid defining "bleP" in .tpp files,
 * as these get included as headers essentially. This would cause
 * redefinitions of TRACE_GROUP in files that include BLE framework headers
 */
#if MBED_TRACE_MAX_LEVEL >= TRACE_LEVEL_DEBUG
#define ble_tr_debug(...)           mbed_tracef(TRACE_LEVEL_DEBUG,   BLE_UTILS_TRACE_GROUP, __VA_ARGS__)   //!< Print debug message
#else
#define ble_tr_debug(...)
#endif

#if MBED_TRACE_MAX_LEVEL >= TRACE_LEVEL_INFO
#define ble_tr_info(...)            mbed_tracef(TRACE_LEVEL_INFO,    BLE_UTILS_TRACE_GROUP, __VA_ARGS__)   //!< Print info message
#else
#define ble_tr_info(...)
#endif

#if MBED_TRACE_MAX_LEVEL >= TRACE_LEVEL_WARN
#define ble_tr_warning(...)         mbed_tracef(TRACE_LEVEL_WARN,    BLE_UTILS_TRACE_GROUP, __VA_ARGS__)   //!< Print warning message
#define ble_tr_warn(...)            mbed_tracef(TRACE_LEVEL_WARN,    BLE_UTILS_TRACE_GROUP, __VA_ARGS__)   //!< Alternative warning message
#else
#define ble_tr_warning(...)
#define ble_tr_warn(...)
#endif

#if MBED_TRACE_MAX_LEVEL >= TRACE_LEVEL_ERROR
#define ble_tr_error(...)           mbed_tracef(TRACE_LEVEL_ERROR,   BLE_UTILS_TRACE_GROUP, __VA_ARGS__)   //!< Print Error Message
#define ble_tr_err(...)             mbed_tracef(TRACE_LEVEL_ERROR,   BLE_UTILS_TRACE_GROUP, __VA_ARGS__)   //!< Alternative error message
#else
#define ble_tr_error(...)
#define ble_tr_err(...)
#endif

/**
 * Log a BLE error
 * @param[in] error Error type to log
 * @param[in] msg Message to prefix error log with
 */
void ble_log_error(ble_error_t error, const char *msg);

/**
 * Log a ble::address_t
 * @param[in] addr Address to log
 */
void ble_log_address(const ble::address_t &addr);

/**
 * Log local device MAC address
 */
void ble_log_local_mac_address(BLE &ble);

/**
 * Print the given address into the given buffer.
 * @param[in] buf Buffer to print address into
 * @param[in] addr Address to print into buf
 *
 * @note buf must be at least 19 chars to ensure the whole address fits
 */
void ble_log_sprintf_address(char *buf, const ble::address_t &addr);

/**
 * Convert a ble::phy_t to its corresponding string representation
 * @param[in] phy Phy to convert
 * @retval String representation of phy
 */
const char* phy_to_string(ble::phy_t phy);

#endif /* _BLE_LOGGING_H_ */
