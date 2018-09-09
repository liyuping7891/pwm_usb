/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/
 
#include "RTE_Components.h"
#include "cmsis_os2.h"
#include "stm32f1xx_hal.h"
#include "hal_main.h" 
#include "thread.h"
#include "WinUSB_Echo.h"
 
#ifdef RTE_Compiler_EventRecorder
#include "EventRecorder.h"
#endif
 
/*----------------------------------------------------------------------------
 * Application main thread
 *---------------------------------------------------------------------------*/
//void app_main (void *argument) {
// 
//  // ...
//  for (;;) {}
//}
 
void ta (void *argument) {
 
  // ...
  for (;;) {}
} 
 
void tb (void *argument) {
 
  // ...
  for (;;) {}
}
 
void tc (void *argument) {
 
  // ...
  for (;;) {}
}
 


int main (void) {
 
  // System Initialization
  SystemCoreClockUpdate();
  
#ifdef RTE_Compiler_EventRecorder
  // Initialize and start Event Recorder
  EventRecorderInitialize(EventRecordError, 1U);
#endif
  // ...
  hal_main();
 
  osKernelInitialize();                 // Initialize CMSIS-RTOS
  
//  osThreadNew(app_main, NULL, NULL);    // Create application main thread
  osThreadNew(ta, NULL, NULL);    // Create application main thread
  osThreadNew(tb, NULL, NULL);    // Create application main thread
  osThreadNew(tc, NULL, NULL);    // Create application main thread
  osThreadNew(app_main, NULL, &app_main_attr);
//  init_usb_thread();
  
  init_key_parse_thread();
  
  init_parse_cmd_thread();
  init_pwm_thread();
  init_freq_gen_thread();
  init_tty_thread();
  
  osKernelStart();                      // Start thread execution
  for (;;) {}
}
