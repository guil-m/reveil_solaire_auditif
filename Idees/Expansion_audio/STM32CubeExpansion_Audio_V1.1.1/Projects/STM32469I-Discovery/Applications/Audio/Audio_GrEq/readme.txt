/**
  @page Audio_GrEq Audio Graphical Equalizer

  @verbatim
  ******************************************************************************
  * @file    readme.txt
  * @author  MCD Application Team
  * @brief   Description of the Audio Playback Graphical Equalizer
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

This application shows how to configure and use the Graphical Equalizer library, 
taking as use case audio playback from USB storage.

Any wave file stored under the USB Key can be opened using the FatFs 
file system and transferred to the internal SRAM using the file system. 
All the wave files properties are read from the Wave File Header.

If the selected input file has sample rate different then 48000Khz 
this example will exit displaying an error message.

The wave file is read by chunks of 480 stereo samples which are processed through 
the graphical equalizer and finaly sent to audio codec.

The Graphical equalizer (GrEq) module is in charge of fine tuning the sound 
spectrum according to user personal preferences. 
This is done by modifying gain factors at fixed frequencies represented by sliders.
The number of bands is fixed to 10 (can be provided at 5 or 8 on request). 
The gain factors are adjustable from -12 dB to +12 dB in standard mode.
The equalizer presettings are calculated for 48000 Hz sample rate.
Detailed information about the GrEq libraries capabilities are available at:
  \Middlewares\ST\STM32_Audio\Addons\GREQ\Release_Notes.html

A touch screen interface is used to navigate from a file to another (Next and Previous 
buttons), to change volume, pause playback and to quit playback menu by pressing Stop button, 
enable/disable the graphical equalizer, change the equaliser pre-settings: pop, jazz, rock, 
vocal, classical, hiphop. 

Plug a headphone to hear the sound  /!\ Take care of yours ears. Default volume is 50%.

Note: The audio files provided under "/Utilities/Media/Audio" are based on a free 
music download from www.DanoSongs.com website and user can load his own audio 
(*.wav) files in the USB Key root.
For this application, Only stereo 48Khz audio files. It is recommended on 
STM32469I-Discovery board to limit updated information on LCD display to reduce parasitic noise
during dynamic reconfiguration of some module parameters. 

@note The application needs to ensure that the SysTick time base is always set to 1 millisecond
      to have correct HAL operation.

@par Directory contents 

  - Audio_GrEq/Src/main.c               Main program
  - Audio_GrEq/Src/stm32f4xx_it.c       Interrupt handlers
  - Audio_GrEq/Src/system_stm32f4xx.c   STM32F4xx system source file
  - Audio_GrEq/Src/usbh_conf.c          Board support package for the USB host library 	
  - Audio_GrEq/Src/waveplayer.c         Audio play file
  - Audio_GrEq/Inc/ffconf.h             FAT file system module configuration file   
  - Audio_GrEq/Inc/main.h               Main program header file
  - Audio_GrEq/Inc/stm32f4xx_hal_conf.h HAL configuration file
  - Audio_GrEq/Inc/stm32f4xx_it.h       Interrupt handlers header file
  - Audio_GrEq/Inc/usbh_conf.h          USB driver Configuration file
  - Audio_GrEq/Inc/waveplayer.h         Audio play header file
      
      
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
