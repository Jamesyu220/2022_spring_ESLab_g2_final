#include "mbed.h"
 
// Sensors drivers present in the BSP library
#include "stm32l475e_iot01_tsensor.h"
#include "stm32l475e_iot01_hsensor.h"
#include "stm32l475e_iot01_psensor.h"
#include "stm32l475e_iot01_magneto.h"
#include "stm32l475e_iot01_gyro.h"
#include "stm32l475e_iot01_accelero.h"
#include <cstdint>
#include "Mysensor.h"
 

#define STANDARD_PARAM 0
int acc_is_first = 1;
int gyro_is_first = 1;

DigitalOut led(LED1);
int acc_x_normalize = 30;
int acc_y_normalize = 0;
int acc_z_normalize = 1015; // positive: moving upward

// int acc_x_normalize = 0;
// int acc_y_normalize = 0;
// int acc_z_normalize = 0; // positive: moving upward


float gyro_x_normalize = -210; // positive: front moving downward
float gyro_y_normalize = -700;
float gyro_z_normalize = 1120; // positive: clockwize rotation 
float gyro_scale = 100;
float gyro_threshold = 200;

// #define GYRO_Z_CLK 1
// #define GYRO_Z_CCLK 2
// #define GYRO_Y_R 4
// #define GYRO_Y_L 8
// #define GYRO_X_B 16
// #define GYRO_X_F 32
// #define GYRO_Z_CLK_WEAK 64
// #define GYRO_Z_CCLK_WEAK 128
// #define GYRO_Y_R_WEAK 256
// #define GYRO_Y_L_WEAK 512
// #define GYRO_X_B_WEAK 1024
// #define GYRO_X_F_WEAK 2048
// #define GYRO_Z_CLK_STRONG (1 << 12)
// #define GYRO_Z_CCLK_STRONG (1 << 13)
// #define GYRO_Y_R_STRONG (1 << 14)
// #define GYRO_Y_L_STRONG (1 << 15)
// #define GYRO_X_B_STRONG (1 << 16)
// #define GYRO_X_F_STRONG (1 << 17)

// #define ACC_Z_U 1
// #define ACC_Z_D 2
// #define ACC_Y_F 4
// #define ACC_Y_B 8
// #define ACC_X_R 16
// #define ACC_X_L 32

// #define Z_MASK ~3
// #define Y_MASK ~12
// #define X_MASK ~48

// #define WINDOW_SIZE_ACC 4
// #define WINDOW_SIZE_ACC_SMALL 4

// #define IS_RIGHT_HAND 1
// #define IS_LEFT_HAND 0

// #define WALK_WINDOW_SIZE 10
// #define WALK_WINDOW_TRANSITION 4

int STRONG_TH = 800;
int MID_TH = 200;
int WEAK_TH = 100;

Mutex acc_mutex;
Mutex gyro_mutex;
Mutex sensor_mutex;
Mutex rotation_mutex;
Mutex move_mutex;

int rotation;
int my_move;

Thread t1;
Thread t2;
Thread t3;
Thread t4;
Thread e_thread;

void our_acc(int16_t * acc_data)
{
    BSP_ACCELERO_AccGetXYZ(acc_data);
    acc_data[0] -= acc_x_normalize;
    acc_data[1] -= acc_y_normalize;
    acc_data[2] -= acc_z_normalize;
}

void our_gyro(float* g_data)
{
    BSP_GYRO_GetXYZ(g_data);
    g_data[0] -= gyro_x_normalize;
    g_data[1] -= gyro_y_normalize;
    g_data[2] -= gyro_z_normalize;
    for(int i = 0; i < 3; ++i){
        g_data[i] = g_data[i] / gyro_scale;
    }
}

void calibrate_gyro(void const *name){
    gyro_mutex.lock();
    float g_data[100][3];
    sensor_mutex.lock();
    for(int i = 0; i < 100; ++i)
    {
        BSP_GYRO_GetXYZ(g_data[i]);
        // printf("gyro: (%f, %f, %f) \n", g_data[i][0], g_data[i][1], g_data[i][2]);
    }
    sensor_mutex.unlock();
    
    float c_x = 0;
    float c_y = 0;
    float c_z = 0;
    for(int i = 0; i < 100; ++i)
    {
        // printf("i: %d\n", i);
        
        
        c_x += g_data[i][0];
        c_y += g_data[i][1];
        c_z += g_data[i][2];

    }
    c_x = c_x/100;
    c_y = c_y/100;
    c_z = c_z/100;
    printf("thread %d after gyro calibration: %f, %f, %f\n", *((int*)name), c_x, c_y, c_z);
    gyro_mutex.lock();
    if(gyro_is_first && !STANDARD_PARAM)
        gyro_x_normalize = c_x;
    else
        gyro_x_normalize = (gyro_x_normalize + c_x) / 2;
    if(gyro_is_first && !STANDARD_PARAM)
        gyro_y_normalize = c_y;
    else
        gyro_y_normalize = (gyro_y_normalize + c_y) / 2;
    if(gyro_is_first && !STANDARD_PARAM)
        gyro_z_normalize = c_z;
    else
        gyro_z_normalize = (gyro_z_normalize + c_z) / 2;
    if(gyro_is_first == 1){
        gyro_is_first = 0;
    }
    gyro_mutex.unlock();
}

void calibrate_acc(void const *name){
    // use most frequent number
    int16_t acc_data[100][3];
    sensor_mutex.lock();
    for(int i = 0; i < 100; ++i)
    {
        
        BSP_ACCELERO_AccGetXYZ(acc_data[i]);
        // printf("acc: (%d, %d, %d) \n", acc_data[i][0], acc_data[i][1], acc_data[i][2]);
    }
    sensor_mutex.unlock();
    // printf("fuck \n");
    int c_x = 0;
    int c_y = 0;
    int c_z = 0;
    for(int i = 0; i < 100; ++i)
    {
        // printf("i: %d\n", i);
        // int16_t tmp_x = 0;
        // int16_t tmp_y = 0;
        // int16_t tmp_z = 0;
        
        // if ( i != 0){
        //     if(abs(acc_data[i][0] - acc_data[i-1][0]) <= 50){
        //         tmp_x += acc_data[i][0];
            
        //     }
        //     if(abs(acc_data[i][1] - acc_data[i-1][1]) <= 50){
        //         tmp_y += acc_data[i][1];
        //     }
        //     if(abs(acc_data[i][2] - acc_data[i-1][2]) <= 50){ 
        //         tmp_z += acc_data[i][2];
        //     }
        // }
        
        
        c_x = c_x + (int)acc_data[i][0];
        c_y = c_y + (int)acc_data[i][1];
        c_z = c_z + (int)acc_data[i][2];
        // printf("%d: thread %d adding acc calibration: %d, %d, %d\n", i, *((int*)name), c_x, c_y, c_z);

    }
    printf("thread %d before acc calibration: %d, %d, %d\n", *((int*)name), c_x, c_y, c_z);
    c_x = (int)((float)c_x/100.0);
    c_y = (int)((float)c_y/100.0);
    c_z = (int)((float)c_z/100.0);
    printf("thread %d after acc calibration: %d, %d, %d\n", *((int*)name), c_x, c_y, c_z);
    acc_mutex.lock();
    if(acc_is_first && !STANDARD_PARAM)
        acc_x_normalize = c_x;
    else
        acc_x_normalize = (acc_x_normalize + c_x) / 2;
    if(acc_is_first && !STANDARD_PARAM)
        acc_y_normalize = c_y;
    else
        acc_y_normalize = (acc_y_normalize + c_y) / 2;
    if(acc_is_first && !STANDARD_PARAM)
        acc_z_normalize = c_z;
    else
        acc_z_normalize = (acc_z_normalize + c_z) / 2;
    if(acc_is_first == 1){
        acc_is_first = 0;
    }
    // acc_y_normalize = (acc_y_normalize + c_y) / 2;
    // acc_z_normalize = (acc_z_normalize + c_z) / 2;
    acc_mutex.unlock();
}

// void calibrate_gyro_and_acc(){
//     calibrate_gyro();
//     calibrate_acc();
// }

void data_init(){
    //     float sensor_value = 0;
    int16_t pDataXYZ[3] = {0};
    float pGyroDataXYZ[3] = {0};

    printf("Start sensor init\n");

    // int buffer_count_f = 0;
    // int buffer_count_b = 0;
    // int buffer_count_u = 0;
    // int buffer_count_d = 0;
    // int buffer_count_l = 0;
    // int buffer_count_r = 0;
    BSP_MAGNETO_Init();
    BSP_GYRO_Init();
    BSP_ACCELERO_Init();
    const int a1 = 1;
    const int a2 = 2;
    const int a3 = 3;
    const int a4 = 4;
    printf("acc normalization(before): %d, %d, %d\n", acc_x_normalize, acc_y_normalize, acc_z_normalize);
    printf("gyro normalization(before): %f, %f, %f\n", gyro_x_normalize, gyro_y_normalize, gyro_z_normalize);
    t1.start(callback(calibrate_acc, (void *)&a1));
    t2.start(callback(calibrate_acc, (void *)&a2));
    t3.start(callback(calibrate_gyro, (void *)&a3));
    t4.start(callback(calibrate_gyro, (void *)&a4));
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    printf("acc normalization: %d, %d, %d\n", acc_x_normalize, acc_y_normalize, acc_z_normalize);
    printf("gyro normalization: %f, %f, %f\n", gyro_x_normalize, gyro_y_normalize, gyro_z_normalize);
}
 
int Mysensor(int *rotation, int *move)
{
    static int walk_status[WALK_WINDOW_SIZE] = {0};
    static int16_t pDataXYZ[3] = {0};
    static float pGyroDataXYZ[3] = {0};
    static int gyro_output;
    static int acc_output;
    static int acc_state_counter_z;
    static int acc_state_counter_y;
    static int acc_state_counter_x;
    static long count = 0;
 
    // printf("Start sensor init\n");

 
    // BSP_MAGNETO_Init();
    // BSP_GYRO_Init();
    // BSP_ACCELERO_Init();
    
 
 
        
    gyro_output = 0;
    our_gyro(pGyroDataXYZ);
    if(pGyroDataXYZ[2] > 200){
        gyro_output |= GYRO_Z_CLK;
        count += 1;
    }else if(pGyroDataXYZ[2] < -200){
        gyro_output |= GYRO_Z_CCLK;
        count -= 1;
    }if(pGyroDataXYZ[2] > 100){
        gyro_output |= GYRO_Z_CLK_WEAK;
        count += 1;
    }else if(pGyroDataXYZ[2] < -100){
        gyro_output |= GYRO_Z_CCLK_WEAK;
        count -= 1;
    }if(pGyroDataXYZ[2] > 500){
        gyro_output |= GYRO_Z_CLK_STRONG;
        count += 1;
    }else if(pGyroDataXYZ[2] < -500){
        gyro_output |= GYRO_Z_CCLK_STRONG;
        count -= 1;
    }

    if(pGyroDataXYZ[1] > 200){
        gyro_output |= GYRO_Y_L;
        count += 1;
    }else if(pGyroDataXYZ[1] < -200){
        gyro_output |= GYRO_Y_R;
        count -= 1;
    }if(pGyroDataXYZ[1] > 100){
        gyro_output |= GYRO_Y_L_WEAK;
        count += 1;
    }else if(pGyroDataXYZ[1] < -100){
        gyro_output |= GYRO_Y_R_WEAK;
        count -= 1;
    }if(pGyroDataXYZ[1] > 500){
        gyro_output |= GYRO_Y_L_STRONG;
        count += 1;
    }else if(pGyroDataXYZ[1] < -500){
        gyro_output |= GYRO_Y_R_STRONG;
        count -= 1;
    }

    if(pGyroDataXYZ[0] > 200){
        gyro_output |= GYRO_X_B;
        count += 1;
    }else if(pGyroDataXYZ[0] < -200){
        gyro_output |= GYRO_X_F;
        count -= 1;
    }if(pGyroDataXYZ[0] > 100){
        gyro_output |= GYRO_X_B_WEAK;
        count += 1;
    }else if(pGyroDataXYZ[0] < -100){
        gyro_output |= GYRO_X_F_WEAK;
        count -= 1;
    }if(pGyroDataXYZ[0] > 500){
        gyro_output |= GYRO_X_B_STRONG;
        count += 1;
    }else if(pGyroDataXYZ[0] < -500){
        gyro_output |= GYRO_X_F_STRONG;
        count -= 1;
    }
    *rotation = gyro_output;
    // ////////////////////// print status
    // printf("gyro status: ");
    // if(gyro_output & GYRO_Z_CLK){
    //     printf("clockwise, ");
    // }else if(gyro_output & GYRO_Z_CCLK){
    //     printf("counter clockwise, ");
    // }else{
    //     printf("no z, ");
    // }

    // if(gyro_output & GYRO_Y_R){
    //     printf("right tilted, ");
    // }else if(gyro_output & GYRO_Y_L){
    //     printf("left tilted, ");
    // }else{
    //     printf("no y, ");
    // }

    // if(gyro_output & GYRO_X_B){
    //     printf("backwward, ");
    // }else if(gyro_output & GYRO_X_F){
    //     printf("frontward, ");
    // }else{
    //     printf("no x, ");
    // }
    // printf("\n");
    ////////////////////////////////////////////

    ////////////// checking walk state ////////////
    // for(int i = 0; i < WALK_WINDOW_SIZE-1; ++i){
    //     walk_status[i] = walk_status[i+1];
    // }
    // walk_status[WALK_WINDOW_SIZE-1] = gyro_output & (GYRO_Y_L_STRONG|GYRO_Y_R_STRONG);
    // int walk_count;
    // walk_count = 0;
    // for(int i = 0; i < WALK_WINDOW_SIZE-1; ++i){
    //     if(walk_status[i] != walk_status[i+1]) walk_count += 1;
    // }
    // if(walk_count > WALK_WINDOW_TRANSITION){
    //     printf("the man is walking!!!\n");
    //     return 0;
    // }
    //////////////////////////////////////////////
    /*///////////////// acc ///////////////////////
        * experiment of acc part
        * x:
            * right: -2, 1, 0
        * y:
            * forward: -2, 1, 0
        * z:
            * up: 1, -2, 0
        */
    
    our_acc(pDataXYZ);
    if(acc_state_counter_z < 0 || !(acc_output &(~Z_MASK)) ){
        if(pDataXYZ[2] > 100 && (gyro_output & GYRO_X_B)){
            // acc_output = 1;
            acc_output |= ACC_Z_U;
            // printf("going up!!!\n");
        }else if(pDataXYZ[2] < -200 && (gyro_output & GYRO_X_F)){
            // acc_output = -2;
            // printf("going down!!!\n");
            acc_output |= ACC_Z_D;
        }else{
            acc_output &= Z_MASK;
        }
        acc_state_counter_z = WINDOW_SIZE_ACC_SMALL;
        // if(abs(pDataXYZ[2]) > STRONG_TH) {acc_state_counter_z *= 2; printf("strong z!!!\n"); }
        // printf("%d\n", pDataXYZ[2]);
    }else{
        acc_state_counter_z -= 1;
    }
    
    if(acc_state_counter_y < 0 || !(acc_output &(~Y_MASK)) ){
        if(pDataXYZ[1] > 200 && (gyro_output & (GYRO_Z_CCLK|GYRO_Z_CLK))){
            // acc_output = 1;
            acc_output |= ACC_Y_B;
        }else if(pDataXYZ[1] < -200 && (gyro_output  & (GYRO_Z_CCLK|GYRO_Z_CLK)) ){
            // acc_output = -2;
            acc_output |= ACC_Y_F;
        }else{
            acc_output &= Y_MASK;
        }
        acc_state_counter_y = WINDOW_SIZE_ACC;
        if(abs(pDataXYZ[1]) > STRONG_TH) {acc_state_counter_y *= 2;  } // printf("strong y!!!\n");
    }else{
        acc_state_counter_y -= 1;
    }


    if(acc_state_counter_x < 0 || !(acc_output &(~X_MASK)) ){
        if(pDataXYZ[0] > 200 && (gyro_output & (GYRO_Z_CCLK|GYRO_Z_CLK)) ){
            // acc_output = 1;
            acc_output |= ACC_X_L;
        }else if(pDataXYZ[0] < -800 && (gyro_output & (GYRO_Z_CCLK|GYRO_Z_CLK))){
            // acc_output = -2;
            acc_output |= ACC_X_R;
        }else{
            acc_output &= X_MASK;
        }
        acc_state_counter_x = WINDOW_SIZE_ACC;
        if(abs(pDataXYZ[0]) > STRONG_TH) { acc_state_counter_x *= 2;  } // printf("strong x!!!\n");
    }else{
        acc_state_counter_x -= 1;
    }
    *move = acc_output;
    
    ////// print acc status
    // printf("acc status: ");
    // if(acc_output & ACC_Z_U){
    //     printf("up, ");
    // }else if(acc_output & ACC_Z_D){
    //     printf("down, ");
    // }else{
    //     printf("no z, ");
    // }

    // if(acc_output & ACC_Y_F){
    //     printf("move front, ");
    // }else if(acc_output & ACC_Y_B){
    //     printf("move back, ");
    // }else{
    //     printf("no y, ");
    // }

    // if(acc_output & ACC_X_R){
    //     printf("right, ");
    // }else if(acc_output & ACC_X_L){
    //     printf("left, ");
    // }else{
    //     printf("no x, ");
    // }
    // printf("\n");
    ///////////////////////////////////////////////

    //////////// print total status
    ////////////////////// print status
    // printf("gyro status: ");
    // if(gyro_output & GYRO_Z_CLK){
    //     printf("clockwise, ");
    // }else if(gyro_output & GYRO_Z_CCLK){
    //     printf("counter clockwise, ");
    // }else{
    //     printf("no z, ");
    // }

    // if(gyro_output & GYRO_Y_R){
    //     printf("right tilted, ");
    // }else if(gyro_output & GYRO_Y_L){
    //     printf("left tilted, ");
    // }else{
    //     printf("no y, ");
    // }

    // if(gyro_output & GYRO_X_B){
    //     printf("backwward, ");
    // }else if(gyro_output & GYRO_X_F){
    //     printf("frontward, ");
    // }else{
    //     printf("no x, ");
    // }
    // printf("acc status: ");
    // if(acc_output & ACC_Z_U){
    //     printf("up, ");
    // }else if(acc_output & ACC_Z_D){
    //     printf("down, ");
    // }else{
    //     printf("no z, ");
    // }

    // if(acc_output & ACC_Y_F){
    //     printf("move front, ");
    // }else if(acc_output & ACC_Y_B){
    //     printf("move back, ");
    // }else{
    //     printf("no y, ");
    // }

    // if(acc_output & ACC_X_R){
    //     printf("right, ");
    // }else if(acc_output & ACC_X_L){
    //     printf("left, ");
    // }else{
    //     printf("no x, ");
    // }
    // printf("\n");
    /////////////////////////////////
    return 0;
        
 
 
    
}