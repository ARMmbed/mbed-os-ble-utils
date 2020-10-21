/* mbed Microcontroller Library
 * Copyright (c) 2006-2019 ARM Limited
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

#ifndef GATT_SERVER_PROCESS_H_
#define GATT_SERVER_PROCESS_H_

#include "ble_process.h"

/**
 * Simple GattServer wrapper. It will advertise and allow a connection.
 */
class GattServerProcess : public BLEProcess
{
public:
    GattServerProcess(events::EventQueue &event_queue, BLE &ble_interface) :
        BLEProcess(event_queue, ble_interface)
    {
    }

    const char* get_device_name() override
    {
        static const char name[] = "GattServer";
        return name;
    }
};

#endif /* GATT_SERVER_PROCESS_H_ */
