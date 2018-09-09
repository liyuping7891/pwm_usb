/**
  ******************************************************************************
  * @file           : thread.h
  * @brief          : Header for thread.c file.
  ******************************************************************************
  *
  * key-parse
  * pwm-proc
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __THREAD_H__
#define __THREAD_H__

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "stm32f1xx_hal.h"
#include "cmsis_os2.h"
#include "hal_main.h"

#include "Driver_USART.h"
#include "rl_usb.h"

#define GOT_DATA_FROM_USB 0x01

/* Private define ------------------------------------------------------------*/


/*----------------------------------------------------------------------------
 *  Thread 3
 *---------------------------------------------------------------------------*/
#define PWM_CTL_THROUGH_USB_CONFIRM     0x0001
#define PWM_CTL_THROUGH_USART_CONFIRM   0x0002

#define PWM_PERIOD                      17578

/*----------------------------------------------------------------------------
 *  Thread 7
 *---------------------------------------------------------------------------*/
#define PARSE_CMD  0x00000001

#define PWM_GEN_CMD     0x01
#define FREQ_GEN_CMD    0x12

#define PWM_RETAIN      0x00
#define PWM_FIXED       0x40
#define PWM_INCREMETE   0x80
#define PWM_DECREMENT   0xC0

#define PWM_DELTA_1     0x00
#define PWM_DELTA_10    0x01
#define PWM_DELTA_100   0x02
#define PWM_DELTA_1000  0x03

#define MAX_PWM_CH      0x08

#define PWM_CH1         0x00
#define PWM_CH2         0x01
#define PWM_CH3         0x02
#define PWM_CH4         0x03
#define PWM_CH5         0x04
#define PWM_CH6         0x05
#define PWM_CH7         0x06
#define PWM_CH8         0x07

typedef struct {
  bool      change;
  uint16_t  val;
} CMD_PWM_FROM_USB;

extern uint8_t buf_from_usb[];

//usb_cmd format:
//selector + PWM_VAL 
//1 byte     2 bytes
//selector:
//    bit[7:6]: 00-value retian, 
//              01-value fixed
//              10-increment with delta
//              11-decrement with delta
//    bit[5:4]: delta, 00-1, 01-10, 02-100, 03-1000
//    bit[3:0]: pwm channel, 1 - 8 channel,
//              PWM_CH[1-4]-->TIM2_PWM_CH[1-4], 
//              PWM_CH[5-8]-->TIM3_PWM_CH[1-4],

/*----------------------------------------------------------------------------
 *  Thread 8
 *---------------------------------------------------------------------------*/
#define FREQ_CTL_THROUGH_USB_CONFIRM    0x0001
#define MAX_FREQ_CH     2
#define PWM_RETAIN      0x00
#define PWM_FIXED       0x40
#define PWM_INCREMETE   0x80
#define PWM_DECREMENT   0xC0

typedef struct {
  bool      change;
  uint16_t  period; // ms
  uint16_t  duty;   // ms
} CMD_FREQ_FROM_USB;
 

// Extenal declaration
extern osThreadId_t tid_parse_cmd;
extern  uint8_t  bulk_out_buf[];

#ifdef __cplusplus
 extern "C" {
#endif

int init_key_parse_thread (void);

int init_pwm_thread (void);

int init_usb_thread (void);

int init_usart2_ctl_pwm_thread (void);

int init_parse_cmd_thread (void);

int init_freq_gen_thread(void);

#ifdef __cplusplus
}
#endif

#endif /* __THREAD_H__ */

/*****END OF FILE****/