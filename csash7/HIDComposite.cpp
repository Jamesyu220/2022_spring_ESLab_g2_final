#include "mbed.h"
#include "rtos.h"
#include "ble/BLE.h"
#include "ble/Gap.h"
#include "SecurityManager.h"
#include "events/mbed_events.h"
#include "BLE_HID/HIDServiceBase.h"
#include "ble/services/BatteryService.h"
#include "BLE_HID/HIDDeviceInformationService.h"
#include "HIDComposite.h"

/**
 * This program implements a complete HID-over-Gatt Profile:
 *  - HID is provided by MouseService
 *  - Battery Service
 *  - Device Information Service
 *
 * Complete strings can be sent over BLE using printf. Please note, however, than a 12char string
 * will take about 500ms to transmit, principally because of the limited notification rate in BLE.
 * KeyboardService uses a circular buffer to store the strings to send, and calls to putc will fail
 * once this buffer is full. This will result in partial strings being sent to the client.
 */

rtos::Thread t;

typedef ble_error_t (Gap::*disconnect_call_t)(ble::connection_handle_t, ble::local_disconnection_reason_t);
const static disconnect_call_t disconnect_call = &Gap::disconnect;

void HIDComposite::schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context)
{
    _event_queue.call(mbed::Callback<void()>(&context->ble, &BLE::processEvents));
}

HIDComposite::HIDComposite(BLE &ble) : BLEMouse(ble),
                                BLEKeyboard(ble),
                               _battery_level(50),
                               device_name("Example"),
                               manufacturersName("ARM"),
                               _ble(ble),
                               _event_queue(event_queue),
                               _battery_uuid(GattService::UUID_BATTERY_SERVICE),
                               _battery_service(_ble, _battery_level),
                               batteryService(false),
                               _handle(0),
                               _adv_data_builder(_adv_buffer),
                               ifconnected(false),
                               pos_x(0),
                               pos_y(0)
{
}

void HIDComposite::start()
{
    _ble.onEventsToProcess(
        makeFunctionPointer(this, &HIDComposite::schedule_ble_events));

    _ble.securityManager().setSecurityManagerEventHandler(this);
    _ble.gap().setEventHandler(this);
    _ble.init(this, &HIDComposite::on_init_complete);
    t.start(mbed::callback(&_event_queue, &events::EventQueue::dispatch_forever));


}

void HIDComposite::mouse_click(uint8_t b)
{
    BLEMouse::click(b);
}

void HIDComposite::mouse_move(signed char x, signed char y, signed char wheel)
{
    BLEMouse::move(x, y, wheel);
    // pos_x += x;
    // pos_y += y;
}

void HIDComposite::mouse_g28()
{
    for (int i = 0; i < 4; i++){
        BLEMouse::move(-100, -100);
        ThisThread::sleep_for(10ms);
    }
    pos_x = 0;
    pos_y = 0;
    ThisThread::sleep_for(50ms);
}

void HIDComposite::mouse_move_to(int x, int y)
{
    int x_rel = x - pos_x;
    int y_rel = y - pos_y;
    signed char x_rel_part, y_rel_part;
    while (x_rel != 0 || y_rel != 0){
        if (x_rel > 100){
            x_rel_part = 100;
            x_rel -= 100;
        }
        else if (x_rel < -100){
            x_rel_part = -100;
            x_rel += 100;
        }
        else{
            x_rel_part = x_rel;
            x_rel = 0;
        }
        if (y_rel > 100){
            y_rel_part = 100;
            y_rel -= 100;
        }
        else if (y_rel < -100){
            y_rel_part = -100;
            y_rel += 100;
        }
        else{
            y_rel_part = y_rel;
            y_rel = 0;
        }
        BLEMouse::move(x_rel_part, y_rel_part);
        pos_x += x_rel_part;
        pos_y += y_rel_part;
        ThisThread::sleep_for(30ms);
    }
    printf("pos: %d, %d\n", pos_x, pos_y);
}

void HIDComposite::mouse_press(uint8_t b)
{
    BLEMouse::press(b);
}

void HIDComposite::mouse_release(uint8_t b)
{
    BLEMouse::release(b);
}

bool HIDComposite::mouse_isPressed(uint8_t b)
{
    return BLEMouse::isPressed(b);
}

size_t HIDComposite::keyboard_write(uint8_t k)
{
    return BLEKeyboard::write(k);
}
size_t HIDComposite::keyboard_write(const MediaKeyReport c)
{
    return BLEKeyboard::write(c);
}
size_t HIDComposite::keyboard_write(const uint8_t *buffer, size_t size)
{
    return BLEKeyboard::write(buffer, size);
}

size_t HIDComposite::keyboard_press(uint8_t k)
{
    return BLEKeyboard::press(k);
}
size_t HIDComposite::keyboard_press(const MediaKeyReport k)
{
    return BLEKeyboard::press(k);
}

size_t HIDComposite::keyboard_release(uint8_t keycode)
{
    return BLEKeyboard::release(keycode);
}
size_t HIDComposite::keyboard_release(const MediaKeyReport k)
{
    return BLEKeyboard::release(k);
}

void HIDComposite::keyboard_releaseAll()
{
    return BLEKeyboard::releaseAll();
}

bool HIDComposite::isConnected()
{
    return ifconnected;
}

void HIDComposite::setManufacturerName(const char *manufacturersName2)
{
    manufacturersName = manufacturersName2;
}

void HIDComposite::setDeviceName(const char *device_name2)
{
    device_name = device_name2;
}

void HIDComposite::setBatteryLevel(uint8_t _battery_level)
{
    batteryService = true;
    _battery_service.updateBatteryLevel(_battery_level);
}

void HIDComposite::on_init_complete(BLE::InitializationCompleteCallbackContext *params)
{
    if (params->error != BLE_ERROR_NONE)
    {
        return;
    }

    ble_error_t error;
    /* If the security manager is required this needs to be called before any
     * calls to the Security manager happen. */
    error = _ble.securityManager().init(
        true,
        false,
        SecurityManager::IO_CAPS_NONE,
        NULL,
        false,
        NULL);

    if (error)
    {
        // printf("Error during init %d\r\n", error);
        return;
    }

    error = _ble.securityManager().preserveBondingStateOnReset(true);

    if (error)
    {
        // printf("Error during preserveBondingStateOnReset %d\r\n", error);
    }

    start_advertising();
}

void setManufacturerInfo(BLE &ble, const char *manufacturersName)
{

    PnPID_t pnpID;
    pnpID.vendorID_source = 0x2;   // from the USB Implementer's Forum
    pnpID.vendorID = 0x0D28;       // NXP
    pnpID.productID = 0x0204;      // CMSIS-DAP (well, it's a keyboard but oh well)
    pnpID.productVersion = 0x0100; // v1.0
    HIDDeviceInformationService deviceInfo(ble, manufacturersName, "m1", "abc", "def", "ghi", "jkl", &pnpID);
}

void HIDComposite::start_advertising()
{

    ble::AdvertisingParameters adv_parameters(
        ble::advertising_type_t::CONNECTABLE_UNDIRECTED,
        ble::adv_interval_t(ble::millisecond_t(30)));
    adv_parameters.setTxPower(ble::advertising_power_t(-10));
    setManufacturerInfo(_ble, manufacturersName);

    const uint8_t _vendor_specific_data[4] = {0xAD, 0xDE, 0xBE, 0xEF};
    _adv_data_builder.setManufacturerSpecificData(_vendor_specific_data);
    _ble.gap().setAdvertisingScanResponse(
        ble::LEGACY_ADVERTISING_HANDLE,
        _adv_data_builder.getAdvertisingData());
    _adv_data_builder.clear();

    _adv_data_builder.setFlags();
    _adv_data_builder.setName(device_name);
    _adv_data_builder.setAppearance(ble::adv_data_appearance_t::MOUSE);
    if (batteryService)
    {
        _adv_data_builder.setLocalServiceList(mbed::make_Span(&_battery_uuid, 1));
    }

    ble_error_t error = _ble.gap().setAdvertisingParameters(
        ble::advertising_type_t::CONNECTABLE_UNDIRECTED,
        adv_parameters);

    if (error)
    {
        // print_error(error, "_ble.gap().setAdvertisingParameters() failed");
        return;
    }

    error = _ble.gap().setAdvertisingPayload(
        ble::LEGACY_ADVERTISING_HANDLE,
        _adv_data_builder.getAdvertisingData());

    if (error)
    {
        // print_error(error, "_ble.gap().setAdvertisingPayload() failed");
        return;
    }

    /* Start advertising */

    error = _ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);

    if (error)
    {
        // print_error(error, "_ble.gap().startAdvertising() failed");
        return;
    }

    _ble.securityManager().setPairingRequestAuthorisation(false);
}

void HIDComposite::onDisconnectionComplete(const ble::DisconnectionCompleteEvent &)
{
    ifconnected = false;
    _ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
    _ble.securityManager().setPairingRequestAuthorisation(false);
}

void HIDComposite::onConnectionComplete(const ble::ConnectionCompleteEvent &event)
{
    ifconnected = true;

    ble_error_t error;

    _handle = event.getConnectionHandle();

    error = _ble.securityManager().setLinkSecurity(
        _handle,
        SecurityManager::SECURITY_MODE_ENCRYPTION_NO_MITM);

    if (error)
    {
        // printf("Error during SM::setLinkSecurity %d\r\n", error);
        return;
    }
}

void HIDComposite::pairingRequest(
    ble::connection_handle_t connectionHandle)
{
    // printf("Pairing requested - authorising\r\n");
    _ble.securityManager().acceptPairingRequest(connectionHandle);
}

void HIDComposite::pairingResult(
    ble::connection_handle_t connectionHandle,
    SecurityManager::SecurityCompletionStatus_t result)
{
    if (result == SecurityManager::SEC_STATUS_SUCCESS)
    {
        // printf("Pairing successful\r\n");
    }
    else
    {
        // printf("Pairing failed\r\n");
    }
}

void HIDComposite::begin()
{
    HIDComposite::start();
}
