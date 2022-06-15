#include "mbed.h"
#include "./csash7/HIDMouse.h"

int main()
{
  BLE &ble = BLE::Instance();
  HIDMouse bleMouse(ble);
  bleMouse.setDeviceName("Universal_Joystick");
  bleMouse.setManufacturerName("ESLab_Team_2");
  bleMouse.setBatteryLevel(50);
  bleMouse.begin();

    int16_t x = 0;
    int16_t y = 0;
    int32_t radius = 10;
    int32_t angle = 0;

    while (1) {
        //will cause mouse to move in a circle
        x = cos((double)angle * 3.14 / 180.0) * radius;
        y = sin((double)angle * 3.14 / 180.0) * radius;

        //will move mouse x, y away from its previous position on the screen
        bleMouse.move(x, y);
        angle += 3;
        ThisThread::sleep_for(10);
    }
    
  return 0;
}