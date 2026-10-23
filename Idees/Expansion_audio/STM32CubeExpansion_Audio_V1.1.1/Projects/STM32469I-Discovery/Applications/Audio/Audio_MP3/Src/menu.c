/**
******************************************************************************
* @file    menu.c
* @author  MCD Application Team
* @brief   This file implements Menu Functions
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


/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define TOUCH_RECORD_XMIN       420
#define TOUCH_RECORD_XMAX       500
#define TOUCH_RECORD_YMIN       260
#define TOUCH_RECORD_YMAX       340

#define TOUCH_PLAYBACK_XMIN     200
#define TOUCH_PLAYBACK_XMAX     280
#define TOUCH_PLAYBACK_YMIN     260
#define TOUCH_PLAYBACK_YMAX     340

#define TOUCH_TEXT_LINE         18

/* Private macro -------------------------------------------------------------*/
/* Global extern variables ---------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
AUDIO_DEMO_StateMachine     AudioDemo;
AUDIO_PROC_StateTypeDef     AudioState;

/* Private function prototypes -----------------------------------------------*/
static void AUDIO_ChangeSelectMode(AUDIO_DEMO_SelectMode select_mode);
static void LCD_ClearTextZone(void);

/* Private functions ---------------------------------------------------------*/

/**
* @brief  Manages AUDIO Menu Process.
* @param  None
* @retval None
*/
void AUDIO_MenuProcess(void)
{
  TS_StateTypeDef  TS_State;

  Point PlaybackLogoPoints[] = {{TOUCH_PLAYBACK_XMIN, TOUCH_PLAYBACK_YMIN},
  {TOUCH_PLAYBACK_XMAX, (TOUCH_PLAYBACK_YMIN+TOUCH_PLAYBACK_YMAX)/2},
  {TOUCH_PLAYBACK_XMIN, TOUCH_PLAYBACK_YMAX}};

  if(appli_state == APPLICATION_READY)
  {
    switch(AudioDemo.state)
    {
    case AUDIO_DEMO_IDLE:

      BSP_LCD_SetFont(&LCD_LOG_HEADER_FONT);
      LCD_ClearTextZone();
      BSP_LCD_ClearStringLine(14);    /* Clear touch screen buttons dedicated zone */
      BSP_LCD_ClearStringLine(15);
      BSP_LCD_ClearStringLine(16);
      BSP_LCD_ClearStringLine(17);
      BSP_LCD_ClearStringLine(18);
      BSP_LCD_ClearStringLine(19);
      BSP_LCD_ClearStringLine(20);

      BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
      BSP_LCD_FillPolygon(PlaybackLogoPoints, 3);                 /* Playback sign */
      BSP_LCD_FillCircle((TOUCH_RECORD_XMAX+TOUCH_RECORD_XMIN)/2, /* Record circle */
                         (TOUCH_RECORD_YMAX+TOUCH_RECORD_YMIN)/2,
                         (TOUCH_RECORD_XMAX-TOUCH_RECORD_XMIN)/2);
      BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
      BSP_LCD_SetFont(&LCD_LOG_TEXT_FONT);

      BSP_LCD_DisplayStringAtLine(TOUCH_TEXT_LINE, (uint8_t *)" Use touch screen to enter playback or record menu");
      BSP_LCD_SetFont(&LCD_LOG_TEXT_FONT);

      AudioDemo.state = AUDIO_DEMO_WAIT;

      break;

    case AUDIO_DEMO_WAIT:

      HAL_Delay(150);
      BSP_TS_GetState(&TS_State);
      if(TS_State.touchDetected == 1)
      {
        if ((TS_State.touchX[0] > TOUCH_RECORD_XMIN-20) && (TS_State.touchX[0] < TOUCH_RECORD_XMAX+20) &&
            (TS_State.touchY[0] > TOUCH_RECORD_YMIN-20) && (TS_State.touchY[0] < TOUCH_RECORD_YMAX+20))
        {
          AudioDemo.state = AUDIO_DEMO_RECORD;

          /* Clear the LCD */
          LCD_ClearTextZone();
          BSP_LCD_ClearStringLine(14);    /* Clear touch screen buttons dedicated zone */
          BSP_LCD_ClearStringLine(15);
          BSP_LCD_ClearStringLine(16);
          BSP_LCD_ClearStringLine(17);
          BSP_LCD_ClearStringLine(18);
          BSP_LCD_ClearStringLine(19);
          BSP_LCD_ClearStringLine(20);

        }
        else if ((TS_State.touchX[0] > TOUCH_PLAYBACK_XMIN-20) && (TS_State.touchX[0] < TOUCH_PLAYBACK_XMAX+20) &&
                 (TS_State.touchY[0] > TOUCH_PLAYBACK_YMIN-20) && (TS_State.touchY[0] < TOUCH_PLAYBACK_YMAX+20))
        {
          AudioDemo.state = AUDIO_DEMO_PLAYBACK;

          /* Clear the LCD */
          LCD_ClearTextZone();
          BSP_LCD_ClearStringLine(14);    /* Clear touch screen buttons dedicated zone */
          BSP_LCD_ClearStringLine(15);
          BSP_LCD_ClearStringLine(16);
          BSP_LCD_ClearStringLine(17);
          BSP_LCD_ClearStringLine(18);
          BSP_LCD_ClearStringLine(19);
          BSP_LCD_ClearStringLine(20);

        }
        else
        {
          AudioDemo.state = AUDIO_DEMO_EXPLORE;
        }

      }
      break;

    case AUDIO_DEMO_EXPLORE:

      /* Clear the LCD */
      LCD_ClearTextZone();
      BSP_LCD_ClearStringLine(14);    /* Clear touch screen buttons dedicated zone */
      BSP_LCD_ClearStringLine(15);
      BSP_LCD_ClearStringLine(16);
      BSP_LCD_ClearStringLine(17);
      BSP_LCD_ClearStringLine(18);
      BSP_LCD_ClearStringLine(19);
      BSP_LCD_ClearStringLine(20);

      if(appli_state == APPLICATION_READY)
      {
        if(AUDIO_ShowAudioFiles() > 0)
        {
          LCD_ErrLog("There is no WAV file on the USB Key.\n");
          AUDIO_ChangeSelectMode(AUDIO_SELECT_MENU);
          AudioDemo.state = AUDIO_DEMO_IDLE;
        }
        else
        {
          AudioDemo.state = AUDIO_DEMO_IDLE;
        }
      }
      else
      {
        AudioDemo.state = AUDIO_DEMO_WAIT;
      }
      break;

    case AUDIO_DEMO_PLAYBACK:

      if(appli_state == APPLICATION_READY)
      {
        if(AudioState == AUDIO_STATE_IDLE)
        {
          if(AUDIO_ShowAudioFiles() > 0)
          {
            LCD_ErrLog("There is no Audio file (WAV, MP3) on the USB Key.\n");
            AUDIO_ChangeSelectMode(AUDIO_SELECT_MENU);
          }
          else
          {
            /* Start Playing */
            AudioState = AUDIO_STATE_INIT;
          }

        }
        else /* Not idle */
        {
          if (AudioState == AUDIO_STATE_START)
          {
            /* Clear the LCD */
            LCD_ClearTextZone();
          }
          /* Execute Audio Process state machine */
          if((PLAYER_Process() == AUDIO_ERROR_IO) || (PLAYER_Process() == AUDIO_ERROR_SRC) || \
            (PLAYER_Process() == AUDIO_ERROR_INVALID_VALUE))
          {
            /* Clear the LCD */
            LCD_ClearTextZone();

            AUDIO_ChangeSelectMode(AUDIO_SELECT_MENU);
            AudioDemo.state = AUDIO_DEMO_IDLE;
          }
          if(AudioState == AUDIO_STATE_BACKMENU)
          {
            AudioState = AUDIO_STATE_IDLE;
            AudioDemo.state = AUDIO_DEMO_IDLE;
          }
        }
      }
      else
      {
        AudioDemo.state = AUDIO_DEMO_WAIT;
      }
      break;

    case AUDIO_DEMO_RECORD:
      BSP_LCD_SetFont(&LCD_LOG_TEXT_FONT);

      if(appli_state == APPLICATION_READY)
      {
        if(AudioState == AUDIO_STATE_IDLE)
        {
          /* Init storage */
          AUDIO_StorageInit();

          /* Clear the LCD */
          LCD_ClearTextZone();

          /* Start Playing */
          AudioState = AUDIO_STATE_INIT;

        }
        else /* Not idle */
        {

          if((RECORDER_Process() == AUDIO_ERROR_IO) || (RECORDER_Process() == AUDIO_ERROR_EOF) || \
            (RECORDER_Process() == AUDIO_ERROR_INVALID_VALUE))
          {
            /* Clear the LCD */
            LCD_ClearTextZone();

            AUDIO_ChangeSelectMode(AUDIO_SELECT_MENU);
            AudioDemo.state = AUDIO_DEMO_IDLE;
          }

          if(AudioState == AUDIO_STATE_BACKMENU)
          {
            AudioState = AUDIO_STATE_IDLE;
            AudioDemo.state = AUDIO_DEMO_IDLE;
            HAL_Delay(150);
          }

        }
      }
      else
      {
        AudioDemo.state = AUDIO_DEMO_WAIT;
      }
      break;

    default:
      break;
    }
  }


  if(appli_state == APPLICATION_DISCONNECT)
  {
    appli_state = APPLICATION_IDLE;
    AUDIO_ChangeSelectMode(AUDIO_SELECT_MENU);
    BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
  }
}

/*******************************************************************************
Static Functions
*******************************************************************************/

/**
* @brief  Changes the selection mode.
* @param  select_mode: Selection mode
* @retval None
*/
static void AUDIO_ChangeSelectMode(AUDIO_DEMO_SelectMode select_mode)
{
  if(select_mode == AUDIO_SELECT_MENU)
  {
    LCD_LOG_UpdateDisplay();
    AudioDemo.state = AUDIO_DEMO_IDLE;
  }
  else if(select_mode == AUDIO_PLAYBACK_CONTROL)
  {
    LCD_ClearTextZone();
  }
  else if(select_mode == AUDIO_RECORD_CONTROL)
  {
    LCD_ClearTextZone();
  }
}

/**
* @brief  Clears the text zone.
* @param  None
* @retval None
*/
static void LCD_ClearTextZone(void)
{
  uint8_t i = 0;

  for(i= 0; i < 13; i++)
  {
    BSP_LCD_ClearStringLine(i + 3);
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
