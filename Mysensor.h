#ifndef _MYSENSOR_H
#define _MYSENSOR_H
#include "mbed.h"

#define GYRO_Z_CLK 1
#define GYRO_Z_CCLK 2
#define GYRO_Y_R 4
#define GYRO_Y_L 8
#define GYRO_X_B 16
#define GYRO_X_F 32
#define GYRO_Z_CLK_WEAK 64
#define GYRO_Z_CCLK_WEAK 128
#define GYRO_Y_R_WEAK 256
#define GYRO_Y_L_WEAK 512
#define GYRO_X_B_WEAK 1024
#define GYRO_X_F_WEAK 2048
#define GYRO_Z_CLK_STRONG (1 << 12)
#define GYRO_Z_CCLK_STRONG (1 << 13)
#define GYRO_Y_R_STRONG (1 << 14)
#define GYRO_Y_L_STRONG (1 << 15)
#define GYRO_X_B_STRONG (1 << 16)
#define GYRO_X_F_STRONG (1 << 17)

#define ACC_Z_U 1
#define ACC_Z_D 2
#define ACC_Y_F 4
#define ACC_Y_B 8
#define ACC_X_R 16
#define ACC_X_L 32

#define Z_MASK ~3
#define Y_MASK ~12
#define X_MASK ~48

#define WINDOW_SIZE_ACC 4
#define WINDOW_SIZE_ACC_SMALL 4

#define IS_RIGHT_HAND 1
#define IS_LEFT_HAND 0

#define WALK_WINDOW_SIZE 10
#define WALK_WINDOW_TRANSITION 4
 
// Sensors drivers present in the BSP library
#include "stm32l475e_iot01_tsensor.h"
#include "stm32l475e_iot01_hsensor.h"
#include "stm32l475e_iot01_psensor.h"
#include "stm32l475e_iot01_magneto.h"
#include "stm32l475e_iot01_gyro.h"
#include "stm32l475e_iot01_accelero.h"
#include <cstdint>

void our_acc(int16_t * acc_data);
void our_gyro(float* g_data);
void calibrate_gyro();
void calibrate_acc();
void calibrate_gyro_and_acc();
int Mysensor(int *rotation, int *move);

#endif