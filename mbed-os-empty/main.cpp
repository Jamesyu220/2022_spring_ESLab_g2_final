#include "mbed.h"
 
// Sensors drivers present in the BSP library
#include "stm32l475e_iot01_tsensor.h"
#include "stm32l475e_iot01_hsensor.h"
#include "stm32l475e_iot01_psensor.h"
#include "stm32l475e_iot01_magneto.h"
#include "stm32l475e_iot01_gyro.h"
#include "stm32l475e_iot01_accelero.h"
#include <cstdint>
 
DigitalOut led(LED1);
int acc_x_normalize = 30;
int acc_y_normalize = 0;
int acc_z_normalize = 1015; // positive: moving upward

float gyro_x_normalize = -210; // positive: front moving downward
float gyro_y_normalize = -700;
float gyro_z_normalize = 1120; // positive: clockwize rotation 
float gyro_scale = 100;
float gyro_threshold = 200;


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

void calibrate_gyro(){
    return;
}

void calibrate_acc(){
    // use most frequent number
    fprintf(stderr, "ebter function\n");
    int16_t acc_data[1000][3];
    printf("shit\n");
    for(int i = 0; i < 1000; ++i)
    {
        BSP_ACCELERO_AccGetXYZ(acc_data[i]);
    }
    printf("fuck \n");
    int16_t c_x = 0;
    int16_t c_y = 0;
    int16_t c_z = 0;
    for(int i = 0; i < 10; ++i)
    {
        printf("i: %d\n", i);
        int16_t tmp_x = 0;
        int16_t tmp_y = 0;
        int16_t tmp_z = 0;
        
        int x_count = 0;
        int y_count = 0;
        int z_count = 0;
        for(int j = 0; j < 100; ++j)
        {
            if(abs(acc_data[100*i+j][0] - acc_data[100*i+j-1][0]) <= 10){
                tmp_x += acc_data[100*i+j][0];
                x_count += 1;
            }
            if(abs(acc_data[100*i+j][1] - acc_data[100*i+j-1][1]) <= 10){
                tmp_y += acc_data[100*i+j][1];
                y_count += 1;
            }
            if(abs(acc_data[100*i+j][2] - acc_data[100*i+j-1][2]) <= 10){ 
                tmp_z += acc_data[100*i+j][2];
                z_count += 1;
            }
        }
        c_x += tmp_x / x_count;
        c_y += tmp_y / y_count;
        c_z += tmp_z / z_count;

    }
    c_x /= 10;
    c_y /= 10;
    c_z /= 10;
    printf("after calibration: %d, %d, %d\n", c_x, c_y, c_z);
}

void calibrate_gyro_and_acc(){
    calibrate_gyro();
    calibrate_acc();
}
 
int main()
{
    float sensor_value = 0;
    int16_t pDataXYZ[3] = {0};
    float pGyroDataXYZ[3] = {0};
 
    printf("Start sensor init\n");

 
    BSP_MAGNETO_Init();
    BSP_GYRO_Init();
    BSP_ACCELERO_Init();
 
    while(1) {
    // calibrate_acc();
 
        ThisThread::sleep_for(10ms);
 
        led = 1;
 
        // BSP_MAGNETO_GetXYZ(pDataXYZ);
        // printf("\nMAGNETO_X = %d\n", pDataXYZ[0]);
        // printf("MAGNETO_Y = %d\n", pDataXYZ[1]);
        // printf("MAGNETO_Z = %d\n", pDataXYZ[2]);
 
        // our_gyro(pGyroDataXYZ);
        // // printf("\nGYRO_X = %.2f\n", pGyroDataXYZ[0]);
        // // printf("GYRO_Y = %.2f\n", pGyroDataXYZ[1]);
        // printf("GYRO_Z = %.2f\n", pGyroDataXYZ[2]);

        int output = 0;
        our_gyro(pGyroDataXYZ);
        if(pGyroDataXYZ[2] > 200){
            output = 1;
        }else if(pGyroDataXYZ[2] < -200){
            output = -2;
        }else{
            output = 0;
        }
        printf("\rGYRO_Z = %d\n", output);
        
        // printf("\nACCELERO_X = %d\n", pDataXYZ[0]);
        // printf("ACCELERO_Y = %d\n", pDataXYZ[1]);

        /*///////////////// acc ///////////////////////
         * experiment of acc part
            * x:
                * right: -2, 1, 0
            * y:
                * forward: -2, 1, 0
            * z:
                * up: 1, -2, 0
         */
        // int output = 0;
        // our_acc(pDataXYZ);
        // if(pDataXYZ[0] > 200){
        //     output = 1;
        // }else if(pDataXYZ[0] < -200){
        //     output = -2;
        // }else{
        //     output = 0;
        // }
        // printf("\rACCELERO_Z = %d\n", output);
        ///////////////////////////////////////////////


        ThisThread::sleep_for(10ms);
        led = 0;

        // BSP_MAGNETO_GetXYZ(pDataXYZ);
        // printf("\nMAGNETO_X = %d\n", pDataXYZ[0]);
        // printf("MAGNETO_Y = %d\n", pDataXYZ[1]);
        // printf("MAGNETO_Z = %d\n", pDataXYZ[2]);
 
        our_gyro(pGyroDataXYZ);
        if(pGyroDataXYZ[2] > 200){
            output = 1;
        }else if(pGyroDataXYZ[2] < -200){
            output = -2;
        }else{
            output = 0;
        }
        printf("\rGYRO_Z = %d\n", output);
        // printf("\nGYRO_X = %.2f\n", pGyroDataXYZ[0]);
        // printf("GYRO_Y = %.2f\n", pGyroDataXYZ[1]);
        // printf("GYRO_Z = %.2f\n", pGyroDataXYZ[2]);
 
        
        // printf("\nACCELERO_X = %d\n", pDataXYZ[0]);
        // printf("ACCELERO_Y = %d\n", pDataXYZ[1]);

        //////////////////// acc part /////////////////////
        // our_acc(pDataXYZ);
        // if(pDataXYZ[0] > 200){
        //     output = 1;
        // }else if(pDataXYZ[0] < -200){
        //     output = -2;
        // }else{
        //     output = 0;
        // }
        // printf("\rACCELERO_Z = %d\n", output);
        ///////////////////////////////////////////////////
 
 
    }
    printf("here\n");
    calibrate_acc();
}