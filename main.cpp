// #include "./csash7/HIDComposite.h"
#include "mbed.h"
#include "Mysensor.h"
#include "USBMouse.h"

static void MX_GPIO_Init(gpio_t &, gpio_t &, gpio_t &, gpio_t &, gpio_t &);
AnalogIn x(A4);
AnalogIn y(A5);
// main() runs in its own thread in the OS

#define FIRE_X 155
#define FIRE_Y 195
#define ANGLE_X 200
#define ANGLE_Y 190
#define POWER_X 200
#define POWER_Y 225
#define WEAPON_X 130
#define WEAPON_Y 225

uint16_t x_pos = 0;
uint16_t y_pos = 0;

USBMouse mouse;

void mouse_g28(){
    for (int i = 0; i < 4; i++){
        mouse.move(-100, -100);
        ThisThread::sleep_for(10ms);
    }
    x_pos = 0;
    y_pos = 0;
    ThisThread::sleep_for(50ms);
}

void mouse_move_to(int x, int y)
{
    int x_rel = x - x_pos;
    int y_rel = y - y_pos;
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
        mouse.move(x_rel_part, y_rel_part);
        x_pos += x_rel_part;
        y_pos += y_rel_part;
        ThisThread::sleep_for(30ms);
    }
    printf("pos: %d, %d\n", x_pos, y_pos);
}



int main() {
    printf("start calibration...\n");
    // data_init();
    printf("start calibration...\n");
//   BLE &ble = BLE::Instance();
//   USBMouse mouse;
  
  gpio_t *big_black, *small_black, *yellow, *green, *sw;
  big_black = new gpio_t;
  small_black = new gpio_t;
  yellow = new gpio_t;
  green = new gpio_t;
  sw = new gpio_t;
  int bb, sb, ye, gr, s;
  int p_bb, p_sb, p_ye, p_gr, p_s;
  bool start = false;

  
  bool is_g28 = false;

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
      mouse_g28();
      ThisThread::sleep_for(30ms);
      mouse_move_to(ANGLE_X, ANGLE_Y);
      mouse.click(MOUSE_LEFT);
    } else if (!p_bb && bb) {
      printf("release big black \n");
    //   mouse.click(MOUSE_LEFT);
    }
    // small black
    if (!p_sb && sb) {
      printf("press small black \n");
      mouse_g28();
      ThisThread::sleep_for(30ms);
      mouse_move_to(FIRE_X, FIRE_Y);
      mouse.click(MOUSE_LEFT);
    } else if (p_sb && !sb) {
      printf("release small black \n");
    }
    // yellow
    if (!p_ye && ye) {
      printf("press yellow \n");
      mouse_g28();
      ThisThread::sleep_for(30ms);
      mouse_move_to(WEAPON_X, WEAPON_Y);
      mouse.click(MOUSE_LEFT);
    } else if (p_ye && !ye) {
    //   mouse.click(MOUSE_LEFT);
      printf("release yellow \n");
    }
    // green
    if (!p_gr && gr) {
      printf("press green \n");
      mouse_g28();
      ThisThread::sleep_for(30ms);
      mouse_move_to(POWER_X, POWER_Y);
      ThisThread::sleep_for(30ms);
      mouse.click(MOUSE_LEFT);
    } else if (p_gr && !gr) {
    //   mouse.click(MOUSE_LEFT);
      ThisThread::sleep_for(30ms);
    //   mouse_g28();
    //   ThisThread::sleep_for(30ms);
    //   mouse_move_to(POWER_X, POWER_Y);
      printf("release green \n");
    }

    // sw
    if (p_s && !s) {
      printf("press sw \n");
      mouse.click(MOUSE_LEFT);
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
      printf("right \n");
      x_pos += 10;
    } else if (x.read() < 0.4f) {
      printf("left \n");
      x_pos -= 10;
    }

    if (y.read() > 0.6f) {
      printf("down \n");
      y_pos += 10;
    } else if (y.read() < 0.4f) {
      printf("up \n");
      y_pos -= 10;
    }
    // int gyro_output, acc_output;
    // Mysensor(&gyro_output, &acc_output);
    // if (gyro_output & GYRO_Z_CLK) {
    //   x_pos += 10;
    // } else if (gyro_output & GYRO_Z_CCLK) {
    //   x_pos -= 10;
    // } else {
    //   ;
    // }

    // if (gyro_output & GYRO_Y_R) {
    //   x_pos += 10;
    // } else if (gyro_output & GYRO_Y_L) {
    //   x_pos -= 10;
    // } else {
    //   ;
    // }

    // if (gyro_output & GYRO_X_B) {
    //   y_pos -= 10;
    // } else if (gyro_output & GYRO_X_F) {
    //   y_pos += 10;
    // } else {
    //   ;
    // }

    // // if(acc_output & ACC_Z_U){
    // //     y_pos -= 10;
    // // }else if(acc_output & ACC_Z_D){
    // //     y_pos += 10;
    // // }else{
    // //     ;
    // // }

    // // if(acc_output & ACC_Y_F){
    // //     printf("move front, ");
    // // }else if(acc_output & ACC_Y_B){
    // //     printf("move back, ");
    // // }else{
    // //     printf("no y, ");
    // // }

    // // if(acc_output & ACC_X_R){
    // //     x_pos += 10;
    // // }else if(acc_output & ACC_X_L){
    // //     x_pos -= 10;
    // // }else{
    // //    ;
    // // }

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
