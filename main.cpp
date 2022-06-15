#include "./csash7/HIDMouse.h"
#include "mbed.h"
#include "Mysensor.h"

static void MX_GPIO_Init(gpio_t &, gpio_t &, gpio_t &, gpio_t &, gpio_t &);
AnalogIn x(A4);
AnalogIn y(A5);
// main() runs in its own thread in the OS

int main() {
  BLE &ble = BLE::Instance();
  HIDMouse mouse(ble);
  mouse.setDeviceName("Universal_Joystick");
  mouse.setManufacturerName("ESLab_Team_2");
  mouse.setBatteryLevel(50);
  mouse.begin();
  gpio_t *big_black, *small_black, *yellow, *green, *sw;
  big_black = new gpio_t;
  small_black = new gpio_t;
  yellow = new gpio_t;
  green = new gpio_t;
  sw = new gpio_t;
  int bb, sb, ye, gr, s;
  int p_bb, p_sb, p_ye, p_gr, p_s;
  bool start = false;

  uint16_t x_pos = 0x3000;
  uint16_t y_pos = 0;

  MX_GPIO_Init(*big_black, *small_black, *yellow, *green, *sw);
  // unsigned long long int i = 0;
  while (true) {
    //++i;

    x_pos = 0;
    y_pos = 0;
    bb = gpio_read(big_black);
    sb = gpio_read(small_black);
    ye = gpio_read(yellow);
    gr = gpio_read(green);
    s = gpio_read(sw);
    if (!start) {
      p_bb = bb;
      p_sb = sb;
      p_ye = ye;
      p_gr = gr;
      p_s = s;
      start = true;
    }

    // big black
    if (p_bb && !bb) {
      printf("press big black \n");
    } else if (!p_bb && bb) {
      printf("release big black \n");
    }
    // small black
    if (!p_sb && sb) {
      printf("press small black \n");
    } else if (p_sb && !sb) {
      printf("release small black \n");
    }
    // yellow
    if (!p_ye && ye) {
      printf("press yellow \n");
    } else if (p_ye && !ye) {
      printf("release yellow \n");
    }
    // green
    if (!p_gr && gr) {
      printf("press green \n");
    } else if (p_gr && !gr) {
      printf("release green \n");
    }

    // sw
    if (p_s && !s) {
      printf("press sw \n");
    } else if (!p_s && s) {
      printf("release sw \n");
    }

    p_bb = bb;
    p_sb = sb;
    p_ye = ye;
    p_gr = gr;
    p_s = s;

    // x.set_reference_voltage(5.0f);
    if (x.read() > 0.6f) {
      printf("back \n");
      y_pos += 10;
    } else if (x.read() < 0.4f) {
      printf("go \n");
      y_pos -= 10;
    }

    if (y.read() > 0.6f) {
      printf("left \n");
      x_pos -= 10;
    } else if (y.read() < 0.4f) {
      printf("right \n");
      x_pos += 10;
    }
    int gyro_output, acc_output;
    Mysensor(&gyro_output, &acc_output);
    if (gyro_output & GYRO_Z_CLK) {
      x_pos += 10;
    } else if (gyro_output & GYRO_Z_CCLK) {
      x_pos -= 10;
    } else {
      ;
    }

    if (gyro_output & GYRO_Y_R) {
      x_pos += 10;
    } else if (gyro_output & GYRO_Y_L) {
      x_pos -= 10;
    } else {
      ;
    }

    if (gyro_output & GYRO_X_B) {
      y_pos -= 10;
    } else if (gyro_output & GYRO_X_F) {
      y_pos += 10;
    } else {
      ;
    }

    // if(acc_output & ACC_Z_U){
    //     y_pos -= 10;
    // }else if(acc_output & ACC_Z_D){
    //     y_pos += 10;
    // }else{
    //     ;
    // }

    // if(acc_output & ACC_Y_F){
    //     printf("move front, ");
    // }else if(acc_output & ACC_Y_B){
    //     printf("move back, ");
    // }else{
    //     printf("no y, ");
    // }

    // if(acc_output & ACC_X_R){
    //     x_pos += 10;
    // }else if(acc_output & ACC_X_L){
    //     x_pos -= 10;
    // }else{
    //    ;
    // }

    mouse.move(x_pos, y_pos);
    ThisThread::sleep_for(30ms);
  }
}

/** Configure pins as
 * Analog
 * Input
 * Output
 * EVENT_OUT
 * EXTI
 */
static void MX_GPIO_Init(gpio_t &big_black, gpio_t &small_black, gpio_t &yellow,
                         gpio_t &green, gpio_t &sw) {

  gpio_set(PA_7);
  gpio_init(&big_black, PA_7);
  gpio_dir(&big_black, PIN_INPUT);
  gpio_mode(&big_black, PullNone);

  gpio_set(PA_2);
  gpio_init(&green, PA_2);
  gpio_dir(&green, PIN_INPUT);
  gpio_mode(&green, PullNone);

  gpio_set(PA_15);
  gpio_init(&yellow, PA_15);
  gpio_dir(&yellow, PIN_INPUT);
  gpio_mode(&yellow, PullNone);

  gpio_set(PB_2);
  gpio_init(&small_black, PB_2);
  gpio_dir(&small_black, PIN_INPUT);
  gpio_mode(&small_black, PullNone);

  gpio_set(PA_1);
  gpio_init(&sw, PA_1);
  gpio_dir(&sw, PIN_INPUT);
  gpio_mode(&sw, PullUp);
}