/**
  @page Audio_SVC  Audio Smart Volume Control

  @verbatim
  ******************************************************************************
  * @file    readme.txt
  * @author  MCD Application Team
  * @brief   Description of the Audio SVC
  ******************************************************************************
  *
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
@endverbatim

@par Application Description 

This application shows how to configure and use the SVC library, 
taking as use case audio playback from USB storage.

Any wave file stored under the USB Key can be opened using the FatFs 
file system and transferred to the internal SRAM using the file system. 
All the wave files properties are read from the Wave File Header.

SVC lib brief description.
Detailed information about the SVC libraries capabilities are available at:
  \Middlewares\ST\STM32_Audio\Addons\SVC\Release_Notes.html

A touch screen interface is used to :

* navigate from a file to another (Next and Previous buttons)
* change SVC volume (db) by pressing vol+/vol- buttons
* pause and quit playback menu by pressing Stop/pause buttons. 
* disable  SVC volume by pressing SVC MUTE ON/OFF button

Plug a headphone to hear the sound  /!\ Take care of yours ears. Default volume is 50%.

Note: The audio files provided under "/Utilities/Media/Audio" are based on a free 
music download from www.DanoSongs.com website and user can load his own audio 
(*.wav) files in the USB Key root.
For this application, only stereo audio input is supported and Audio frequency 
to the codec has to be always 48Khz.
It is recommended on STM32469I-Discovery board to limit updated information on LCD display to reduce
parasitic noise during dynamic reconfiguration of some module parameters. 

@note The application needs to ensure that the SysTick time base is always set to 1 millisecond
      to have correct HAL operation.

@par Directory contents 

  - Audio_SVC/Src/main.c               Main program
  - Audio_SVC/Src/stm32f4xx_it.c       Interrupt handlers
  - Audio_SVC/Src/system_stm32f4xx.c   STM32F4xx system source file
  - Audio_SVC/Src/usbh_conf.c          Board support package for the USB host library 	
  - Audio_SVC/Src/waveplayer.c         Audio play file
  - Audio_SVC/Inc/ffconf.h             FAT file system module configuration file   
  - Audio_SVC/Inc/main.h               Main program header file
  - Audio_SVC/Inc/stm32f4xx_hal_conf.h HAL configuration file
  - Audio_SVC/Inc/stm32f4xx_it.h       Interrupt handlers header file
  - Audio_SVC/Inc/usbh_conf.h          USB driver Configuration file
  - Audio_SVC/Inc/waveplayer.h         Audio play header file
      
      
@par Hardware and Software environment

  - This example runs on STM32F469xx devices.
    
  - This application has been tested with STM32469I-Discovery board and can be
    easily tailored to any other supported device and development board.   

  - STM32469I-Discovery Set-up
    - Plug the USB key into the STM32469I-Discovery board through 'USB micro A-Male 
      to A-Female' cable (FS mode: connector CN13).
    - Connect a headphone to connector CN3.

@par How to use it ? 

In order to make the program work, you must do the following:
 - Open your preferred toolchain 
 - Rebuild all files and load your image into target memory
 - Run the application
 
 * <h3><center>&copy; COPYRIGHT STMicroelectronics</center></h3>
 */
