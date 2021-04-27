/* mbed Microcontroller Library
 * Copyright (c) 2018 ARM Limited
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
 */

#ifndef PRETTY_PRINTER_H_
#define PRETTY_PRINTER_H_

#include <mbed.h>
#include "ble/BLE.h"

#include "platform/mbed_toolchain.h"

#warning "pretty_printer.h has been replaced with ble_logging.h. Please include that file instead."
#include "ble_logging.h"

MBED_DEPRECATED("print_error is deprecated. Use ble_log_error instead.")
inline void print_error(ble_error_t error, const char* msg)
{
    ble_log_error(error, msg);
}

/** print device address to the terminal */
MBED_DEPRECATED("print_address is deprecated. Use ble_log_address instead")
inline void print_address(const ble::address_t &addr)
{
    ble_log_address(addr);
}

MBED_DEPRECATED("print_mac_address is deprecated. Use ble_log_local_mac_address instead")
inline void print_mac_address()
{
    ble_log_local_mac_address();
}

#endif /* PRETTY_PRINTER_H_ */
