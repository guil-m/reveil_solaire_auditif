/**
  ******************************************************************************
  * @file    main.h
  * @author  MCD Application Team
  * @brief   Header for main.c module
  ******************************************************************************
    * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stdio.h"
#include "usbh_core.h"

#include "lcd_log.h"
#include "ff.h"
#include "ff_gen_drv.h"
#include "usbh_diskio.h"

#include "player.h"

/* Exported Defines ----------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
/* Application State Machine Structure */
typedef enum {
  APPLICATION_IDLE = 0,
  APPLICATION_START,
  APPLICATION_READY,
  APPLICATION_DISCONNECT,
}AUDIO_ApplicationTypeDef;

/* Audio Demo State Structure */
typedef enum {
  AUDIO_DEMO_IDLE = 0,
  AUDIO_DEMO_WAIT,
  AUDIO_DEMO_EXPLORE,
  AUDIO_DEMO_PLAYBACK,
  AUDIO_DEMO_RECORD,
}AUDIO_Demo_State;

/* Audio Demo State Machine Structure */
typedef struct _DemoStateMachine {
  __IO AUDIO_Demo_State state;
  __IO uint8_t select;
}AUDIO_DEMO_StateMachine;


typedef enum {
  AUDIO_SELECT_MENU = 0,
  AUDIO_PLAYBACK_CONTROL,
  AUDIO_RECORD_CONTROL,
}AUDIO_DEMO_SelectMode;

/* Exported variables---------------------------------------------------------*/
extern USBH_HandleTypeDef hUSBHost;
extern FATFS USBH_fatfs;
extern FIL AudioFile;
extern AUDIO_ApplicationTypeDef appli_state;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/* Menu API */
void AUDIO_MenuProcess(void);
uint8_t AUDIO_ShowAudioFiles(void);

/* Disk Explorer API */
uint8_t AUDIO_StorageInit(void);
FRESULT AUDIO_StorageParse(void);

/* Toggle LEDs */
void Toggle_Leds(void);

void Error_Handler(void);

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
