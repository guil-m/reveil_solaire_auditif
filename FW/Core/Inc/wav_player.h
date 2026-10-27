/**
 ******************************************************************************
 * @file    wav_player.h
 * @brief   Lecteur WAV : carte SD (FatFs/SDIO) -> I2S2 (DMA) -> PCM5102A
 *
 * Cible   : STM32F405RGT6 + HAL STM32CubeF4 (projet généré par CubeMX)
 * Formats : WAV PCM 8 / 16 / 24 bits, mono ou stéréo, 8 kHz à 96 kHz
 ******************************************************************************
 */
#ifndef WAV_PLAYER_H
#define WAV_PLAYER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------- Configuration (surchargeable) ------------------- */

/** Taille du buffer DMA complet, en trames stéréo (2 moitiés ping-pong).
 *  4096 trames = 16 Ko de RAM ; chaque moitié dure ~46 ms à 44,1 kHz.
 *  Plus grand = plus tolérant aux pauses de la carte SD. */
#ifndef WAV_BUF_FRAMES
#define WAV_BUF_FRAMES        4096u
#endif

/** Volume initial en pourcent (0..100), courbe quadratique (plus naturelle). */
#ifndef WAV_DEFAULT_VOLUME
#define WAV_DEFAULT_VOLUME    80u
#endif

/** 1 = rejoue en boucle tous les .wav de la racine ; 0 = une seule passe. */
#ifndef WAV_LOOP_ALL
#define WAV_LOOP_ALL          1
#endif

/** 1 = journal texte sur USART3 (huart3, header J4 de ta carte). */
#ifndef WAV_ENABLE_UART_LOG
#define WAV_ENABLE_UART_LOG   0
#endif

/* ------------------------------- API --------------------------------------- */

/**
 * @brief  Monte la carte SD puis lit en boucle les fichiers .wav de la racine.
 * @note   Ne retourne jamais (sauf WAV_LOOP_ALL == 0 : alors elle attend en WFI).
 *         À appeler une seule fois, dans USER CODE 2 de main(), après
 *         MX_FATFS_Init(), MX_I2S2_Init(), MX_SDIO_SD_Init() et MX_DMA_Init().
 */
void WavPlayer_Run(void);

/** @brief Règle le volume logiciel, 0..100 (utilisable pendant la lecture). */
void WavPlayer_SetVolume(uint8_t percent);

#ifdef __cplusplus
}
#endif

#endif /* WAV_PLAYER_H */
