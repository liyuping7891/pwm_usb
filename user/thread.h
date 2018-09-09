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


/* Private define ------------------------------------------------------------*/
#ifdef __cplusplus
 extern "C" {
#endif
int init_key_parse_thread (void);
int init_pwm_thread (void);
int init_pwm_wakeup_thread (void);
int init_usb_ctl_pwm_thread (void);

#ifdef __cplusplus
}
#endif

#endif /* __THREAD_H__ */

/*****END OF FILE****/
