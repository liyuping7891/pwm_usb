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
#include "thread.h"
  
/*----------------------------------------------------------------------------
 *  static function
 *  static variable
 *---------------------------------------------------------------------------*/
osThreadId_t tid_Thread;
osThreadId_t tid_key_parse;
osThreadId_t tid_pwm_process;
osThreadId_t tid_pwm_wakeup;
osThreadId_t tid_usb_process;
osThreadId_t tid_usart2_ctl_pwm;
osThreadId_t tid_parse_cmd;
osThreadId_t tid_freq_gen;
osThreadId_t tid_tty;

CMD_PWM_FROM_USB  cmd_buf_pwm_dac[MAX_PWM_CH];
uint16_t          duty_buf[MAX_PWM_CH];
CMD_FREQ_FROM_USB cmd_buf_freq[MAX_FREQ_CH]; 

uint8_t buf_from_usb[64];

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 *      Thread 2 'key_parse': parse input from USART or USB
 *---------------------------------------------------------------------------*/
void key_parse(void *argument) {
 
  while (1) {
    ; // Insert thread code here...
    //Driver_USART1.Send("Now Thread 1!!\r\n", 21);
    osThreadYield();
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
void pwm_process (void *argument) {
  uint8_t i;
  
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
    osThreadFlagsWait(PWM_CTL_THROUGH_USB_CONFIRM, osFlagsWaitAny, osWaitForever);
    Driver_USART1.Send("Now Thread ---> 3.\r\n", 21);
    Driver_USART1.Send("Got pwm ctl request.\r\n", 30);
    Driver_USART1.Send("\r\n", 5);
    osThreadFlagsSet(tid_tty, 0x01);
    
    for(i = 0; i < MAX_PWM_CH; i++)
    {
      if(cmd_buf_pwm_dac[i].change == true)
      {
        duty_buf[i] %= PWM_PERIOD;
        switch(i)
        {
          case PWM_CH1: 
            TIM2->CCR1 = duty_buf[i];
            break;
          case PWM_CH2:
            TIM2->CCR2 = duty_buf[i];
            break;
          case PWM_CH3:
            TIM2->CCR3 = duty_buf[i];
            break;
          case PWM_CH4:
            TIM2->CCR4 = duty_buf[i];
            break;
          case PWM_CH5:
            TIM2->CCR1 = duty_buf[i];
            break;
          case PWM_CH6:
            TIM3->CCR2 = duty_buf[i];
            break;
          case PWM_CH7:
            TIM3->CCR3 = duty_buf[i];
            break;
          case PWM_CH8:
            TIM3->CCR4 = duty_buf[i];
            break;
          default:
            break;
        }
      }else{
        ;
      }
    }
    
    osThreadYield();
  }
}

int init_pwm_thread (void) {
 
  tid_pwm_process = osThreadNew (pwm_process, NULL, NULL);
  if (!tid_pwm_process) return(-1);
  
  return(0);
}
 
/*----------------------------------------------------------------------------
*      Thread 4 : wakeup pwm process
 *---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 *      Thread 5
 *      pwm control using usb process
 *---------------------------------------------------------------------------*/
void usb_process(void *argument) {
  
//  USBD_Initialize(0);
//  USBD_Connect(0);

  while (1) {
    ;
    osThreadYield();
  }
}

int init_usb_thread (void) {
 
  tid_usb_process = osThreadNew (usb_process, NULL, NULL);
  if (!tid_usb_process) return(-1);
  
  return(0);
}
 
/*----------------------------------------------------------------------------
 *      Thread 6
 *      pwm control using USART2
 *---------------------------------------------------------------------------*/
void usart2_ctl_pwm (void *argument) {
    
  while (1) {
    ;
    osThreadYield();  
  }
}

int init_usart2_ctl_pwm_thread (void) {
 
  tid_usart2_ctl_pwm = osThreadNew (usart2_ctl_pwm, NULL, NULL);
  if (!tid_usart2_ctl_pwm) return(-1);
  
  return(0);
}

/*----------------------------------------------------------------------------
 *  Thread 7
 *  parse cmd from usb
 *---------------------------------------------------------------------------*/
static void parse_cmd(void *argument)
{
  uint8_t i, j, k;
  uint8_t cmd_type;
  
  while(1)
  {
    osThreadFlagsWait(GOT_DATA_FROM_USB, osFlagsWaitAny, osWaitForever);
    
    Driver_USART1.Send("Now Thread -------> 7.\r\n", 30);    
    Driver_USART1.Send("Got USB Request\r\n", 21);    
    Driver_USART1.Send("\r\n", 5);
    
    cmd_type = buf_from_usb[0];
//    SEGGER_RTT_printf(0,"Here is the cmd:\n");
//    SEGGER_RTT_Write(0, buf_from_usb, 24 + 1);
//    SEGGER_RTT_printf(0,"\n");
        
    if((cmd_type ^ PWM_GEN_CMD) == 0)
    {
      duty_buf[0] = TIM2->CCR1;
      duty_buf[1] = TIM2->CCR2;
      duty_buf[2] = TIM2->CCR3;
      duty_buf[3] = TIM2->CCR4;
      duty_buf[4] = TIM3->CCR1;
      duty_buf[5] = TIM3->CCR2;
      duty_buf[6] = TIM3->CCR3;
      duty_buf[7] = TIM3->CCR4;
      
      for(i = 0; i < MAX_PWM_CH; i++)
      {
        j = buf_from_usb[3 * i + 1] & 0xC0;
        k = (buf_from_usb[3 * i + 1] & 0x30) >> 4;
        
        switch(j)
        {
          case PWM_RETAIN:
            cmd_buf_pwm_dac[i].change = false;
          case PWM_FIXED:
            cmd_buf_pwm_dac[i].change = true;
            duty_buf[i] = buf_from_usb[3 * i + 1]
                          + buf_from_usb[3 * i + 2] * 256;
            break;
          case PWM_INCREMETE:
            cmd_buf_pwm_dac[i].change = true;
            duty_buf[i] += 10 ^ k;
            break;
          case PWM_DECREMENT:
            cmd_buf_pwm_dac[i].change = true;
            duty_buf[i] -= 10 ^ k;
            break;
          default:
            break;
        }
      }       
      osThreadFlagsSet(tid_pwm_process, PWM_CTL_THROUGH_USB_CONFIRM); 
    }
    else if((cmd_type ^ FREQ_GEN_CMD) == 0)
    {
      
      osThreadFlagsSet(tid_freq_gen, FREQ_CTL_THROUGH_USB_CONFIRM);
    }
    else
    {
      // reserve for more commands
    }
    
    osThreadYield();
  }
}
    
int init_parse_cmd_thread (void) {
 
  tid_parse_cmd = osThreadNew (parse_cmd, NULL, NULL);
  if (!tid_parse_cmd) return(-1);
  
  return(0);
}


/*----------------------------------------------------------------------------
 *  Thread 8, freq_gen: output 2.85-3.65Hz adjustable pwm
 *---------------------------------------------------------------------------*/
static void freq_gen(void *argument)
{
//  uint8_t i, j, k;
  
  while(1)
  {
    osThreadFlagsWait(FREQ_CTL_THROUGH_USB_CONFIRM, osFlagsWaitAny, osWaitForever);
    
    Driver_USART1.Send("Now Thread --------> 8.\r\n", 30);
    
    osThreadYield();
  }
}
    
int init_freq_gen_thread (void) {
 
  tid_freq_gen = osThreadNew (freq_gen, NULL, NULL);
  if (!tid_freq_gen) return(-1);
  
  return(0);
}

/*----------------------------------------------------------------------------
 *  Thread 9, usart1 used for terminal output
 *---------------------------------------------------------------------------*/
void myUSART_callback(uint32_t event)
{
  uint32_t mask;
  mask = ARM_USART_EVENT_RECEIVE_COMPLETE  |
         ARM_USART_EVENT_TRANSFER_COMPLETE |
         ARM_USART_EVENT_SEND_COMPLETE     |
         ARM_USART_EVENT_TX_COMPLETE       ;
  if (event & mask) {
    /* Success: Wakeup Thread */
  }
  
  if (event & ARM_USART_EVENT_RX_TIMEOUT) {
    //__breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
  }
  
  if (event & (ARM_USART_EVENT_RX_OVERFLOW | ARM_USART_EVENT_TX_UNDERFLOW)) {
    //__breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
  }
}

static void tty(void *argument)
{
  static
    ARM_DRIVER_USART      *USARTdrv = &Driver_USART1;
//  ARM_DRIVER_VERSION      version;
//  ARM_USART_CAPABILITIES  drv_capabilities;
//  char                    cmd;

  #ifdef DEBUG
  version = USARTdrv->GetVersion();
  if (version.api < 0x200)   /* requires at minimum API version 2.00 or higher */
  {                          /* error handling */
      return;
  }
  drv_capabilities = USARTdrv->GetCapabilities();
  if (drv_capabilities.event_tx_complete == 0)
  {                          /* error handling */
      return;
  }
  #endif

  /*Initialize the USART driver */
  USARTdrv->Initialize(myUSART_callback);
  /*Power up the USART peripheral */
  USARTdrv->PowerControl(ARM_POWER_FULL);
  /*Configure the USART to 4800 Bits/sec */
  USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS |
                    ARM_USART_DATA_BITS_8 |
                    ARM_USART_PARITY_NONE |
                    ARM_USART_STOP_BITS_1 |
                    ARM_USART_FLOW_CONTROL_NONE, 115200);
   
  /* Enable Receiver and Transmitter lines */
  USARTdrv->Control (ARM_USART_CONTROL_TX, 1);
  USARTdrv->Control (ARM_USART_CONTROL_RX, 1);
   
  while (1)
  {
    osThreadFlagsWait (0x01, osFlagsWaitAny, osWaitForever); 
    
    USARTdrv->Send("Now Thread ---------> 9.\r\n", 30);
    USARTdrv->Send("Now the CH info is:\r\n", 30);
    USARTdrv->Send(duty_buf, 24);
    USARTdrv->Send("\r\n", 5);
    Driver_USART1.Send(buf_from_usb, 30);
    
    osThreadYield();
  }
}
    
int init_tty_thread (void) {
 
  tid_tty = osThreadNew (tty, NULL, NULL);
  if (!tid_tty) return(-1);
  
  return(0);
}


