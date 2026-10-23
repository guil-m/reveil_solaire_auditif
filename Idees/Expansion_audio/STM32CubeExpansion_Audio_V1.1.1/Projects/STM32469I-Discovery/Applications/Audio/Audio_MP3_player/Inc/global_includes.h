/**
  ******************************************************************************
  * @file    global_includes.h
  * @author  MCD Application Team
  * @brief   contains all global include files
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
#ifndef __GLOBAL_INCLUDES_H__
#define __GLOBAL_INCLUDES_H__

/* Includes ------------------------------------------------------------------*/
#ifdef USE_STM32746G_DISCO
  #include "stm32f7xx_hal.h"

  #include "stm32746g_discovery.h"
  #include "stm32746g_discovery_audio.h"
  #include "stm32746g_discovery_ts.h"
  #include "stm32746g_discovery_lcd.h"
#endif

#ifdef USE_STM32F469I_DISCO
  #include "stm32f4xx_hal.h"

  #include "stm32469i_discovery.h"
  #include "stm32469i_discovery_audio.h"
  #include "stm32469i_discovery_ts.h"
  #include "stm32469i_discovery_lcd.h"
#endif


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#endif /* __GLOBAL_INCLUDES_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/


