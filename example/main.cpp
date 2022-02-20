/* mbed Microcontroller Library
 * Copyright (c) 2006-2022 ARM Limited
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

#include "mbed.h"

#include "ble_app.h"

// Handles for button and led and connection
uint8_t btnvaluehandle = 0;
uint8_t ledvaluehandle = 0;
uint8_t connectionhandle = 0;

// Initial values for button and led
uint8_t btnvalue = 0;
uint8_t prev_btnvalue = btnvalue;
uint8_t ledvalue = 0;

// Initialise the digital pin LED1 as an output
DigitalOut led(LED1);

// Initialise the button pin 
DigitalIn button(BUTTON1, PullUp);

Ticker BTN_ticker;

BLEApp app;

void BTN_tickerhandler()
{
    btnvalue = !button.read();

    if (btnvalue != prev_btnvalue) {
        app.updateCharacteristicValue(btnvaluehandle, btnvalue, sizeof(btnvalue));
    }    
    prev_btnvalue = btnvalue;
}



void bleApp_InitCompletehandler(BLE &ble, events::EventQueue &_event)
{
    printf("Setting up 2M PHY\r\n");
    /* setup the default phy used in connection to 2M to reduce power consumption */
    if (ble.gap().isFeatureSupported(ble::controller_supported_features_t::LE_2M_PHY)) {
        ble::phy_set_t phys(/* 1M */ false, /* 2M */ true, /* coded */ false);
        ble_error_t error = ble.gap().setPreferredPhys(/* tx */&phys, /* rx */&phys);
        
        /* PHY 2M communication will only take place if both peers support it */
        if (error) {
            print_error(error, "GAP::setPreferedPhys failed\r\n");
        }
    } else {
        /* otherwise it will use 1M by default */
        printf("2M not supported. Sticking with 1M PHY\r\n");
    }
}

void bleApp_Connectionhandler(BLE &ble, events::EventQueue &_event, const ble::ConnectionCompleteEvent &event)
{
    connectionhandle = event.getConnectionHandle();

    printf("Callback alert for Connection handle %u. Now connected to: ", connectionhandle);
    print_address(event.getPeerAddress());

    // Initialise a ticker to monitor button status
    BTN_ticker.attach(&BTN_tickerhandler, 250ms);

}

void bleApp_Disconnectionhandler(BLE &ble, events::EventQueue &_event, const ble::DisconnectionCompleteEvent &params)
{
    printf("Callback alert following Disconnection event.\r\n");

    // Detach ticker
    BTN_ticker.detach(); 
}

void bleApp_UpdatesEnabledhandler(const GattUpdatesEnabledCallbackParams &params)
{
    printf("Callback alert following Updates Enabled event.\r\n");
    
}

void bleApp_UpdatesDisabledhandler(const GattUpdatesDisabledCallbackParams &params)
{
    printf("Callback alert following Updates Disabled event.\r\n");
    
}

void bleApp_WriteEventhandler(const GattWriteCallbackParams &params)
{
    printf("Write Event callback alert via connection handle %u.\r\n", params.connHandle);
    if (params.handle == ledvaluehandle) {
        ledvalue = params.data[0];
        printf("Update LED to %s\r\n", ledvalue == 1 ? "ON" : "OFF");
        led = !ledvalue;
    }
    
}

void bleApp_ReadEventhandler(const GattReadCallbackParams &params)
{
    printf("Read Event callback alert via connection handle %u.\r\n", params.connHandle);
    if (params.handle == ledvaluehandle) printf("LED characteristic data read: %u\r\n", params.data[0]);
    else if (params.handle == btnvaluehandle) printf("BTN characteristic data read: %u\r\n", params.data[0]);
    
}

void bleApp_MTUchangehandler(ble::connection_handle_t connectionHandle, uint16_t attMtuSize)
{
    printf("MTU change alert.\r\n");
    printf("connection handle: %u\r\n", connectionHandle);
    printf("New Mtu Size: %u\r\n", attMtuSize);
    
}


int main()
{
    // We declare these after the class definition
    const char *DEVICE_NAME =         "BtnLED";
    // We add the Gatt Service UUID to GattServerProcess as its used to advertise the service
    const char *GATTSERVICE_UUID =    "00001523-1212-efde-1523-785feabcd123";
    // Button and LED Characteristics
    const char *BUTTONCHAR_UUID =     "00001524-1212-efde-1523-785feabcd123";
    const char *LEDCHAR_UUID =        "00001525-1212-efde-1523-785feabcd123";

    printf("nRF52840 Button LED BLE Application\r\n");

    led = !ledvalue;                        // This LED works in reverse
    btnvalue = !button.read();               // Read the button GPIO value (active low so reverse)

    app.set_advertising_name(DEVICE_NAME);
    app.set_GattUUID_128(GATTSERVICE_UUID);

    // Create our Gatt Service Profile
    // For Button Characteristic, we add in an additional notification property
    ReadOnlyGattCharacteristic<uint8_t> btn_characteristic(UUID(BUTTONCHAR_UUID), &btnvalue, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
    ReadWriteGattCharacteristic<uint8_t> led_characteristic(UUID(LEDCHAR_UUID), &ledvalue);

    GattCharacteristic *charTable[] = { &btn_characteristic, & led_characteristic };
    GattService BLS_GattService(UUID(GATTSERVICE_UUID), charTable, sizeof(charTable) / sizeof(charTable[0]));
    
    // We now add in our button & led service
    app.add_new_gatt_service(BLS_GattService);

    // We set up all our optional Gatt Server event handlers   
    app.on_connect(bleApp_Connectionhandler);
    app.on_disconnect(bleApp_Disconnectionhandler);
    app.on_updatesenabled(bleApp_UpdatesEnabledhandler);
    app.on_updatesdisabled(bleApp_UpdatesDisabledhandler);
    app.on_serverwriteevent(bleApp_WriteEventhandler);
    app.on_serverreadevent(bleApp_ReadEventhandler);
    app.on_AttMtuChange(bleApp_MTUchangehandler);

    btnvaluehandle = btn_characteristic.getValueHandle();
    ledvaluehandle = led_characteristic.getValueHandle();

    // We start our app and need to include a BLE Initialise Complete handler
    app.start(bleApp_InitCompletehandler);

    while (true) {
        ThisThread::sleep_for(1s);
    }
}
