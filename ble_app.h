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

#ifndef BLE_APP_H_
#define BLE_APP_H_

#include "pretty_printer.h"
#include "ble/BLE.h"
#include "ChainableGapEventHandler.h"
#include "events/mbed_events.h"
#include "platform/Callback.h"
#include "platform/NonCopyable.h"

static const uint16_t MAX_ADVERTISING_PAYLOAD_SIZE = 50;

/**
 * This is a simplified app that handles running a BLE process for you. This will initialise the instance
 * and handle the event queue.
 *
 * Use add_gap_event_handler() to get notified of gap events like connections.
 * Use set_advertising_name to enable advertising under the given name. Use nullptr to disable advertising.
 * Use set_target_name to enable scanning and attempt to connect to a device with the given name.
 * Use nullptr to stop the scan.
 *
 * Use the start() method to start your application. This call will block and continue execution in the given
 * callback.
 * Use the stop() method to end the BLE process. This will stop servicing the event queue and shutdown
 * the BLE instance. This will cause the start() method that started it to return.
 *
 */
class BLEApp : private mbed::NonCopyable<BLEApp>, public ble::Gap::EventHandler
{
public:
    /**
     * Construct a BLEApp from a BLE instance.
     * Call start() to initiate ble processing.
     */
    BLEApp() : _ble(BLE::Instance())
    {
    }

    ~BLEApp()
    {
        stop();
    }

    /**
     * Initialize the ble interface, this will call the callback at the end of init.
     *
     * @param post_init_cb Callback that will be called on init complete.
     */
    void start(mbed::Callback<void(BLE&, events::EventQueue&)> post_init_cb)
    {
        printf("Ble App started\r\n");

        _post_init_cb = post_init_cb;

        if (_ble.hasInitialized()) {
            printf("Error: the ble instance has already been initialized.\r\n");
            return;
        }

        /* Register the BLEApp as the handler for gap events */
        _gap_handler.addEventHandler(this);
        _ble.gap().setEventHandler(&_gap_handler);

        /* This will inform us of all events so we can schedule their handling
         * using our event queue */
        _ble.onEventsToProcess(
            makeFunctionPointer(this, &BLEApp::schedule_ble_events)
        );

        ble_error_t error = _ble.init(
            this, &BLEApp::on_init_complete
        );

        if (error) {
            print_error(error, "Error returned by BLE::init.\r\n");
            return;
        }

        /* Process the event queue. */
        _event_queue.dispatch_forever();
        /* run left over events */
        _event_queue.dispatch_once();

        return;
    }

    /**
     * Close advertising and/or existing connections and stop the App.
     */
    void stop()
    {
        /* let any left over events run */
        _event_queue.call([this]() {
            if (_ble.hasInitialized()) {
                _ble.shutdown();
                printf("Ble App stopped.\r\n");
            }
            _event_queue.break_dispatch();

            _connected = false;
            _is_connecting = false;
            _is_scanning = false;
            _gap_handler = ChainableGapEventHandler();
        });
    }

    /**
     * Subscribe with your own gap handler.
     *
     * @param[in] gap_handler Handler implementing selected ble::Gap::EventHandler methods.
     *
     * @returns True on success.
     */
    bool add_gap_event_handler(ble::Gap::EventHandler *gap_handler)
    {
        return _gap_handler.addEventHandler(gap_handler);
    }

    /** Set name we advertise as. */
    bool set_advertising_name(const char *advertising_name)
    {
        char* new_name = nullptr;
        size_t length = strlen(advertising_name) + 1;

        if (advertising_name && length) {
            new_name = (char*)malloc(length);
            if (!new_name) {
                return false;
            }
            memcpy(new_name, advertising_name, length);
        }

        _event_queue.call([this,new_name]() {
            delete _advertising_name;
            _advertising_name = new_name;
            _event_queue.call([this]() { start_activity(); });
        });

        return true;
    }

    /** Set name we want to connect to. */
    bool set_target_name(const char *target_name)
    {
        char* new_name = nullptr;
        size_t length = strlen(target_name) + 1;

        if (target_name && length) {
            new_name = (char*)malloc(length);
            if (!new_name) {
                return false;
            }
            memcpy(new_name, target_name, length);
        }

        _event_queue.call([this,new_name]() {
            delete _target_name;
            _target_name = new_name;
            _event_queue.call([this]() { start_activity(); });
        });

        return true;
    }

    /** Get name we advertise as if set, otherwise returns nullptr. */
    const char* get_advertising_name() const
    {
        return _advertising_name;
    }

    /** Get name we coonect to if set, otherwise returns nullptr. */
    const char* get_target_name() const
    {
        return _target_name;
    }

protected:
    /**
     * Sets up adverting payload and start advertising.
     * This function is invoked when the ble interface is initialized.
     */
    void on_init_complete(BLE::InitializationCompleteCallbackContext *event)
    {
        if (event->error) {
            print_error(event->error, "Error during the initialisation\r\n");
            return;
        }

        printf("Ble instance initialized\r\n");

        _event_queue.call([this]() { _post_init_cb(_ble, _event_queue); });

        /* All calls are serialised on the user thread through the event queue */
        _event_queue.call([this]() { start_activity(); });
    }

    /**
     * Start the gatt client process when a connection event is received.
     * This is called by Gap to notify the application we connected
     */
    void onConnectionComplete(const ble::ConnectionCompleteEvent &event) override
    {
        _is_connecting = false;
        if (event.getStatus() == BLE_ERROR_NONE) {
            _connected = true;
            _conn_handle = event.getConnectionHandle();
            printf("Connected to: ");
            print_address(event.getPeerAddress());
        } else {
            printf("Failed to connect\r\n");
            _event_queue.call([this]() { start_activity(); });
        }
    }

    /**
     * Stop the gatt client process when the device is disconnected then restart advertising.
     * This is called by Gap to notify the application we disconnected
     */
    void onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event) override
    {
        if (_connected) {
            _connected = false;
            printf("Disconnected.\r\n");
            _event_queue.call([this]() { start_activity(); });
        }
    }

    /** Restarts main activity */
    void onAdvertisingEnd(const ble::AdvertisingEndEvent &event)
    {
        _event_queue.call([this]() { start_activity(); });
    }

    /**
     * Start advertising or scanning. Triggered by init or disconnection.
     */
    virtual void start_activity()
    {
        if (!_ble.hasInitialized()) {
            return;
        }

        if (_advertising_name) {
            start_advertising();
        } else {
            _ble.gap().stopAdvertising(_adv_handle);
        }

        if (_target_name) {
            start_scanning();
        } else {
            _ble.gap().stopScan();
        }
    }

    /**
     * Start the advertising process; it ends when a device connects.
     */
    void start_advertising()
    {
        ble_error_t error;

        if (!_advertising_name || _ble.gap().isAdvertisingActive(_adv_handle)) {
            /* we're already advertising */
            return;
        }

        ble::AdvertisingParameters adv_params(
            ble::advertising_type_t::CONNECTABLE_UNDIRECTED,
            ble::adv_interval_t(ble::millisecond_t(40))
        );

        error = _ble.gap().setAdvertisingParameters(_adv_handle, adv_params);

        if (error) {
            printf("_ble.gap().setAdvertisingParameters() failed\r\n");
            return;
        }

        uint8_t adv_buffer[MAX_ADVERTISING_PAYLOAD_SIZE];
        ble::AdvertisingDataBuilder adv_data_builder(adv_buffer);
        
        adv_data_builder.clear();
        adv_data_builder.setFlags();
        error = adv_data_builder.setName(_advertising_name);

        if (error) {
            print_error(error, "AdvertisingDataBuilder::setName() failed (name too long?)\r\n");
            return;
        }

        /* Set payload for the set */
        error = _ble.gap().setAdvertisingPayload(
            _adv_handle, adv_data_builder.getAdvertisingData()
        );

        if (error) {
            print_error(error, "Gap::setAdvertisingPayload() failed\r\n");
            return;
        }

        error = _ble.gap().startAdvertising(_adv_handle, ble::adv_duration_t(ble::second_t(10)));

        if (error) {
            print_error(error, "Gap::startAdvertising() failed\r\n");
            return;
        }

        printf("Advertising as \"%s\"\r\n", _advertising_name);
    }

    /** scan for GattServer */
    void start_scanning()
    {
        if (_is_scanning || _connected || !_target_name) {
            /* already connected or scan not needed */
            return;
        }

        ble::ScanParameters scan_params;
        scan_params.set1mPhyConfiguration(ble::scan_interval_t(80), ble::scan_window_t(40), false);
        _ble.gap().setScanParameters(scan_params);

        ble_error_t ret = _ble.gap().startScan(ble::scan_duration_t(ble::second_t(10)));

        if (ret == ble_error_t::BLE_ERROR_NONE) {
            _is_scanning = true;
            printf("Started scanning for \"%s\"\r\n", _target_name);
        } else {
            printf("Starting scan failed\r\n");
        }
    }

    /** Restarts main activity */
    void onScanTimeout(const ble::ScanTimeoutEvent &event) override {
        _is_scanning = false;
        _event_queue.call([this]() { start_activity(); });
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
                if (field.value.size() == strlen(_target_name) &&
                    (memcmp(field.value.data(), _target_name, field.value.size()) == 0)) {

                    printf("We found \"%s\", connecting...\r\n", _target_name);

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

    /**
     * Schedule processing of events from the BLE middleware in the event queue.
     */
    void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *event)
    {
        _event_queue.call(mbed::callback(&event->ble, &BLE::processEvents));
    }

protected:
    events::EventQueue _event_queue;
    BLE &_ble;

    char *_advertising_name = nullptr;
    char *_target_name = nullptr;
    
    ble::advertising_handle_t _adv_handle = ble::LEGACY_ADVERTISING_HANDLE;

    ble::connection_handle_t _conn_handle;
    bool _connected = false;
    bool _is_connecting = false;
    bool _is_scanning = false;

    mbed::Callback<void(BLE&, events::EventQueue&)> _post_init_cb;
    ChainableGapEventHandler _gap_handler;
};

#endif /* BLE_APP_H_ */
