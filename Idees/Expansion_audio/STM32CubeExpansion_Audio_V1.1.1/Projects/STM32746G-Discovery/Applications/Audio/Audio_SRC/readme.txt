/**
  @page Audio_SRC  Audio Sample Rate Converter

  @verbatim
  ******************************************************************************
  * @file    readme.txt
  * @author  MCD Application Team
  * @brief   Description of the Audio Sampling Rate Converter
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

This application shows how to configure and use the sample rate converter libraries, 
taking as use case audio playback from USB storage.

Any wave file stored under the USB Key can be opened using the FatFs 
file system and transferred to the internal SRAM using the file system. 
All the wave files properties are read from the Wave File Header.

This application is made only for stereo input files.
If the selected input file is not stereo this example will exit displaying an error message.

The SRC converts the detected sample rate from original wav file to 48Khz Fs.
Detailed information about the SRC libraries (SRC236 and SRC441) are available at:
  \Middlewares\ST\STM32_Audio\Addons\SRC236\Release_Notes.html
  \Middlewares\ST\STM32_Audio\Addons\SRC441\Release_Notes.html

The new sample buffer is finaly sent to audio codec.
The SRC are optimized in memory to work with small buffers (10ms @24khz, 5ms @48khz, 2,5ms @96khz).

On the other hand, this example is such to provide always output to the audio codec at 48khz 
in blocks of 480 stereo samples (each 10ms) because the other AudioEffects features work on 
that scheduling. The aim is to easily combine this application with the other STM audio effects.
Therefore in order to provide a block of 480 stereo sample @48khz, 
it can be necessary to execute the SRC several times (e.g. 4 times if input wav is 96khz).

Two stereo buffer of 480 stereo 16 bits samples are used in ping-pong  
to be able to read regularly data from USB to be sent to audio codec.
The codec sees it as one buffer of 920 stereo samples, handled by DMA. 
- Using FatFs driver, at start the application reads from USB to fill the entire buffer 
(920 stereo samples)
- This buffer is sent to audio BSP which uses DMA mode to transfer data from MCU
to audio codec (16 bits)
- At DMA half transfer ISR, an indication is sent to application to indicate that 
half of the buffer has been sent (480 stereo samples)
- At DMA half transfer ISR exit, application can read from USB to fill the 1st part of the buffer
- At DMA transfer complete ISR, DMA transfer is requested to send 1st part of the buffer
- At DMA transfer complete ISR exit, application can read bytes from USB to fill
the 2nd part of the buffer
- When file size is reached, audio codec is stopped and a new play is requested.

A touch screen interface is used to navigate from a file to another (Next and Previous 
buttons), to change volume, pause playback and to quit playback menu by pressing Stop button. 

Plug a headphone to hear the sound  /!\ Take care of yours ears. Default volume is 50%.

Note: The audio files provided under "/Utilities/Media/Audio" are based on a free 
music download from www.DanoSongs.com website and user can load his own audio 
(*.wav) files in the USB Key root.
Warning: For this application, only stereo audio input is supported and Audio frequency 
to the codec has to be always 48Khz.

@note The application needs to ensure that the SysTick time base is always set to 1 millisecond
      to have correct HAL operation.

@par Directory contents 

  - Audio_SRC/Src/main.c               Main program
  - Audio_SRC/Src/stm32f7xx_it.c       Interrupt handlers
  - Audio_SRC/Src/system_stm32f7xx.c   STM32F7xx system source file
  - Audio_SRC/Src/usbh_conf.c          Board support package for the USB host library 	
  - Audio_SRC/Src/waveplayer.c         Audio play file
  - Audio_SRC/Inc/ffconf.h             FAT file system module configuration file   
  - Audio_SRC/Inc/main.h               Main program header file
  - Audio_SRC/Inc/stm32f7xx_hal_conf.h HAL configuration file
  - Audio_SRC/Inc/stm32f7xx_it.h       Interrupt handlers header file
  - Audio_SRC/Inc/usbh_conf.h          USB driver Configuration file
  - Audio_SRC/Inc/waveplayer.h         Audio play header file
      
      
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
