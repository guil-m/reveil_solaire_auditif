/**
******************************************************************************
* @file    recorder.c
* @author  MCD Application Team
* @brief   This file provides the Audio In (record) interface API
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
#include "recorder.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* LCD OTM8009A on F4 Disco: (X*Y)800*480 pixels */

#define TOUCH_PAUSE_XMIN         (int)(130*(800.0/480))
#define TOUCH_PAUSE_XMAX         (int)(210*(800.0/480))
#define TOUCH_PAUSE_YMIN         (int)(212*(480.0/272))
#define TOUCH_PAUSE_YMAX         (int)(252*(480.0/272))
#define TOUCH_PAUSE_XSPACE       (int)(20*(800.0/480))

#define TOUCH_STOP_XMIN           (int)(220*(800.0/480))
#define TOUCH_STOP_XMAX           (int)(260*(800.0/480))
#define TOUCH_STOP_YMIN           (int)(212*(480.0/272))
#define TOUCH_STOP_YMAX           (int)(252*(480.0/272))

#define TOUCH_RECORD_XMIN         (int)(310*(800.0/480))
#define TOUCH_RECORD_XMAX         (int)(360*(800.0/480))
#define TOUCH_RECORD_YMIN         (int)(212*(480.0/272))
#define TOUCH_RECORD_YMAX         (int)(252*(480.0/272))

#define TOUCH_VOL_MINUS_XMIN    (int)(20*(800.0/480))
#define TOUCH_VOL_MINUS_XMAX    (int)(70*(800.0/480))
#define TOUCH_VOL_MINUS_YMIN    (int)(212*(480.0/272))
#define TOUCH_VOL_MINUS_YMAX    (int)(252*(480.0/272))

#define TOUCH_VOL_XOFFSET       (int)(6*(480.0/272))
#define TOUCH_VOL_YLINE         17
#define MSG_RECORD_TEXTYLINE    16

#define TOUCH_VOL_PLUS_XMIN     (int)(402*(800.0/480))
#define TOUCH_VOL_PLUS_XMAX     (int)(452*(800.0/480))
#define TOUCH_VOL_PLUS_YMIN     (int)(212*(480.0/272))
#define TOUCH_VOL_PLUS_YMAX     (int)(252*(480.0/272))

#define TOUCH_BACK_XMIN         (int)(394*(800.0/480))
#define TOUCH_BACK_XMAX         (int)(468*(800.0/480))
#define TOUCH_BACK_YMIN         (int)(120*(480.0/272))
#define TOUCH_BACK_YMAX         (int)(166*(480.0/272))

#define TOUCH_BACK_XOFFSET      (int)(10*(480.0/272))
#define MSG_BACK_YLINE          10

#define MSG_RECORD_XPOS         (int)(240*(800.0/480))

#define PLAYER_COUNT_TEXT_XMIN     (int)(158*(800.0/480))
#define PLAYER_COUNT_TEXT_KB_XMIN  (int)(90*(800.0/480))
#define PLAYER_COUNT_TEXT_YLINE    8


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

#if defined(__WAV_ENCODER__)
WAVE_FormatTypeDef WaveFormat;
#endif
uint8_t pHeaderBuff[44];

/* Audio Input file */
FIL AudioRecFile;
char FileRecExtension;

/* Buffers */
uint16_t *pEncSamples = NULL;
uint32_t EncPacketSize = MAX_IN_PACKET_SZE;
uint32_t i=0;

/* Global structure for the encoder */
Encoder_TypeDef  sEncoderStruct;
uint32_t WrtNbr = 0;

/* Input Audio File*/
static uint8_t  *FileRecPath;
static AUDIO_IN_BufferTypeDef  BufferRecCtl;

static __IO uint32_t uwVolume = DEFAULT_AUDIO_IN_VOLUME;
static uint32_t  display_update = 1;

/* Default micro sampling frequency */
static uint32_t SampleFreq = AUDIO_IN_FREQ_ADPT;

/* Private function prototypes -----------------------------------------------*/
static void RecorderDisplayButtons(void);

/* Private functions ---------------------------------------------------------*/

/*
A double MEMS microphone MP45DT02 mounted on STM324X9I-DISCOVERY is connected
to the Inter-IC Sound (I2S) peripheral. The I2S is configured in master
receiver mode. In this mode, the I2S peripheral provides the clock to the MEMS
microphones through CLK_IN and acquires the data (Audio samples) from the MEMS
microphone through PDM_OUT.

Data acquisition is performed in 16-bit PDM format and using I2S DMA mode.

DMA is configured in circular mode

In order to avoid data-loss, a 512 bytes buffer is used (BufferRecCtl.pdm_buff):
- When a DMA half transfer is detected using the call back BSP_AUDIO_IN_HalfTransfer_CallBack()
PDM frame has been received (256 bytes), a conversion to PCM is done
and then this PCM frame is encoded and saved in EncSamples buffer.
- After converting/filtering samples from PDM to PCM, the samples are encoded and stored in USB buffer.
- These two steps are repeated  when the DMA Transfer complete interrupt is detected
- When half of internal USB buffer is reach, an evacuation though USB is done.

To avoid data-loss:
- IT ISR priority must be set at a higher priority than USB, this priority
order must be respected when managing other interrupts;
- The processing time of converting/filtering samples from PDM to PCM
PDM_Filter_64_LSB()) should be lower than the time required to fill a
single buffer.

Note that a PDM Audio software decoding library provided in binary is used in
this application. For IAR EWARM toolchain, the library is labeled
"libPDMFilter_CM4_IAR.a".
*/

/**
* @brief  Starts Audio streaming.
* @param  None
* @retval Audio error
*/
AUDIO_ErrorTypeDef RECORDER_Start(void)
{
  uint32_t byteswritten = 0;
  uint8_t str[FILEMGR_FILE_NAME_SIZE + 20];

  uwVolume = 100;   /* DEFAULT_AUDIO_IN_VOLUME */
  FileRecPath = REC_AUDIO_NAME;

  f_close(&AudioRecFile);

  /* Create a new file system */
  if(f_open(&AudioRecFile, (char*)FileRecPath, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK)
  {
    return AUDIO_ERROR_IO;
  }

  /* Effect Initialization and configuration */
  /*-----------------------------------------*/

  /* Enables and resets CRC-32 from STM32 HW */
  __HAL_RCC_CRC_CLK_ENABLE();
  CRC->CR = CRC_CR_RESET;

  /* get the extension of audio file to be recorded */
  FileRecExtension = FileRecPath[strlen((char *)FileRecPath) - 1];

  /* Initialize the encoder instance with Null pointers */
  CODERS_SelectEncoder(&sEncoderStruct,' ');

  /* Select the appropriate encoder */
  if(CODERS_SelectEncoder(&sEncoderStruct, FileRecExtension) != 0)
  {
    return AUDIO_ERROR_CODEC;
  }

  /* Get the packet size */
  EncPacketSize = AUDIO_IN_PCM_BUFFER_SIZE;  /* PCM input buffer in 16b samples*/

  /* Get the most appropriate packet size */
  switch (FileRecExtension)
  {
  case 'V':
  case 'v':
#if defined(__WAV_ENCODER__)
    EncPacketSize = WAVIN_PACKET_SZE; /*stereo in bytes*/
    if (sEncoderStruct.EncoderInit != NULL)
    {
      if( sEncoderStruct.EncoderInit(SampleFreq, pHeaderBuff))
      {
        return AUDIO_ERROR_CODEC;
      }
    }
    /* Write the header file (File size will be updated at the end of the recording) */
    if(f_write(&AudioRecFile, pHeaderBuff, 44, (void*)&byteswritten) != FR_OK)
    {
      return AUDIO_ERROR_IO;
    }
#endif
    break;

  case '3':
#if defined(__MP3_ENCODER__)
    EncPacketSize = MP3IN_PACKET_SZE; /*stereo in bytes*/

    /*parameters specific to MP3 encoder*/
    MP3EncNumChannels = DEFAULT_AUDIO_IN_CHANNEL_NBR;
    MP3EncBitRate     = 128000; /*compression factor in bps*/

    if (sEncoderStruct.EncoderInit != NULL)
    {
      if( sEncoderStruct.EncoderInit(SampleFreq, pHeaderBuff))
      {
        return AUDIO_ERROR_CODEC;
      }
    }
#endif
    break;
  default:
    EncPacketSize = WAVIN_PACKET_SZE;
    if (sEncoderStruct.EncoderInit != NULL)
    {
      if( sEncoderStruct.EncoderInit(SampleFreq, pHeaderBuff))
      {
        return AUDIO_ERROR_CODEC;
      }
    }
    break;
  }

  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
  sprintf((char *)str, "Recording file: %s",
          (char *)FileRecPath);
  BSP_LCD_ClearStringLine(4);
  BSP_LCD_DisplayStringAtLine(4, str);

  BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
  sprintf((char *)str,  "Sample rate : %d Hz", (int)AUDIO_IN_FREQ_ADPT);
  BSP_LCD_ClearStringLine(5);
  BSP_LCD_DisplayStringAtLine(5, str);

  sprintf((char *)str,  "Channels number : %d", (int)DEFAULT_AUDIO_IN_CHANNEL_NBR);
  BSP_LCD_ClearStringLine(6);
  BSP_LCD_DisplayStringAtLine(6, str);

  sprintf((char *)str,  "Volume : %d ", (int)uwVolume);
  BSP_LCD_ClearStringLine(7);
  BSP_LCD_DisplayStringAtLine(7, str);

  sprintf((char *)str, "File Size :");
  BSP_LCD_ClearStringLine(8);
  BSP_LCD_DisplayStringAtLine(8, str);

  RecorderDisplayButtons();

  BSP_LCD_DisplayStringAt(MSG_RECORD_XPOS, LINE(9), (uint8_t *)"  [RECORD]", LEFT_MODE);

  /* Audio Stream Input */
  BSP_AUDIO_IN_Init(AUDIO_IN_FREQ_ADPT, DEFAULT_AUDIO_IN_BIT_RESOLUTION, DEFAULT_AUDIO_IN_CHANNEL_NBR);
  BSP_AUDIO_IN_Record((uint16_t*)&BufferRecCtl.pdm_buff[0], AUDIO_IN_PDM_BUFFER_SIZE);

  BufferRecCtl.fptr = byteswritten; /* Nb bytes written in output file: header */
  BufferRecCtl.pcm_ptr = 0;
  BufferRecCtl.offset = 0;
  BufferRecCtl.wr_state = BUFFER_EMPTY;

  return AUDIO_ERROR_NONE;
}

/**
* @brief  Manages Audio process.
* @param  None
* @retval Audio error
*/
AUDIO_ErrorTypeDef RECORDER_Process(void)
{
  uint32_t byteswritten = 0;
  AUDIO_ErrorTypeDef audio_error = AUDIO_ERROR_NONE;

  uint32_t elapsed_time;
  uint8_t str[10];

  static TS_StateTypeDef  TS_State={0};

  uint32_t t_size = 0;    /* buffer size to write in bytes */
  uint32_t wrt_error = 0x00;

  switch(AudioState)
  {
  case AUDIO_STATE_PRERECORD:
    if(TS_State.touchDetected == 1)   /* If previous touch has not been released, we don't proceed any touch command */
    {
      BSP_TS_GetState(&TS_State);
    }
    else
    {
      BSP_TS_GetState(&TS_State);
      if(TS_State.touchDetected == 1)
      {
        if ((TS_State.touchX[0] > TOUCH_STOP_XMIN) && (TS_State.touchX[0] < TOUCH_STOP_XMAX) &&
            (TS_State.touchY[0] > TOUCH_STOP_YMIN) && (TS_State.touchY[0] < TOUCH_STOP_YMAX))
        {
          AudioState = AUDIO_STATE_STOP;
        }
        else if ((TS_State.touchX[0] > TOUCH_RECORD_XMIN) && (TS_State.touchX[0] < TOUCH_RECORD_XMAX) &&
                 (TS_State.touchY[0] > TOUCH_RECORD_YMIN) && (TS_State.touchY[0] < TOUCH_RECORD_YMAX))
        {
          display_update = 1;
          AudioState = AUDIO_STATE_RECORD;
        }
        else if((TS_State.touchX[0] > TOUCH_BACK_XMIN) && (TS_State.touchX[0] < TOUCH_BACK_XMAX) &&
                (TS_State.touchY[0] > TOUCH_BACK_YMIN) && (TS_State.touchY[0] < TOUCH_BACK_YMAX))
        {
          /* Stop input audio stream */
          BSP_AUDIO_IN_Stop();
          f_close(&AudioRecFile);

          AudioState = AUDIO_STATE_BACKMENU;
        }
        else if((TS_State.touchX[0] > TOUCH_VOL_MINUS_XMIN) && (TS_State.touchX[0] < TOUCH_VOL_MINUS_XMAX) &&
                (TS_State.touchY[0] > TOUCH_VOL_MINUS_YMIN) && (TS_State.touchY[0] < TOUCH_VOL_MINUS_YMAX))
        {
          AudioState = AUDIO_STATE_VOLUME_DOWN;
          if(uwVolume >= 5)
          {
            uwVolume -= 5;
          }
        }
        else if((TS_State.touchX[0] > TOUCH_VOL_PLUS_XMIN) && (TS_State.touchX[0] < TOUCH_VOL_PLUS_XMAX) &&
                (TS_State.touchY[0] > TOUCH_VOL_PLUS_YMIN) && (TS_State.touchY[0] < TOUCH_VOL_PLUS_YMAX))
        {
          AudioState = AUDIO_STATE_VOLUME_UP;
          if(uwVolume <= 95)
          {
            uwVolume += 5;
          }
        }

        if ((AudioState == AUDIO_STATE_VOLUME_DOWN) || (AudioState == AUDIO_STATE_VOLUME_UP))
        {
          sprintf((char *)str,  "Volume : %d ", (int)uwVolume);
          BSP_LCD_ClearStringLine(7);
          BSP_LCD_DisplayStringAtLine(7, str);
          BSP_AUDIO_IN_SetVolume(uwVolume);
          AudioState = AUDIO_STATE_PRERECORD;
        }
      }
      else
      {
        AudioState = AUDIO_STATE_PRERECORD;
      }
    }
    break;

  case AUDIO_STATE_RECORD:

    if (display_update)
    {
      audio_error = RECORDER_Start();
      if (audio_error != AUDIO_ERROR_NONE)
      {
        AudioState = AUDIO_STATE_IDLE;
        return AUDIO_ERROR_IO;
      }

      BSP_LCD_SetTextColor(LCD_COLOR_RED);    /* Display red record circle */
      BSP_LCD_FillCircle((TOUCH_RECORD_XMAX+TOUCH_RECORD_XMIN)/2,
                         (TOUCH_RECORD_YMAX+TOUCH_RECORD_YMIN)/2,
                         (TOUCH_RECORD_XMAX-TOUCH_RECORD_XMIN)/2);
      BSP_LCD_SetFont(&LCD_LOG_TEXT_FONT);
      BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
      BSP_LCD_DisplayStringAt(MSG_RECORD_XPOS, LINE(9), (uint8_t *)"  [RECORD]", LEFT_MODE);
      display_update = 0;
    }

    if(TS_State.touchDetected == 1)   /* If previous touch has not been released, we don't proceed any touch command */
    {
      BSP_TS_GetState(&TS_State);
    }
    else
    {
      BSP_TS_GetState(&TS_State);
      if(TS_State.touchDetected == 1)
      {
        if ((TS_State.touchX[0] > TOUCH_STOP_XMIN) && (TS_State.touchX[0] < TOUCH_STOP_XMAX) &&
            (TS_State.touchY[0] > TOUCH_STOP_YMIN) && (TS_State.touchY[0] < TOUCH_STOP_YMAX))
        {
          AudioState = AUDIO_STATE_STOP;
        }
        else if ((TS_State.touchX[0] > TOUCH_PAUSE_XMIN) && (TS_State.touchX[0] < TOUCH_PAUSE_XMAX) &&
                 (TS_State.touchY[0] > TOUCH_PAUSE_YMIN) && (TS_State.touchY[0] < TOUCH_PAUSE_YMAX))
        {
          AudioState = AUDIO_STATE_PAUSE;
        }
        else if((TS_State.touchX[0] > TOUCH_VOL_MINUS_XMIN) && (TS_State.touchX[0] < TOUCH_VOL_MINUS_XMAX) &&
                (TS_State.touchY[0] > TOUCH_VOL_MINUS_YMIN) && (TS_State.touchY[0] < TOUCH_VOL_MINUS_YMAX))
        {
          AudioState = AUDIO_STATE_VOLUME_DOWN;
        }
        else if((TS_State.touchX[0] > TOUCH_VOL_PLUS_XMIN) && (TS_State.touchX[0] < TOUCH_VOL_PLUS_XMAX) &&
                (TS_State.touchY[0] > TOUCH_VOL_PLUS_YMIN) && (TS_State.touchY[0] < TOUCH_VOL_PLUS_YMAX))
        {
          AudioState = AUDIO_STATE_VOLUME_UP;
        }
        /*Wait for touch released */
        do
        {
          BSP_TS_GetState(&TS_State);
        }while(TS_State.touchDetected > 0);

      }
    }

    /* MAX Recording time reached, so stop audio interface and close file */
    if(BufferRecCtl.fptr >= REC_SAMPLE_LENGTH)
    {
      display_update = 1;
      AudioState = AUDIO_STATE_STOP;
      break;
    }

    /* ++ Encoding Process of output buffer before writing to USB Key ++ */
    if(BufferRecCtl.wr_state == BUFFER_FULL)
    {

      /*---------------------------------------------------------------------------------------------*/
#if defined(__WAV_ENCODER__)

      /* input parameter for WAV format */
      t_size =AUDIO_IN_PCM_BUFFER_SIZE; /* eqv Whole buffer for 16b samples or half buffer to encode in 8b bytes [1 half*2(bytes)*2(stereo)*480]*/

      sEncoderStruct.Encoder_EncodeData((int8_t*)(BufferRecCtl.pcm_buff + BufferRecCtl.offset), /* interleaved input buffer (L/R/L...) ptr on 16b */
                                        (int8_t*)&pEncSamples,                                  /* ptr of output buffer ptr in 8b bytes */
                                        &t_size,                                                /* input size in bytes for WAV */
                                        NULL);

      if (t_size != 0)
      {
        /*t_size: nb encoded data bytes to write in audio file and byteswritten: nb bytes copied in file*/
        wrt_error = f_write(&AudioRecFile, pEncSamples, (pEncSamples != NULL)? (t_size):0, &byteswritten);

        if (wrt_error != FR_OK)
        {
          BSP_LCD_SetTextColor(LCD_COLOR_RED);
          BSP_LCD_DisplayStringAtLine(14, (uint8_t *)"RECORD FAIL");
          return AUDIO_ERROR_IO;
        }
        if ( (wrt_error != FR_OK) || (byteswritten == 0) ) /*end of file*/
        {
          display_update = 1;
          AudioState = AUDIO_STATE_STOP;
        }
      }
#endif /*__WAV_ENCODER__*/
      /*---------------------------------------------------------------------------------------------*/

#if defined(__MP3_ENCODER__)

      /* P3E__Encode576: Function encodes "576 audio samples" for 1 or two channels on 16b. iPCM_interleave = 2...
      return encoded data in bytes with size in bytes...
      Note: for  MPEG1  sampling  rates  1  MP3  frame  consists  of  two  granules
      (576  samples  in  each granule), so this function will produce data every second call (normally). */

      sEncoderStruct.Encoder_EncodeData((int8_t*)(BufferRecCtl.pcm_buff + BufferRecCtl.offset), /* interleaved input buffer (L/R/L...) ptr on 16b */
                                        (int8_t*)&pEncSamples,                                  /* ptr of output buffer ptr in 8b bytes */
                                        &t_size,                                                /* output size of enc data in bytes for MP3 */
                                        NULL);

      if (t_size != 0)
      {
        wrt_error = f_write(&AudioRecFile, (uint8_t*)pEncSamples, (pEncSamples != NULL)? (t_size):0, &byteswritten);

        if (wrt_error != FR_OK)
        {
          BSP_LCD_SetTextColor(LCD_COLOR_RED);
          BSP_LCD_DisplayStringAtLine(14, (uint8_t *)"RECORD FAIL");
          return AUDIO_ERROR_IO;
        }
        if ( (wrt_error != FR_OK) || (byteswritten == 0) )/*end of file*/
        {
          display_update = 1;
          AudioState = AUDIO_STATE_STOP;
        }
      }

#endif /*__MP3_ENCODER__*/
      /*---------------------------------------------------------------------------------------------*/

      BufferRecCtl.fptr += byteswritten;

      BACKUP_PRIMASK();
      DISABLE_IRQ();

      BufferRecCtl.wr_state =  BUFFER_EMPTY;

      RESTORE_PRIMASK();
    }
    /* ++ Elapsed time in s and kB Size for Written file ++ */

    /* Get elapsed time */
    if (sEncoderStruct.Encoder_GetElapsedTime != NULL)
    {
      elapsed_time = sEncoderStruct.Encoder_GetElapsedTime();
    }
    else
    {
      return AUDIO_ERROR_CODEC;
    }
    /* time in s */
    sprintf((char *)str, "[%02d:%02d]", (int)(elapsed_time /60), (int)(elapsed_time%60));

    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    BSP_LCD_DisplayStringAt(PLAYER_COUNT_TEXT_XMIN, LINE(PLAYER_COUNT_TEXT_YLINE), str, LEFT_MODE);

    /* file size kB */
    sprintf((char *)str, "%4d KB", (int)((int32_t)BufferRecCtl.fptr/1024));
    BSP_LCD_DisplayStringAt(PLAYER_COUNT_TEXT_KB_XMIN, LINE(PLAYER_COUNT_TEXT_YLINE), str, LEFT_MODE);

    break;

  case AUDIO_STATE_STOP:

    /* Stop recorder */
    BSP_AUDIO_IN_Stop();
    BSP_LCD_SetTextColor(LCD_COLOR_CYAN);   /* Display blue cyan record circle */
    BSP_LCD_FillCircle((TOUCH_RECORD_XMAX+TOUCH_RECORD_XMIN)/2,
                       (TOUCH_RECORD_YMAX+TOUCH_RECORD_YMIN)/2,
                       (TOUCH_RECORD_XMAX-TOUCH_RECORD_XMIN)/2);
    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_FillRect(TOUCH_STOP_XMIN, TOUCH_STOP_YMIN , /* Stop rectangle */
                     TOUCH_STOP_XMAX - TOUCH_STOP_XMIN,
                     TOUCH_STOP_YMAX - TOUCH_STOP_YMIN);
    BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
    display_update = 1;
    HAL_Delay(150);

    /* ++ WAV Header update ++ */
#ifdef __WAV_ENCODER__
    if((FileRecExtension == 'v') || (FileRecExtension == 'V'))
    {
      if(f_lseek(&AudioRecFile, 0) == FR_OK)
      {
        /* Update the wav file header save it into wav file */
        sEncoderStruct.Encoder_HeaderUpdate((uint8_t*)pHeaderBuff, (WAVE_FormatTypeDef*)&WaveFormat,(uint32_t*) &(BufferRecCtl.fptr));

        if(f_write(&AudioRecFile, pHeaderBuff, sizeof(WAVE_FormatTypeDef), (void*)&byteswritten) == FR_OK)
        {
          audio_error = AUDIO_ERROR_EOF;
        }
        else
        {
          audio_error = AUDIO_ERROR_IO;
          BSP_LCD_SetTextColor(LCD_COLOR_RED);
          BSP_LCD_DisplayStringAtLine(14, (uint8_t *)"RECORD FAIL");
        }
      }
      else
      {
        BSP_LCD_SetTextColor(LCD_COLOR_RED);
        BSP_LCD_DisplayStringAtLine(14, (uint8_t *)"RECORD FAIL");
        audio_error = AUDIO_ERROR_IO;
      }
    }
#endif

    /* ++ Free resources used by the encoder ++ */
    if (sEncoderStruct.EncoderDeInit != NULL)
    {
      sEncoderStruct.EncoderDeInit();
    }

    /* Empty the encoder structure */
    CODERS_SelectEncoder(&sEncoderStruct,' ');

#ifdef __MP3_ENCODER__
    if(FileRecExtension == '3')
    {
      /*free allocated buffers for MP3*/
      if(pMP3EncPersistent)    free(pMP3EncPersistent);
      if(pMP3EncScratch)       free(pMP3EncScratch);
    }
#endif

    /* Close file */
    f_close(&AudioRecFile);
    AudioState = AUDIO_STATE_PRERECORD;

    break;

  case AUDIO_STATE_PAUSE:
    BSP_LCD_SetTextColor(LCD_COLOR_RED);    /* Displays red pause rectangles */
    BSP_LCD_FillRect(TOUCH_PAUSE_XMIN, TOUCH_PAUSE_YMIN , 15, TOUCH_PAUSE_YMAX - TOUCH_PAUSE_YMIN);
    BSP_LCD_FillRect(TOUCH_PAUSE_XMIN + TOUCH_PAUSE_XSPACE, TOUCH_PAUSE_YMIN, 15, TOUCH_PAUSE_YMAX - TOUCH_PAUSE_YMIN);
    BSP_LCD_SetTextColor(LCD_COLOR_CYAN);   /* Display blue cyan record circle */
    BSP_LCD_FillCircle((TOUCH_RECORD_XMAX+TOUCH_RECORD_XMIN)/2,
                       (TOUCH_RECORD_YMAX+TOUCH_RECORD_YMIN)/2,
                       (TOUCH_RECORD_XMAX-TOUCH_RECORD_XMIN)/2);
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    BSP_LCD_DisplayStringAt(MSG_RECORD_XPOS, LINE(9), (uint8_t *)"  [PAUSE] ", LEFT_MODE);
    BSP_AUDIO_IN_Pause();
    AudioState = AUDIO_STATE_WAIT;
    break;

  case AUDIO_STATE_RESUME:
    BSP_LCD_SetTextColor(LCD_COLOR_CYAN);    /* Displays blue cyan pause rectangles */
    BSP_LCD_FillRect(TOUCH_PAUSE_XMIN, TOUCH_PAUSE_YMIN , 15, TOUCH_PAUSE_YMAX - TOUCH_PAUSE_YMIN);
    BSP_LCD_FillRect(TOUCH_PAUSE_XMIN + TOUCH_PAUSE_XSPACE, TOUCH_PAUSE_YMIN, 15, TOUCH_PAUSE_YMAX - TOUCH_PAUSE_YMIN);
    BSP_LCD_SetTextColor(LCD_COLOR_RED);    /* Display red record circle */
    BSP_LCD_FillCircle((TOUCH_RECORD_XMAX+TOUCH_RECORD_XMIN)/2,
                       (TOUCH_RECORD_YMAX+TOUCH_RECORD_YMIN)/2,
                       (TOUCH_RECORD_XMAX-TOUCH_RECORD_XMIN)/2);
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    BSP_LCD_DisplayStringAt(MSG_RECORD_XPOS, LINE(9), (uint8_t *)"  [RECORD]", LEFT_MODE);
    BSP_AUDIO_IN_Resume();
    AudioState = AUDIO_STATE_RECORD;
    break;

  case AUDIO_STATE_VOLUME_UP:
    if(uwVolume <= 95)
    {
      uwVolume += 5;
    }
    sprintf((char *)str,  "Volume : %d ", (int)uwVolume);
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    BSP_LCD_ClearStringLine(7);
    BSP_LCD_DisplayStringAtLine(7, str);
    BSP_AUDIO_IN_SetVolume(uwVolume);
    AudioState = AUDIO_STATE_RECORD;
    break;

  case AUDIO_STATE_VOLUME_DOWN:
    if(uwVolume >= 5)
    {
      uwVolume -= 5;
    }
    sprintf((char *)str,  "Volume : %d ", (int)uwVolume);
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    BSP_LCD_ClearStringLine(7);
    BSP_LCD_DisplayStringAtLine(7, str);
    BSP_AUDIO_IN_SetVolume(uwVolume);
    AudioState = AUDIO_STATE_RECORD;
    break;

  case AUDIO_STATE_NEXT:
  case AUDIO_STATE_PREVIOUS:
    AudioState = AUDIO_STATE_RECORD;
    break;

  case AUDIO_STATE_WAIT:
    if(TS_State.touchDetected == 1)   /* If previous touch has not been released, we don't proceed any touch command */
    {
      BSP_TS_GetState(&TS_State);
    }
    else
    {
      BSP_TS_GetState(&TS_State);
      if(TS_State.touchDetected == 1)
      {
        if ((TS_State.touchX[0] > TOUCH_RECORD_XMIN) && (TS_State.touchX[0] < TOUCH_RECORD_XMAX) &&
            (TS_State.touchY[0] > TOUCH_RECORD_YMIN) && (TS_State.touchY[0] < TOUCH_RECORD_YMAX))
        {
          AudioState = AUDIO_STATE_RESUME;
        }
        else if ((TS_State.touchX[0] > TOUCH_PAUSE_XMIN) && (TS_State.touchX[0] < TOUCH_PAUSE_XMAX) &&
                 (TS_State.touchY[0] > TOUCH_PAUSE_YMIN) && (TS_State.touchY[0] < TOUCH_PAUSE_YMAX))
        {
          AudioState = AUDIO_STATE_RESUME;
        }
        /*Wait for touch released */
        do
        {
          BSP_TS_GetState(&TS_State);
        }while(TS_State.touchDetected > 0);

      }
    }
    break;

  case AUDIO_STATE_INIT:
    RecorderDisplayButtons();
    AudioState = AUDIO_STATE_PRERECORD;
  case AUDIO_STATE_IDLE:
  default:
    /* Do Nothing */
    break;
  }

  return audio_error;
}

/**
* @brief  Micro input I2S DMA copy in PDM buffer
*         Manages the DMA Complete Transfer interrupt.
*         Calculates the remaining file size and new position of the pointer.
* @param  None
* @retval None
*/

void BSP_AUDIO_IN_TransferComplete_CallBack(void)
{
  /* PDM to PCM data convert */
  BSP_AUDIO_IN_PDMToPCM((uint16_t*)&BufferRecCtl.pdm_buff[AUDIO_IN_PDM_BUFFER_SIZE/2], /* input PDM buffer in 16b */
                        &BufferRecCtl.pcm_buff[BufferRecCtl.pcm_ptr]);                 /* output PCM buffer in 16b */


  BufferRecCtl.pcm_ptr+= AUDIO_IN_PDM_BUFFER_SIZE/2/4;

  if(BufferRecCtl.pcm_ptr == AUDIO_IN_PCM_BUFFER_SIZE/2)   /* Half PCM buffer in 16b reached*/
  {
    BufferRecCtl.wr_state   =  BUFFER_FULL;
    BufferRecCtl.offset  = 0;                              /* First PCM buffer part available to write in file*/
  }

  if(BufferRecCtl.pcm_ptr >= AUDIO_IN_PCM_BUFFER_SIZE)
  {
    BufferRecCtl.wr_state   =  BUFFER_FULL;
    BufferRecCtl.offset  = AUDIO_IN_PCM_BUFFER_SIZE/2;     /* Second PCM buffer part available to write in file*/

    BufferRecCtl.pcm_ptr = 0;
  }
}

/**
* @brief  Micro input I2S DMA copy in PDM buffer
*         Manages the DMA Half Transfer complete interrupt.
* @param  None
* @retval None
*/
void BSP_AUDIO_IN_HalfTransfer_CallBack(void)
{
  /* PDM to PCM data convert */
  BSP_AUDIO_IN_PDMToPCM((uint16_t*)&BufferRecCtl.pdm_buff[0],
                        &BufferRecCtl.pcm_buff[BufferRecCtl.pcm_ptr]);


  BufferRecCtl.pcm_ptr+= AUDIO_IN_PDM_BUFFER_SIZE/2/4;

  if(BufferRecCtl.pcm_ptr == AUDIO_IN_PCM_BUFFER_SIZE/2)
  {
    BufferRecCtl.wr_state   =  BUFFER_FULL;
    BufferRecCtl.offset  = 0;
  }

  if(BufferRecCtl.pcm_ptr >= AUDIO_IN_PCM_BUFFER_SIZE)
  {
    BufferRecCtl.wr_state   =  BUFFER_FULL;
    BufferRecCtl.offset  = AUDIO_IN_PCM_BUFFER_SIZE/2;

    BufferRecCtl.pcm_ptr = 0;
  }
}

/*******************************************************************************
Static Functions
*******************************************************************************/

/**
* @brief  Display interface touch screen buttons
* @param  None
* @retval None
*/
static void RecorderDisplayButtons(void)
{
  BSP_LCD_SetFont(&LCD_LOG_HEADER_FONT);

  BSP_LCD_ClearStringLine(14);            /* Clear dedicated zone */
  BSP_LCD_ClearStringLine(15);
  BSP_LCD_ClearStringLine(16);
  BSP_LCD_ClearStringLine(17);
  BSP_LCD_ClearStringLine(18);
  BSP_LCD_ClearStringLine(19);
  BSP_LCD_ClearStringLine(20);

  BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
  BSP_LCD_FillCircle((TOUCH_RECORD_XMAX+TOUCH_RECORD_XMIN)/2, /* Record circle */
                     (TOUCH_RECORD_YMAX+TOUCH_RECORD_YMIN)/2,
                     (TOUCH_RECORD_XMAX-TOUCH_RECORD_XMIN)/2);
  BSP_LCD_FillRect(TOUCH_PAUSE_XMIN, TOUCH_PAUSE_YMIN , 15, TOUCH_PAUSE_YMAX - TOUCH_PAUSE_YMIN);    /* Pause rectangles */
  BSP_LCD_FillRect(TOUCH_PAUSE_XMIN + TOUCH_PAUSE_XSPACE, TOUCH_PAUSE_YMIN, 15, TOUCH_PAUSE_YMAX - TOUCH_PAUSE_YMIN);
  BSP_LCD_FillRect(TOUCH_STOP_XMIN, TOUCH_STOP_YMIN , /* Stop rectangle */
                   TOUCH_STOP_XMAX - TOUCH_STOP_XMIN,
                   TOUCH_STOP_YMAX - TOUCH_STOP_YMIN);

  BSP_LCD_DrawRect(TOUCH_VOL_MINUS_XMIN, TOUCH_VOL_MINUS_YMIN, /* VOl- rectangle */
                   TOUCH_VOL_MINUS_XMAX - TOUCH_VOL_MINUS_XMIN+TOUCH_VOL_XOFFSET,
                   TOUCH_VOL_MINUS_YMAX - TOUCH_VOL_MINUS_YMIN);
  BSP_LCD_DisplayStringAt(TOUCH_VOL_MINUS_XMIN+TOUCH_VOL_XOFFSET, LINE(TOUCH_VOL_YLINE), (uint8_t *)"VOl-", LEFT_MODE);

  BSP_LCD_DrawRect(TOUCH_VOL_PLUS_XMIN, TOUCH_VOL_PLUS_YMIN , /* VOl+ rectangle */
                   TOUCH_VOL_PLUS_XMAX - TOUCH_VOL_PLUS_XMIN+TOUCH_VOL_XOFFSET,
                   TOUCH_VOL_PLUS_YMAX - TOUCH_VOL_PLUS_YMIN);
  BSP_LCD_DisplayStringAt(TOUCH_VOL_PLUS_XMIN+TOUCH_VOL_XOFFSET, LINE(TOUCH_VOL_YLINE), (uint8_t *)"VOl+", LEFT_MODE);

  BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
  BSP_LCD_DrawRect(TOUCH_BACK_XMIN, TOUCH_BACK_YMIN, /* BACK to menu */
                   TOUCH_BACK_XMAX - TOUCH_BACK_XMIN,
                   TOUCH_BACK_YMAX - TOUCH_BACK_YMIN);
  BSP_LCD_DisplayStringAt(TOUCH_BACK_XMIN+TOUCH_BACK_XOFFSET, LINE(MSG_BACK_YLINE), (uint8_t *)"BACK", LEFT_MODE);

  BSP_LCD_SetFont(&LCD_LOG_TEXT_FONT);
  BSP_LCD_DisplayStringAtLine(MSG_RECORD_TEXTYLINE, (uint8_t *)"Use record button to start record, back to exit");
  BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
