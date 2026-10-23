/**
  @page Audio_SMR

  @verbatim
  ******************************************************************************
  * @file    readme.txt
  * @author  MCD Application Team
  * @brief   Description of the Audio SMR Manager
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

This application shows how to configure and use the SMR library, 
taking as use case audio playback from USB storage.

Any wave file stored under the USB Key can be opened using the FatFs 
file system and transferred to the internal SRAM using the file system. 
All the wave files properties are read from the Wave File Header.

SMR lib brief description.
Detailed information about the SMR libraries capabilities are available at:
  \Middlewares\ST\STM32_Audio\Addons\SMR\Release_Notes.html

A touch screen interface is used to:
* navigate from a file to another (Next and Previous buttons) 
* change volume
* pause playback and to quit playback menu by pressing Stop button. 
* change averaging time from 0 to 10000ms
* default filter type: A-weighting pre-filter, it can be changed in application code.

Plug a headphone to hear the sound  /!\ Take care of yours ears. Default volume is 50%.

Note: The audio files provided under "/Utilities/Media/Audio" are based on a free 
music download from www.DanoSongs.com website and user can load his own audio 
(*.wav) files in the USB Key root.
  If the selected input file has sample rate different then 48000Hz 
this example will exit displaying an error message.
  The signal power estimated can be returned on LCD screen by uncommenting compilation 
flag "DISPLAY_OUTPUT" in waveplayer header file. In this case, the quality of stereo
audio signal is noticeable degradated.
      
@note The application needs to ensure that the SysTick time base is always set to 1 millisecond
      to have correct HAL operation.

@par Directory contents 

  - Audio_SMR/Src/main.c               Main program
  - Audio_SMR/Src/stm32f7xx_it.c       Interrupt handlers
  - Audio_SMR/Src/system_stm32f7xx.c   STM32F7xx system source file
  - Audio_SMR/Src/usbh_conf.c          Board support package for the USB host library 	
  - Audio_SMR/Src/waveplayer.c         Audio play file
  - Audio_SMR/Inc/ffconf.h             FAT file system module configuration file   
  - Audio_SMR/Inc/main.h               Main program header file
  - Audio_SMR/Inc/stm32f7xx_hal_conf.h HAL configuration file
  - Audio_SMR/Inc/stm32f7xx_it.h       Interrupt handlers header file
  - Audio_SMR/Inc/usbh_conf.h          USB driver Configuration file
  - Audio_SMR/Inc/waveplayer.h         Audio play header file
      
      
@par Hardware and Software environment

  - This example runs on STM32F756xx/STM32F746xx devices.
    
  - This application has been tested with STM32746G-Discovery board and can be
    easily tailored to any other supported device and development board.   

  - STM32746G-Discovery Set-up
    - Plug the USB key into the STM32746G-Discovery board through 'USB micro A-Male 
      to A-Female' cable (FS mode: connector CN13).
    - Connect a headphone to connector CN10.

@par How to use it ? 

In order to make the program work, you must do the following:
 - Open your preferred toolchain 
 - Rebuild all files and load your image into target memory
 - Run the application
 
 * <h3><center>&copy; COPYRIGHT STMicroelectronics</center></h3>
 */
