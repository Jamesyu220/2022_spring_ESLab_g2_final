#include "mbed.h"
#include "ble/BLE.h"
#include "ble/services/BatteryService.h"
#include "events/mbed_events.h"
#include "BLE_HID/mouse.h"
#include "BLE_HID/keyboard.h"

static events::EventQueue event_queue(/* event count */ 16 * EVENTS_EVENT_SIZE);


class HIDComposite : public ble::Gap::EventHandler,
                 public SecurityManager::EventHandler,
                 public BLEMouse,
                 public BLEKeyboard
{

public:
    BLE &_ble;
    HIDComposite(BLE &ble = BLE::Instance());
    void begin();

    void mouse_click(uint8_t b = MOUSE_BUTTON_LEFT);
    void mouse_move(signed char x, signed char y, signed char wheel = 0);
    void mouse_press(uint8_t b = MOUSE_BUTTON_LEFT);
    void mouse_release(uint8_t b = MOUSE_BUTTON_LEFT);
    bool mouse_isPressed(uint8_t b = MOUSE_BUTTON_LEFT);
    void mouse_move_to(int x, int y);
    void mouse_g28();

    // keyboard functions
    size_t keyboard_write(uint8_t k);
    size_t keyboard_write(const MediaKeyReport c);
    size_t keyboard_write(const uint8_t *buffer, size_t size);

    size_t keyboard_press(uint8_t k);
    size_t keyboard_press(const MediaKeyReport k);

    size_t keyboard_release(uint8_t keycode);
    size_t keyboard_release(const MediaKeyReport k);

    void keyboard_releaseAll();

    // device info
    bool isConnected(void);
    void setManufacturerName(const char *manufacturersName2);
    void setDeviceName(const char *device_name2);
    void setBatteryLevel(uint8_t _battery_level);

private:

    void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context);
    events::EventQueue &_event_queue = event_queue;

    uint8_t _battery_level;
    const char *manufacturersName;
    const char *device_name;

    UUID _battery_uuid;
    BatteryService _battery_service;
	
    bool batteryService;
    uint8_t _adv_buffer[ble::LEGACY_ADVERTISING_MAX_SIZE];
    ble::AdvertisingDataBuilder _adv_data_builder;
    ble::connection_handle_t _handle;
    bool ifconnected;

    int pos_x, pos_y;

private:
    void start(void);
    void on_init_complete(BLE::InitializationCompleteCallbackContext *params);
    void start_advertising();
    virtual void onDisconnectionComplete(const ble::DisconnectionCompleteEvent&);
    virtual void onConnectionComplete(const ble::ConnectionCompleteEvent &event);
    virtual void pairingResult(
        ble::connection_handle_t connectionHandle,
        SecurityManager::SecurityCompletionStatus_t result
    );
	int _dispatch_event;
    virtual void pairingRequest(
        ble::connection_handle_t connectionHandle
    );

};
