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

#ifndef GATT_CLIENT_PROCESS_H_
#define GATT_CLIENT_PROCESS_H_

#include "ble_process.h"

using namespace std::literals::chrono_literals;

/**
 * Simple GattClient wrapper.
 * It will alternate between advertising and scanning to obtain a connection to GattServer.
 */
class GattClientProcess : public BLEProcess
{
public:
    GattClientProcess(events::EventQueue &event_queue, BLE &ble_interface) :
        BLEProcess(event_queue, ble_interface)
    {
    }

    /** Name we advertise as */
    const char* get_device_name() override
    {
        static const char name[] = "GattClient";
        return name;
    }

    /** Name of device we want to connect to */
    const char* get_peer_device_name()
    {
        static const char name[] = "GattServer";
        return name;
    }

private:
    /** Alternate between scanning and advertising */
    virtual void start_activity()
    {
        static bool scan = true;
        if (scan) {
            _event_queue.call([this]() { start_scanning(); });
        } else {
            _event_queue.call([this]() { start_advertising(); });
        }
        scan = !scan;
        _is_connecting = false;
    }

    /** scan for GattServer */
    void start_scanning()
    {
        ble::ScanParameters scan_params;
        _ble.gap().setScanParameters(scan_params);
        ble_error_t ret = _ble.gap().startScan(ble::scan_duration_t(ble::millisecond_t(4000)));
        if (ret == ble_error_t::BLE_ERROR_NONE) {
            printf("Started scanning for \"%s\"\r\n", get_peer_device_name());
        } else {
            printf("Starting scan failed\r\n");
        }
    }

    /** Restarts main activity */
    void onScanTimeout(const ble::ScanTimeoutEvent &event) override {
        start_activity();
    }

    /** Check advertising report for name and connect to any device with the name GattServer */
    void onAdvertisingReport(const ble::AdvertisingReportEvent &event) override {
        /* don't bother with analysing scan result if we're already connecting */
        if (_is_connecting) {
            return;
        }

        if (!event.getType().connectable()) {
            /* we're only interested in connectable devices */
            return;
        }

        ble::AdvertisingDataParser adv_data(event.getPayload());

        /* parse the advertising payload, looking for a discoverable device */
        while (adv_data.hasNext()) {
            ble::AdvertisingDataParser::element_t field = adv_data.next();

            /* connect to a discoverable device */
            if (field.type == ble::adv_data_type_t::COMPLETE_LOCAL_NAME) {
                if (field.value.size() == strlen(get_peer_device_name()) &&
                    (memcmp(field.value.data(), get_peer_device_name(), field.value.size()) == 0)) {

                    printf("We found \"%s\", connecting...\r\n", get_peer_device_name());

                    ble_error_t error = _ble.gap().stopScan();

                    if (error) {
                        print_error(error, "Error caused by Gap::stopScan");
                        return;
                    }

                    const ble::ConnectionParameters connection_params;

                    error = _ble.gap().connect(
                        event.getPeerAddressType(),
                        event.getPeerAddress(),
                        connection_params
                    );

                    if (error) {
                        _ble.gap().startScan();
                        return;
                    }

                    /* we may have already scan events waiting
                     * to be processed so we need to remember
                     * that we are already connecting and ignore them */
                    _is_connecting = true;

                    return;
                }
            }
        }
    }
private:
    bool _is_connecting = false;
};

#endif /* GATT_CLIENT_PROCESS_H_ */
