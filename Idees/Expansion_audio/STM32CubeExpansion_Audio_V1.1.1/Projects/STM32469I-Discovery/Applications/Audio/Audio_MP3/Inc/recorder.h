/**
******************************************************************************
* @file    recorder.h
* @author  MCD Application Team
* @brief   Header for audiorecorder.c module.
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
#ifndef __RECORDER_H
#define __RECORDER_H

/* Includes ------------------------------------------------------------------*/
#include "lcd_log.h"

#include "global_includes.h"
#include "audio_conf.h"

#include "fwkmempool.h"
#include "coders.h"

/* Exported Defines ----------------------------------------------------------*/

#define AUDIO_IN_PDM_BUFFER_SIZE                   INTERNAL_BUFF_SIZE          /* Fixed PDM double buffer size for stereo samples in 16-bits*/
#define AUDIO_IN_PCM_BUFFER_MAX_SIZE               ((MAX_IN_PACKET_SZE/2)*2)   /* Max PCM double buffer size for stereo samples in 16-bits equal to double of Max encoder input packet size*/

/* complete buffer size in 16b half-word */
#if defined(__WAV_ENCODER__)
#define AUDIO_IN_PCM_BUFFER_SIZE    ((WAVIN_PACKET_SZE/2)*2)    /*WAV encoder double buffer size for stereo samples in 16-bits equal to double of MP3 encoder input packet size*/
#define REC_AUDIO_NAME              "waverec.wav"
#endif

#if defined(__MP3_ENCODER__)
#define AUDIO_IN_PCM_BUFFER_SIZE    ((MP3IN_PACKET_SZE/2)*2)   /*MP3 encoder double buffer size for stereo samples in 16-bits*/
#define REC_AUDIO_NAME              "audiorec.mp3"
#endif

#if defined(__ADPCM_ENCODER__)
#define AUDIO_IN_PCM_BUFFER_SIZE    ((ADPCM_PACKET_SZE/2)*2)   /*ADPCM encoder double buffer size for stereo samples in 16-bits*/
#define REC_AUDIO_NAME              "audiorec.adp"
#endif

#if defined(__SPEEX_ENCODER__)
#define AUDIO_IN_PCM_BUFFER_SIZE    ((SPEEX_PACKET_SZE/2)*2)   /*SPEEX encoder double buffer size for stereo samples in 16-bits*/
#define REC_AUDIO_NAME              "audiorec.spx"
#endif

/* Defines for the Audio recording process */
#define DEFAULT_TIME_REC                      30                                                        /* Recording time limit in second (default: 30s) */
#define REC_SAMPLE_LENGTH   (DEFAULT_TIME_REC * AUDIO_IN_FREQ_ADPT * DEFAULT_AUDIO_IN_CHANNEL_NBR * 2)  /* Equivalent recorded samples length in bytes */

/* Exported variabless ------------------------------------------------------------*/
#if defined(__MP3_ENCODER__)
extern uint32_t MP3EncNumChannels;
extern uint32_t MP3EncBitRate;

extern TMP3E_persist* pMP3EncPersistent;
extern TMP3E_scratch* pMP3EncScratch;
#endif

extern AUDIO_PROC_StateTypeDef AudioState;

/* Exported types ------------------------------------------------------------*/
typedef enum {
  BUFFER_EMPTY = 0,
  BUFFER_FULL,
}WR_BUFFER_StateTypeDef;

typedef struct {
  uint16_t pdm_buff[AUDIO_IN_PDM_BUFFER_SIZE];
  uint16_t pcm_buff[AUDIO_IN_PCM_BUFFER_MAX_SIZE];
  uint32_t pcm_ptr;
  WR_BUFFER_StateTypeDef wr_state;
  uint32_t offset;
  uint32_t fptr;
}AUDIO_IN_BufferTypeDef;

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
AUDIO_ErrorTypeDef RECORDER_Process(void);
AUDIO_ErrorTypeDef RECORDER_Start(void);

#endif /* __RECORDER_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
