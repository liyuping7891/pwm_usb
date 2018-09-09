/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * 
  * 1. ***
  * 2. ***
  *
  ******************************************************************************
  */
  
#include <stdlib.h>
#include <string.h>

#include "stm32f1xx_hal.h"
#include "cmsis_os2.h"
#include "hal_main.h"

#include "thread.h"
#include "Driver_USART.h"
  
/*----------------------------------------------------------------------------
 *      Event Flags
 *---------------------------------------------------------------------------*/
osEventFlagsId_t os_evt_got_control;

/*----------------------------------------------------------------------------
 *      Thread Flags
 *---------------------------------------------------------------------------*/
#define THREAD_FLAG_PWM_CHANGE    0x0001
 
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
osThreadId_t tid_Thread;                                      // thread id
 
void Thread (void *argument) {
  
}
int Init_Thread (void) {
  return 0;
}

/*----------------------------------------------------------------------------
 *      Thread 2
 *      key_parse:
 *      parse input from USART or USB
 *---------------------------------------------------------------------------*/
osThreadId_t tid_key_parse;
 
void key_parse(void *argument) {
 
  while (1) {
    ; // Insert thread code here...
  }
}

int init_key_parse_thread (void) {
 
  tid_key_parse = osThreadNew (key_parse, NULL, NULL);
  if (!tid_key_parse) return(-1);
  
  return(0);
}
/*----------------------------------------------------------------------------
 *      Thread 3
 *      pwm control: 
 *      1. sync mode: all output channel change simultaneously
 *      2. non-sync mode: only 1 output channel change 
 *      implement only 1st mode for now
 *---------------------------------------------------------------------------*/
osThreadId_t tid_pwm_process;

void pwm_process (void *argument) {

  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);

  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
  
  while (1) {
    ; // Insert thread code here...
    osThreadFlagsWait(THREAD_FLAG_PWM_CHANGE, osFlagsWaitAny, osWaitForever);
    
    TIM2->CCR1 += 1;
    TIM2->CCR1 %= 17578;
    TIM2->CCR2 += 2;
    TIM2->CCR2 %= 17578;
    TIM2->CCR3 += 4;
    TIM2->CCR3 %= 17578;
    TIM2->CCR4 += 8;
    TIM2->CCR4 %= 17578;
    TIM3->CCR1 += 1;
    TIM3->CCR1 %= 17578;
    TIM3->CCR2 += 4;
    TIM3->CCR2 %= 17578;
    TIM3->CCR3 += 16;
    TIM3->CCR3 %= 17578;
    TIM3->CCR4 += 64;
    TIM3->CCR4 %= 17578;   
  
    osThreadYield();    
  }
}

int init_pwm_thread (void) {
 
  tid_pwm_process = osThreadNew (pwm_process, NULL, NULL);
  if (!tid_pwm_process) return(-1);
  
  return(0);
}
 
/*----------------------------------------------------------------------------
 *      Thread 4
 *      wakeup pwm process
 *---------------------------------------------------------------------------*/
osThreadId_t tid_pwm_wakeup;
 
void pwm_wakeup (void *argument) {
  
  while (1) {
    osThreadFlagsSet(tid_pwm_process, THREAD_FLAG_PWM_CHANGE);
    osThreadYield();
  }
}

int init_pwm_wakeup_thread (void) {
 
  tid_pwm_wakeup = osThreadNew (pwm_wakeup, NULL, NULL);
  if (!tid_pwm_wakeup) return(-1);
  
  return(0);
}

/*----------------------------------------------------------------------------
 *      Thread 5
 *      pwm control using usb process
 *---------------------------------------------------------------------------*/
extern void USBD_CustomClass0_Initialize(void);
osThreadId_t tid_usb_ctl_pwm;

void usb_ctl_pwm (void *argument) {
  
  USBD_CustomClass0_Initialize();
  
  while (1) {
    //osThreadFlagsSet(tid_pwm_process, THREAD_FLAG_PWM_CHANGE);
    osThreadYield();
  }
}

int init_usb_ctl_pwm_thread (void) {
 
  tid_usb_ctl_pwm = osThreadNew (usb_ctl_pwm, NULL, NULL);
  if (!tid_usb_ctl_pwm) return(-1);
  
  return(0);
}
 
/*----------------------------------------------------------------------------
 *      Thread 6
 *      pwm control using USART2
 *---------------------------------------------------------------------------*/
#define THREAD_FLAG_INTRA_FRAME_MB  0x02

//extern ARM_DRIVER_USART Driver_USART1;
extern ARM_DRIVER_USART Driver_USART2;
//extern ARM_DRIVER_USART Driver_USART3;
//extern ARM_DRIVER_USART Driver_USART4;
//extern ARM_DRIVER_USART Driver_USART5;

osThreadId_t tid_usart2_ctl_pwm;

static void mbTimeoutCallback(void *argument) {
  int32_t arg = (int32_t)argument;
  
  osThreadFlagsSet(tid_usart2_ctl_pwm, THREAD_FLAG_INTRA_FRAME_MB);
}

static void cb_myUSART(uint32_t event)
{
  uint32_t mask;
  mask = ARM_USART_EVENT_RECEIVE_COMPLETE  |
         ARM_USART_EVENT_TRANSFER_COMPLETE |
         ARM_USART_EVENT_SEND_COMPLETE     |
         ARM_USART_EVENT_TX_COMPLETE       ;
  if (event & mask) {
    /* Success: Wakeup Thread */
    //osSignalSet(tid_myUART_Thread, 0x01);
  }
  if (event & ARM_USART_EVENT_RX_TIMEOUT) {
    //__breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
  }
  if (event & (ARM_USART_EVENT_RX_OVERFLOW | ARM_USART_EVENT_TX_UNDERFLOW)) {
    //__breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
  }  
}
 
void usart2_ctl_pwm (void *argument) {
  int32_t status = ARM_DRIVER_OK; 
  uint8_t cmd[120]; // format: "CH"+num(01-16, 2bytes)+value(0000-4095, 4bytes)
  osTimerId_t  timer_id_mbTimeout;
  
  Driver_USART2.Initialize(cb_myUSART);
  Driver_USART2.PowerControl(ARM_POWER_FULL);
  status = Driver_USART2.Control(ARM_USART_MODE_ASYNCHRONOUS | 
                                 ARM_USART_DATA_BITS_8 | 
                                 ARM_USART_PARITY_NONE | 
                                 ARM_USART_STOP_BITS_1 | 
                                 ARM_USART_FLOW_CONTROL_NONE, 115200);
  // identical with above settings (default settings removed)
  // configure to UART mode: 8 bits, no parity, 1 stop bit, flow control, 115200 bps
  status = Driver_USART2.Control(ARM_USART_MODE_ASYNCHRONOUS, 115200);
  // enable TX output
  status = Driver_USART2.Control(ARM_USART_CONTROL_TX, 1);
  // enable RX output
  status = Driver_USART2.Control(ARM_USART_CONTROL_RX, 1);
  
  while (1) {
    Driver_USART2.Send("Please input new value:\n\r",30);
    timer_id_mbTimeout = osTimerNew(mbTimeoutCallback, osTimerPeriodic, (void *)5, NULL);
    osTimerStart(timer_id_mbTimeout, 5);
    osThreadFlagsWait(THREAD_FLAG_INTRA_FRAME_MB, osFlagsWaitAll, osWaitForever);
    
    Driver_USART2.Receive(cmd, 120);
      
  }
}

int init_usart2_ctl_pwm_thread (void) {
 
  tid_usart2_ctl_pwm = osThreadNew (usart2_ctl_pwm, NULL, NULL);
  if (!tid_usart2_ctl_pwm) return(-1);
  
  return(0);
}


